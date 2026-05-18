#include "hal_data.h"
#include <stdio.h>
#include <string.h>
#include "zmod4410.h"
#include "hs3001.h"
#include "SEGGER_RTT.h"
FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event);
FSP_CPP_FOOTER

volatile rm_zmod4xxx_iaq_2nd_data_t iaq_2nd_gen_data;
volatile rm_hs300x_data_t hs300x_data;
/* Cờ báo hiệu 3 giây */
volatile bool flag_trigger_3s = false;
float temp_float=0;
float hum_float=0;
extern float run_iaq_prediction(float* sensor_data);
float predicted_iaq;
/* Hàm ngắt Timer đếm hết 3 giây */
void timer3s_callback(timer_callback_args_t * p_args)
{
    if (TIMER_EVENT_CYCLE_END == p_args->event)
    {
        flag_trigger_3s = true;
    }
}

/* UART3 callback */
void uart3_callback(uart_callback_args_t * p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);
}

#define HISTORY_SIZE 10 // 10 mẫu x 3s = 30s để tính delta30s

// Khai báo 6 mảng vòng cho 6 giá trị cần tính độ biến thiên (delta)
static float iaq_history[HISTORY_SIZE] = {0};
static float tvoc_history[HISTORY_SIZE] = {0};
static float etoh_history[HISTORY_SIZE] = {0};
static float rel_iaq_history[HISTORY_SIZE] = {0};
static float temp_history[HISTORY_SIZE] = {0};
static float hum_history[HISTORY_SIZE] = {0};

static int ring_index = 0;
static bool is_30s_passed = false; // Cờ báo hiệu đã thu thập đủ 30 giây đầu tiên

bool stable;
volatile float iaq_raw;
float predicted_iaq = 0.0f;

/// Hoàn chỉnh hệ thống V1
void hal_entry(void)
{


    R_PORT3->PDR |= (1U << 7);


        /* --- Cấu hình P307 làm chân Output --- */
            R_PORT3->PDR = (uint16_t)(R_PORT3->PDR | (1U << 7));

            /* --- Kéo chân P307 (RES_N) xuống mức LOW để ép ZMOD4410 reset --- */
            R_PORT3->PODR = (uint16_t)(R_PORT3->PODR & ~(1U << 7));
            R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);

            /* --- Kéo chân P307 lên mức HIGH để cảm biến thức dậy --- */
            R_PORT3->PODR = (uint16_t)(R_PORT3->PODR | (1U << 7));
            R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);

    /* Gọi các hàm setup gốc từ zmod4410.h */
    g_comms_i2c_bus0_quick_setup();
    g_zmod4xxx_sensor0_quick_setup();
    g_hs300x_sensor0_quick_setup();

    /* --- GIAI ĐOẠN 1: CHỜ DỮ LIỆU HỢP LỆ --- */
    do {
        g_hs300x_sensor0_quick_getting_humidity_and_temperature((rm_hs300x_data_t*) &hs300x_data);
        temp_float = (float) hs300x_data.temperature.integer_part + (float) hs300x_data.temperature.decimal_part / 100.0f;
        hum_float = (float) hs300x_data.humidity.integer_part + (float) hs300x_data.humidity.decimal_part / 100.0f;
        R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);
    } while (hs300x_data.temperature.integer_part == 0 || hs300x_data.humidity.integer_part == 0);


    /* --- 1. MỞ KHÓA THANH GHI HỆ THỐNG --- */
        // Ghi password (0xA5) vào byte cao, và set bit PRC1 (bit 1) để cho phép ghi vào các thanh ghi Low Power/Clock
        R_SYSTEM->PRCR = 0xA502;

        /* --- 2. CẤP CLOCK CHO NGOẠI VI SCI3 --- */
        // SCI3 nằm ở bit 28 của thanh ghi Module Stop Control Register B (MSTPCRB)
        R_MSTP->MSTPCRB &= ~(1U << 28);

        /* --- 3. KHÓA LẠI THANH GHI HỆ THỐNG --- */
        // Đảm bảo an toàn, không cho phép ghi nhầm vào hệ thống nữa
        R_SYSTEM->PRCR = 0xA500;

        /* --- 4. CẤU HÌNH SCI3 (UART) --- */
        R_SCI3->SCR = 0x00;
        R_SCI3->SMR = 0x00;


          // BRR = (100,000,000 / (64 * 2^(2*0-1) * 115200)) - 1 = 26

        R_SCI3->BRR = 26;          // Điền giá trị BRR tính toán được vào đây

        R_SCI3->SCR = 0x30;        // Bật TE (Transmit Enable) và RE (Receive Enable)

    while (1)
    {
        /* 1. Đọc nhiệt ẩm thực tế từ HS300x */
        g_hs300x_sensor0_quick_getting_humidity_and_temperature((rm_hs300x_data_t*) &hs300x_data);
        temp_float = (float) hs300x_data.temperature.integer_part + (float) hs300x_data.temperature.decimal_part / 100.0f;
        hum_float = (float) hs300x_data.humidity.integer_part + (float) hs300x_data.humidity.decimal_part / 100.0f;

        /* 2. DELAY GIẢI PHÓNG BUS I2C TRÁNH XUNG ĐỘT (10ms) */
        R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);

        /* 3. Đọc dữ liệu ZMOD với nhiệt ẩm bù trừ */
        stable = g_zmod4xxx_sensor0_quick_getting_iaq_2nd_gen_data(
                (rm_zmod4xxx_iaq_2nd_data_t*) &iaq_2nd_gen_data, temp_float, hum_float);

        iaq_raw  = iaq_2nd_gen_data.iaq;
        float tvoc_raw = iaq_2nd_gen_data.tvoc;
        float etoh_raw = iaq_2nd_gen_data.etoh;
        float rel_iaq_raw = iaq_2nd_gen_data.rel_iaq;
        // eco2 không dùng trong model mới, nhưng vẫn đọc để log ra UART nếu cần
        float eco2_raw = iaq_2nd_gen_data.eco2;

        /* --- BẮT ĐẦU XỬ LÝ MẢNG VÒNG CHO AI --- */
        float delta30s_iaq = 0.0f;
        float delta30s_tvoc = 0.0f;
        float delta30s_etoh = 0.0f;
        float delta30s_iaq_rel = 0.0f;
        float delta30s_temp = 0.0f;
        float delta30s_hum = 0.0f;

        // Chỉ tính Diff và chạy AI nếu ZMOD đã stable VÀ mảng đã gom đủ 30 giây
        if (stable && is_30s_passed)
        {
            // Lấy giá trị quá khứ (cách đây 30s)
            float iaq_30s_ago     = iaq_history[ring_index];
            float tvoc_30s_ago    = tvoc_history[ring_index];
            float etoh_30s_ago    = etoh_history[ring_index];
            float rel_iaq_30s_ago = rel_iaq_history[ring_index];
            float temp_30s_ago    = temp_history[ring_index];
            float hum_30s_ago     = hum_history[ring_index];

            // Tính Delta (Hiện tại - Quá khứ)
            delta30s_iaq     = iaq_raw - iaq_30s_ago;
            delta30s_tvoc    = tvoc_raw - tvoc_30s_ago;
            delta30s_etoh    = etoh_raw - etoh_30s_ago;
            delta30s_iaq_rel = rel_iaq_raw - rel_iaq_30s_ago;
            delta30s_temp    = temp_float - temp_30s_ago;
            delta30s_hum     = hum_float - hum_30s_ago;

            // ===> ĐƯA ĐẶC TRƯNG VÀO MẢNG (ĐÚNG THỨ TỰ PYTHON)
            float test_features[12] = {
                iaq_raw,
                tvoc_raw,
                etoh_raw,
                rel_iaq_raw,
                temp_float,
                hum_float,
                delta30s_iaq,
                delta30s_tvoc,
                delta30s_etoh,
                delta30s_iaq_rel,
                delta30s_temp,
                delta30s_hum
            };

            // LƯU Ý: Đừng quên áp dụng SCALER_MEAN và SCALER_STD vào test_features
            // trước khi truyền vào hàm run_iaq_prediction() ở file C++ xử lý TFLite
            predicted_iaq = run_iaq_prediction(test_features);
        }

        // Ghi đè giá trị hiện tại vào mảng vòng
        if (stable)
        {
            iaq_history[ring_index]     = iaq_raw;
            tvoc_history[ring_index]    = tvoc_raw;
            etoh_history[ring_index]    = etoh_raw;
            rel_iaq_history[ring_index] = rel_iaq_raw;
            temp_history[ring_index]    = temp_float;
            hum_history[ring_index]     = hum_float;

            ring_index++;
            if (ring_index >= HISTORY_SIZE)
            {
                ring_index = 0;
                is_30s_passed = true; // Đã chạy đủ 30 giây khởi động
            }
        }

        /* --- ĐÁNH GIÁ TRẠNG THÁI MÔI TRƯỜNG (DỰ BÁO 1 PHÚT TỚI) --- */
        const char * env_status = "Waiting_Data"; // Mặc định trong 30s đầu

        if (is_30s_passed)
        {
            // Các ngưỡng này có thể điều chỉnh tùy theo datasheet ZMOD
            if (predicted_iaq <= 2.0f) {
                env_status = "Tot";
            } else if (predicted_iaq <= 3.0f) {
                env_status = "Trung_Binh";
            } else if (predicted_iaq <= 4.0f) {
                env_status = "Kem";
            } else if (predicted_iaq <= 5.0f) {
                env_status = "Xau";
            } else {
                env_status = "Nguy_Hiem";
            }
        }

        /* --- INTEGER MATH: CHUẨN BỊ GỬI UART --- */
        int iaq_a  = (int)(iaq_raw);  int iaq_b  = (int)(iaq_raw  * 10000.0f + 0.5f) % 10000;
        int tvoc_a = (int)(tvoc_raw); int tvoc_b = (int)(tvoc_raw * 10000.0f + 0.5f) % 10000;
        int etoh_a = (int)(etoh_raw); int etoh_b = (int)(etoh_raw * 10000.0f + 0.5f) % 10000;
        int eco2_a = (int)(eco2_raw); int eco2_b = (int)(eco2_raw * 10000.0f + 0.5f) % 10000;
        int rel_a  = (int)(rel_iaq_raw); int rel_b  = (int)(rel_iaq_raw * 10000.0f + 0.5f) % 10000;
        int pred_a = (int)(predicted_iaq); int pred_b = (int)(predicted_iaq * 10000.0f + 0.5f) % 10000;
        int temp_a = (int)(temp_float); int temp_b = (int)(temp_float * 10000.0f + 0.5f) % 10000;
        int hum_a  = (int)(hum_float);  int hum_b  = (int)(hum_float  * 10000.0f + 0.5f) % 10000;

        // Xử lý số âm cho phần thập phân
        if (iaq_b  < 0) iaq_b  = -iaq_b;
        if (tvoc_b < 0) tvoc_b = -tvoc_b;
        if (etoh_b < 0) etoh_b = -etoh_b;
        if (eco2_b < 0) eco2_b = -eco2_b;
        if (rel_b  < 0) rel_b  = -rel_b;
        if (pred_b < 0) pred_b = -pred_b;
        if (temp_b < 0) temp_b = -temp_b;
        if (hum_b  < 0) hum_b  = -hum_b;

        char tx_buf[448];
        int len = snprintf(tx_buf, sizeof(tx_buf),
                            "{\"device_id\":\"ck_ra6m5_gateway_01\","
                            "\"iaq\":%d.%04d,"
                            "\"rel_iaq\":%d.%04d,"
                            "\"tvoc\":%d.%04d,"
                            "\"etoh\":%d.%04d,"
                            "\"eco2\":%d.%04d,"
                            "\"temp\":%d.%04d,"
                            "\"hum\":%d.%04d,"
                            "\"stable\":%d,"
                            "\"err\":0,"
                            "\"predicted_iaq\":%d.%04d," // Đổi tên nhãn UART cho đúng logic
                            "\"status_1m\":\"%s\"}\n",
                            iaq_a, iaq_b, rel_a, rel_b, tvoc_a, tvoc_b, etoh_a, etoh_b, eco2_a, eco2_b,
                            temp_a, temp_b, hum_a, hum_b,
                            (int) stable, pred_a, pred_b, env_status);

        /* Gửi qua UART3 */
        for(int i = 0; i < len; i++) {

                            // Dùng cờ TDRE (bit 7 - 0x80) thay vì TEND (bit 6 - 0x40)
                            while((R_SCI3->SSR & 0x80) == 0) {
                                // Chờ thanh ghi TDR trống
                            }

                            // Ghi 1 byte vào thanh ghi truyền
                            R_SCI3->TDR = (uint8_t) tx_buf[i];
                        }
    }
}

/// Thu thập dữ liệu
//void hal_entry(void)
//{
//    fsp_err_t err;
//
//    /* 1. KHỞI TẠO J-LINK RTT */
//    SEGGER_RTT_Init();
//    SEGGER_RTT_WriteString(0, "\r\n--- KHOI DONG HE THONG THU THAP DU LIEU ---\r\n");
//    // Header CSV để bạn dễ lưu file sau này
//    SEGGER_RTT_WriteString(0, "IAQ,Rel_IAQ,TVOC,EtOH,eCO2,Temp,Hum,Stable\r\n");
//
//    /* 2. RESET PHẦN CỨNG ZMOD4410 (Chân P307) */
//    R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_03_PIN_07, BSP_IO_LEVEL_LOW);
//    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);
//    R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_03_PIN_07, BSP_IO_LEVEL_HIGH);
//    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);
//
//    /* 3. SETUP CÁC DRIVER FSP */
//    g_comms_i2c_bus0_quick_setup();
//    g_zmod4xxx_sensor0_quick_setup();
//    g_hs300x_sensor0_quick_setup();
//
//    /* 4. CHỜ DỮ LIỆU NHIỆT ẨM HỢP LỆ (Giai đoạn Warm-up HS3001) */
//    do {
//        g_hs300x_sensor0_quick_getting_humidity_and_temperature((rm_hs300x_data_t *)&hs300x_data);
//
//        temp_float = (float)hs300x_data.temperature.integer_part + (float)hs300x_data.temperature.decimal_part / 100.0f;
//        hum_float  = (float)hs300x_data.humidity.integer_part + (float)hs300x_data.humidity.decimal_part / 100.0f;
//
//        R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);
//    } while (hs300x_data.temperature.integer_part == 0 || hs300x_data.humidity.integer_part == 0);
//
//    /* 5. KHỞI CHẠY TIMER 3 GIÂY */
//    err = g_timer0.p_api->open(g_timer0.p_ctrl, g_timer0.p_cfg);
//    assert(FSP_SUCCESS == err);
//    err = g_timer0.p_api->start(g_timer0.p_ctrl);
//    assert(FSP_SUCCESS == err);
//
//    /* --- VÒNG LẶP CHÍNH --- */
//    while (1)
//    {
//
//            /* Bước A: Đọc nhiệt ẩm thực tế */
//            g_hs300x_sensor0_quick_getting_humidity_and_temperature((rm_hs300x_data_t *)&hs300x_data);
//            temp_float = (float)hs300x_data.temperature.integer_part + (float)hs300x_data.temperature.decimal_part / 100.0f;
//            hum_float  = (float)hs300x_data.humidity.integer_part + (float)hs300x_data.humidity.decimal_part / 100.0f;
//
//            /* Giải phóng bus I2C ngắn hạn */
//            R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);
//
//            /* Bước B: Đọc dữ liệu khí ZMOD với bù trừ nhiệt ẩm */
//            stable = g_zmod4xxx_sensor0_quick_getting_iaq_2nd_gen_data(
//                            (rm_zmod4xxx_iaq_2nd_data_t *)&iaq_2nd_gen_data, temp_float, hum_float);
//
//            /* Bước C: Tách số nguyên/thập phân (4 chữ số) để in ra RTT */
//            // IAQ (1.0 - 5.0)
//            int iaq_a = (int)iaq_2nd_gen_data.iaq;
//            int iaq_b = (int)(iaq_2nd_gen_data.iaq * 10000.0f + 0.5f) % 10000;
//
//            // Rel IAQ
//            int rel_a = (int)iaq_2nd_gen_data.rel_iaq;
//            int rel_b = (int)(iaq_2nd_gen_data.rel_iaq * 10000.0f + 0.5f) % 10000;
//
//            // TVOC
//            int tvoc_a = (int)iaq_2nd_gen_data.tvoc;
//            int tvoc_b = (int)(iaq_2nd_gen_data.tvoc * 10000.0f + 0.5f) % 10000;
//
//            // EtOH
//            int etoh_a = (int)iaq_2nd_gen_data.etoh;
//            int etoh_b = (int)(iaq_2nd_gen_data.etoh * 10000.0f + 0.5f) % 10000;
//
//            // eCO2
//            int eco2_a = (int)iaq_2nd_gen_data.eco2;
//            int eco2_b = (int)(iaq_2nd_gen_data.eco2 * 10000.0f + 0.5f) % 10000;
//
//            // Temp & Hum
//            int t_a = (int)temp_float;
//            int t_b = (int)(temp_float * 100.0f + 0.5f) % 100;
//            int h_a = (int)hum_float;
//            int h_b = (int)(hum_float * 100.0f + 0.5f) % 100;
//
//            /* Xử lý dấu âm cho phần thập phân nếu có */
//            if (iaq_b < 0)  iaq_b  = -iaq_b;
//            if (tvoc_b < 0) tvoc_b = -tvoc_b;
//            if (etoh_b < 0) etoh_b = -etoh_b;
//            if (eco2_b < 0) eco2_b = -eco2_b;
//
//            /* Bước D: Xuất dữ liệu CSV ra J-Link RTT Viewer */
//            char rtt_msg[256];
//            snprintf(rtt_msg, sizeof(rtt_msg),
//                     "%d.%04d, %d.%04d, %d.%04d, %d.%04d, %d.%04d, %d.%02d, %d.%02d, %d\r\n",
//                     iaq_a, iaq_b, rel_a, rel_b, tvoc_a, tvoc_b, etoh_a, etoh_b, eco2_a, eco2_b,
//                     t_a, t_b, h_a, h_b, (int)stable);
//
//            SEGGER_RTT_WriteString(0, rtt_msg);
//        }
//    }

#if BSP_TZ_SECURE_BUILD
FSP_CPP_HEADER
BSP_CMSE_NONSECURE_ENTRY void template_nonsecure_callable(void);
BSP_CMSE_NONSECURE_ENTRY void template_nonsecure_callable(void) { }
FSP_CPP_FOOTER
#endif

