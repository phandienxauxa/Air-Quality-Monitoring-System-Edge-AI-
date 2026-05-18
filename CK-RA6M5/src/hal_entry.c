#include "hal_data.h"
#include <stdio.h>
#include <string.h>
#include "zmod4410.h"
#include "hs3001.h"
#include "SEGGER_RTT.h"
FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event);
FSP_CPP_FOOTER

///* Testcase: no header file*/
//void g_comms_i2c_bus0_quick_setup(void);
//
//void g_comms_i2c_bus0_quick_setup(void)
//{
//    fsp_err_t err;
//    i2c_master_instance_t * p_driver_instance =
//        (i2c_master_instance_t *) g_comms_i2c_bus0_extended_cfg.p_driver_instance;
//
//    err = p_driver_instance->p_api->open(p_driver_instance->p_ctrl, p_driver_instance->p_cfg);
//    assert(FSP_SUCCESS == err);
//}
//
///* Zmod400 */
//#define G_ZMOD4XXX_SENSOR0_NON_BLOCKING    (1)
//#define G_ZMOD4XXX_SENSOR0_IRQ_ENABLE      (0)  /* Disable IRQ */
//
//#if G_ZMOD4XXX_SENSOR0_NON_BLOCKING
//volatile bool                g_zmod4xxx_i2c_completed = false;
//volatile rm_zmod4xxx_event_t g_zmod4xxx_i2c_callback_event;
//#endif
//
//#if G_ZMOD4XXX_SENSOR0_IRQ_ENABLE
//volatile bool g_zmod4xxx_irq_completed = false;
//#endif
//
///* I2C callback */
//void zmod4xxx_comms_i2c_callback(rm_zmod4xxx_callback_args_t * p_args)
//{
//#if G_ZMOD4XXX_SENSOR0_NON_BLOCKING
//    g_zmod4xxx_i2c_callback_event = p_args->event;
//    if (RM_ZMOD4XXX_EVENT_ERROR != p_args->event)
//    {
//        g_zmod4xxx_i2c_completed = true;
//    }
//#else
//    FSP_PARAMETER_NOT_USED(p_args);
//#endif
//}
//
///* IRQ callback */
//void zmod4xxx_irq_callback(rm_zmod4xxx_callback_args_t * p_args)
//{
//#if G_ZMOD4XXX_SENSOR0_IRQ_ENABLE
//    if (RM_ZMOD4XXX_EVENT_MEASUREMENT_COMPLETE == p_args->event)
//    {
//        g_zmod4xxx_irq_completed = true;
//    }
//#else
//    FSP_PARAMETER_NOT_USED(p_args);
//#endif
//}
//
///* UART3 callback */
//void uart3_callback(uart_callback_args_t * p_args)
//{
//    FSP_PARAMETER_NOT_USED(p_args);
//}
//
//#define G_ZMOD4XXX_DELAY_50    (50)
//#define G_ZMOD4XXX_DELAY_3000  (3000)
//
///* Open ZMOD4410 */
//void g_zmod4xxx_sensor0_quick_setup(void);
//
//void g_zmod4xxx_sensor0_quick_setup(void)
//{
//    fsp_err_t err;
//    err = g_zmod4xxx_sensor0.p_api->open(g_zmod4xxx_sensor0.p_ctrl, g_zmod4xxx_sensor0.p_cfg);
//    assert(FSP_SUCCESS == err);
//}
//
///* Get IAQ 2nd Gen data
// * return: true nếu đã qua warm-up và data hợp lệ
// */
//bool g_zmod4xxx_sensor0_quick_getting_iaq_2nd_gen_data(
//    rm_zmod4xxx_iaq_2nd_data_t * p_gas_data,
//    float temperature,
//    float humidity);
//
//bool g_zmod4xxx_sensor0_quick_getting_iaq_2nd_gen_data(
//    rm_zmod4xxx_iaq_2nd_data_t * p_gas_data,
//    float temperature,
//    float humidity)
//{
//    fsp_err_t              err;
//    rm_zmod4xxx_raw_data_t zmod4xxx_raw_data;
//    bool stabilization_complete = false;
//
//    /* Clear flags */
//#if G_ZMOD4XXX_SENSOR0_IRQ_ENABLE
//    g_zmod4xxx_irq_completed = false;
//#endif
//#if G_ZMOD4XXX_SENSOR0_NON_BLOCKING
//    g_zmod4xxx_i2c_completed = false;
//#endif
//
//    /* Start measurement */
//    err = g_zmod4xxx_sensor0.p_api->measurementStart(g_zmod4xxx_sensor0.p_ctrl);
//    assert(FSP_SUCCESS == err);
//#if G_ZMOD4XXX_SENSOR0_NON_BLOCKING
//    while (!g_zmod4xxx_i2c_completed) { ; }
//    g_zmod4xxx_i2c_completed = false;
//#endif
//
//    /* Delay 3s cho IAQ 2nd Gen measurement cycle */
//    R_BSP_SoftwareDelay(G_ZMOD4XXX_DELAY_3000, BSP_DELAY_UNITS_MILLISECONDS);
//
//    /* Poll cho đến khi measurement xong */
//    do
//    {
//#if G_ZMOD4XXX_SENSOR0_IRQ_ENABLE
//        while (!g_zmod4xxx_irq_completed) { ; }
//        g_zmod4xxx_irq_completed = false;
//#else
//        err = g_zmod4xxx_sensor0.p_api->statusCheck(g_zmod4xxx_sensor0.p_ctrl);
//        assert(FSP_SUCCESS == err);
//#if G_ZMOD4XXX_SENSOR0_NON_BLOCKING
//        while (!g_zmod4xxx_i2c_completed) { ; }
//        g_zmod4xxx_i2c_completed = false;
//#endif
//#endif
//
//        /* Kiểm tra lỗi device */
//        err = g_zmod4xxx_sensor0.p_api->deviceErrorCheck(g_zmod4xxx_sensor0.p_ctrl);
//        assert(FSP_SUCCESS == err);
//#if G_ZMOD4XXX_SENSOR0_NON_BLOCKING
//        while (!g_zmod4xxx_i2c_completed) { ; }
//        g_zmod4xxx_i2c_completed = false;
//
//        if ((RM_ZMOD4XXX_EVENT_DEV_ERR_POWER_ON_RESET  == g_zmod4xxx_i2c_callback_event) ||
//            (RM_ZMOD4XXX_EVENT_DEV_ERR_ACCESS_CONFLICT == g_zmod4xxx_i2c_callback_event))
//        {
//            while (1) { ; } /* Device error — cần reset */
//        }
//#endif
//
//        /* Đọc ADC data */
//        err = g_zmod4xxx_sensor0.p_api->read(g_zmod4xxx_sensor0.p_ctrl, &zmod4xxx_raw_data);
//        if (FSP_ERR_SENSOR_MEASUREMENT_NOT_FINISHED == err)
//        {
//            R_BSP_SoftwareDelay(G_ZMOD4XXX_DELAY_50, BSP_DELAY_UNITS_MILLISECONDS);
//        }
//    }
//    while (FSP_ERR_SENSOR_MEASUREMENT_NOT_FINISHED == err);
//    assert(FSP_SUCCESS == err);
//
//#if G_ZMOD4XXX_SENSOR0_NON_BLOCKING
//    while (!g_zmod4xxx_i2c_completed) { ; }
//    g_zmod4xxx_i2c_completed = false;
//#endif
//
//    /* Kiểm tra lỗi device lần 2 sau read */
//    err = g_zmod4xxx_sensor0.p_api->deviceErrorCheck(g_zmod4xxx_sensor0.p_ctrl);
//    assert(FSP_SUCCESS == err);
//#if G_ZMOD4XXX_SENSOR0_NON_BLOCKING
//    while (!g_zmod4xxx_i2c_completed) { ; }
//    g_zmod4xxx_i2c_completed = false;
//
//    if ((RM_ZMOD4XXX_EVENT_DEV_ERR_POWER_ON_RESET  == g_zmod4xxx_i2c_callback_event) ||
//        (RM_ZMOD4XXX_EVENT_DEV_ERR_ACCESS_CONFLICT == g_zmod4xxx_i2c_callback_event))
//    {
//        while (1) { ; }
//    }
//#endif
//
//    /* Set nhiệt độ & độ ẩm (mặc định 25°C / 50%RH) */
//    err = g_zmod4xxx_sensor0.p_api->temperatureAndHumiditySet(
//              g_zmod4xxx_sensor0.p_ctrl, temperature, humidity);
//    assert(FSP_SUCCESS == err);
//
//    /* Tính IAQ 2nd Gen */
//    err = g_zmod4xxx_sensor0.p_api->iaq2ndGenDataCalculate(
//              g_zmod4xxx_sensor0.p_ctrl, &zmod4xxx_raw_data, p_gas_data);
//
//    if (FSP_SUCCESS == err)
//        stabilization_complete = true;
//    else if (FSP_ERR_SENSOR_IN_STABILIZATION == err || FSP_ERR_SENSOR_INVALID_DATA == err)
//        stabilization_complete = false;
//    else
//        assert(false);
//
//    return stabilization_complete;
//}
//
///* Cờ báo hiệu 3 giây */
//volatile bool flag_trigger_3s = false;
//
///* Hàm này sẽ tự chạy mỗi khi Timer đếm hết 3 giây */
//void timer3s_callback(timer_callback_args_t * p_args)
//{
//    if (TIMER_EVENT_CYCLE_END == p_args->event)
//    {
//        flag_trigger_3s = true;
//    }
//}
//
///* Global data */
//volatile rm_zmod4xxx_iaq_2nd_data_t iaq_2nd_gen_data;
//
///* hal_entry */
//void hal_entry(void)
//{
//    g_comms_i2c_bus0_quick_setup ();
//    g_zmod4xxx_sensor0_quick_setup ();
//
//    const float TEMP_DEFAULT = 25.0f;
//    const float HUM_DEFAULT = 50.0f;
//
//    /* Mở UART3 (TXD=P707, RXD=P706, 115200 baud) */
//    fsp_err_t err;
//    err = g_uart3.p_api->open (g_uart3.p_ctrl, g_uart3.p_cfg);
//    assert(FSP_SUCCESS == err);
//    g_timer0.p_api->open (g_timer0.p_ctrl, g_timer0.p_cfg);
//    g_timer0.p_api->start (g_timer0.p_ctrl);
//
//    while (1)
//    {
//        if (flag_trigger_3s)
//        {
//            flag_trigger_3s = false;
//            bool stable = g_zmod4xxx_sensor0_quick_getting_iaq_2nd_gen_data (
//                    (rm_zmod4xxx_iaq_2nd_data_t*) &iaq_2nd_gen_data, TEMP_DEFAULT, HUM_DEFAULT);
//
//            /* Map IAQ từ thang 1–5 sang 0–500 */
//            float iaq_mapped = (iaq_2nd_gen_data.iaq - 1.0f) / 4.0f * 500.0f;
//            float tvoc_mapped = iaq_2nd_gen_data.tvoc * 100.0f; /* mg/m³ → ~0–500 */
//            float etoh_mapped = iaq_2nd_gen_data.etoh * 100.0f; /* mg/m³ → ~0–500 */
//            float eco2_mapped = iaq_2nd_gen_data.eco2; /* ppm, giữ nguyên */
//
//            /* Integer math — tránh hoàn toàn float printf */
//            int iaq_a = (int) (iaq_mapped);
//            int iaq_b = (int) (iaq_mapped * 100) % 100;
//            int tvoc_a = (int) (tvoc_mapped);
//            int tvoc_b = (int) (tvoc_mapped * 100) % 100;
//            int etoh_a = (int) (etoh_mapped);
//            int etoh_b = (int) (etoh_mapped * 100) % 100;
//            int eco2_a = (int) (eco2_mapped);
//            int eco2_b = (int) (eco2_mapped * 100) % 100;
//            if (iaq_b < 0)
//                iaq_b = -iaq_b;
//            if (tvoc_b < 0)
//                tvoc_b = -tvoc_b;
//            if (etoh_b < 0)
//                etoh_b = -etoh_b;
//            if (eco2_b < 0)
//                eco2_b = -eco2_b;
//
//            char tx_buf[256];
//            int len = snprintf (tx_buf, sizeof(tx_buf), "{\"device_id\":\"ck_ra6m5_gateway_01\","
//                                "\"iaq\":%d.%02d,"
//                                "\"tvoc\":%d.%02d,"
//                                "\"etoh\":%d.%02d,"
//                                "\"eco2\":%d.%02d,"
//                                "\"stable\":%d}\n",
//                                iaq_a, iaq_b, tvoc_a, tvoc_b, etoh_a, etoh_b, eco2_a, eco2_b, (int) stable);
//
//            /* Gửi qua UART3 */
//            g_uart3.p_api->write (g_uart3.p_ctrl, (uint8_t*) tx_buf, (uint32_t) len);
//
//            /* Delay nhỏ tránh flood UART */
//            R_BSP_SoftwareDelay (100, BSP_DELAY_UNITS_MILLISECONDS);
//        }
//    }
//
//#if BSP_TZ_SECURE_BUILD
//    R_BSP_NonSecureEnter();
//#endif
//}

/* ================================================================
 * Warm Start
 * ================================================================ */
/// Test case với file header
//volatile rm_zmod4xxx_iaq_2nd_data_t iaq_2nd_gen_data;
//volatile rm_hs300x_data_t hs300x_data;
///* Cờ báo hiệu 3 giây */
//volatile bool flag_trigger_3s = false;
//float temp_float=0;
//float hum_float=0;
//
///* Hàm ngắt Timer đếm hết 3 giây */
//void timer3s_callback(timer_callback_args_t * p_args)
//{
//    if (TIMER_EVENT_CYCLE_END == p_args->event)
//    {
//        flag_trigger_3s = true;
//    }
//}
//
///* UART3 callback */
//void uart3_callback(uart_callback_args_t * p_args)
//{
//    FSP_PARAMETER_NOT_USED(p_args);
//}
//
///* Testcase: with header file */
//void hal_entry(void)
//{
//    fsp_err_t err;
//
//    /* Gọi các hàm setup gốc từ zmod4410.h */
//    g_comms_i2c_bus0_quick_setup ();
//    g_zmod4xxx_sensor0_quick_setup ();
//    g_hs300x_sensor0_quick_setup ();
//
//    /* --- GIAI ĐOẠN 1: CHỜ DỮ LIỆU HỢP LỆ --- */
//    // Chương trình sẽ kẹt ở vòng lặp này CHỪNG NÀO nhiệt độ = 0 HOẶC độ ẩm = 0
//    do
//    {
//        g_hs300x_sensor0_quick_getting_humidity_and_temperature ((rm_hs300x_data_t*) &hs300x_data);
//
//        // Ép kiểu để lấy phần nguyên.
//        // (Lưu ý: Nếu struct của FSP trả về kiểu int32_t đã nhân 100, bạn có thể cần chia 100 ở đây: hs300x_data.temperature / 100)
//        temp_float = (float) hs300x_data.temperature.integer_part
//                + (float) hs300x_data.temperature.decimal_part / 100.0f;
//        hum_float = (float) hs300x_data.humidity.integer_part + (float) hs300x_data.humidity.decimal_part / 100.0f;
//        R_BSP_SoftwareDelay (50, BSP_DELAY_UNITS_MILLISECONDS);
//
//    }
//    while (hs300x_data.temperature.integer_part == 0 || hs300x_data.humidity.integer_part == 0);
//
//
//    /* Mở UART3 (TXD=P707, RXD=P706, 115200 baud) */
//    err = g_uart3.p_api->open(g_uart3.p_ctrl, g_uart3.p_cfg);
//    assert(FSP_SUCCESS == err);
//
//    /* Mở và bắt đầu Timer 0 đếm 3 giây */
//    err = g_timer0.p_api->open(g_timer0.p_ctrl, g_timer0.p_cfg);
//    assert(FSP_SUCCESS == err);
//    err = g_timer0.p_api->start(g_timer0.p_ctrl);
//    assert(FSP_SUCCESS == err);
//
//    while (1)
//    {
//        /* Khi Timer đếm đủ 3 giây, cờ được dựng lên */
//        if (flag_trigger_3s)
//        {
//            flag_trigger_3s = false;
//
//            /* Gọi hàm gốc để lấy dữ liệu */
//            bool stable = g_zmod4xxx_sensor0_quick_getting_iaq_2nd_gen_data(
//                    (rm_zmod4xxx_iaq_2nd_data_t*)&iaq_2nd_gen_data, temp_float, hum_float);
//
//            /* Map IAQ từ thang 1–5 sang 0–500 và xử lý số thực */
//            float iaq_mapped = (iaq_2nd_gen_data.iaq - 1.0f) / 4.0f * 500.0f;
//            float tvoc_mapped = iaq_2nd_gen_data.tvoc * 100.0f;
//            float etoh_mapped = iaq_2nd_gen_data.etoh * 100.0f;
//            float eco2_mapped = iaq_2nd_gen_data.eco2;
//            float rel_iaq_val = iaq_2nd_gen_data.rel_iaq; /* Biến rel_iaq thêm vào */
//
//            /* Integer math — tránh float printf */
//            int iaq_a = (int)(iaq_mapped);
//            int iaq_b = (int)(iaq_mapped * 100) % 100;
//            int tvoc_a = (int)(tvoc_mapped);
//            int tvoc_b = (int)(tvoc_mapped * 100) % 100;
//            int etoh_a = (int)(etoh_mapped);
//            int etoh_b = (int)(etoh_mapped * 100) % 100;
//            int eco2_a = (int)(eco2_mapped);
//            int eco2_b = (int)(eco2_mapped * 100) % 100;
//            int rel_a = (int)(rel_iaq_val);
//            int rel_b = (int)(rel_iaq_val * 100) % 100;
//
//            if (iaq_b < 0) iaq_b = -iaq_b;
//            if (tvoc_b < 0) tvoc_b = -tvoc_b;
//            if (etoh_b < 0) etoh_b = -etoh_b;
//            if (eco2_b < 0) eco2_b = -eco2_b;
//            if (rel_b < 0) rel_b = -rel_b;
//
//            char tx_buf[256];
//            int len = snprintf(tx_buf, sizeof(tx_buf),
//                                "{\"device_id\":\"ck_ra6m5_gateway_01\","
//                                "\"iaq\":%d.%02d,"
//                                "\"rel_iaq\":%d.%02d,"
//                                "\"tvoc\":%d.%02d,"
//                                "\"etoh\":%d.%02d,"
//                                "\"eco2\":%d.%02d,"
//                                "\"stable\":%d}\n",
//                                iaq_a, iaq_b, rel_a, rel_b, tvoc_a, tvoc_b, etoh_a, etoh_b, eco2_a, eco2_b, (int)stable);
//
//            /* Gửi qua UART3 */
//            g_uart3.p_api->write(g_uart3.p_ctrl, (uint8_t*)tx_buf, (uint32_t)len);
//
//            /* Delay 100ms đã được xóa bỏ để CPU rảnh tay cho AI */
//        }
//    }
//}

/// Test case dùng AI dự đoán tệp dữ liệu train
//#include "hal_data.h"
//#include <stdio.h>
//
//// Báo cho trình biên dịch biết hàm này được viết ở bên file C++
//extern float run_iaq_prediction(float* sensor_data);
//float predicted_iaq;
//void hal_entry(void) {
//    /* Các code khởi tạo UART, FSP của bạn nằm ở đây */
//
//    // Khởi tạo mảng chứa 6 thông số lấy chính xác từ ảnh của bạn
//    float test_features[6] = {
//        0.199f,
//        475.0f,
//        102.9f,
//        1.6f,
//        0.013f,
//        0.7f
//    };
//
//
//
//    while (1) {
//
//
//        // Bơm mảng dữ liệu vào hàm AI
//        predicted_iaq = run_iaq_prediction(test_features);
//
//        // Delay 2 giây
//        R_BSP_SoftwareDelay(2000, BSP_DELAY_UNITS_MILLISECONDS);
//    }
//}

/// Test case đọc nhiệt độ
///* TODO: Enable if you want to open HS300X */
//#define G_HS300X_SENSOR0_NON_BLOCKING (1)
//
//#if G_HS300X_SENSOR0_NON_BLOCKING
//volatile bool g_hs300x_completed = false;
//#endif
//
//#if RM_HS300X_CFG_PROGRAMMING_MODE
//uint32_t g_hs300x_sensor_id;
//#endif
//
///* TODO: Enable if you want to use a callback */
//#define G_HS300X_SENSOR0_CALLBACK_ENABLE (1)
//#if G_HS300X_SENSOR0_CALLBACK_ENABLE
//void hs300x_callback(rm_hs300x_callback_args_t * p_args)
//{
//#if G_HS300X_SENSOR0_NON_BLOCKING
//    if (RM_HS300X_EVENT_SUCCESS == p_args->event)
//    {
//        g_hs300x_completed = true;
//    }
//#else
//    FSP_PARAMETER_NOT_USED(p_args);
//#endif
//}
//#endif
//
///* Quick setup for g_hs300x_sensor0.
// * - g_comms_i2c_bus0 must be setup before calling this function
// *     (See Developer Assistance -> g_hs300x_sensor0 -> g_comms_i2c_device1 -> g_comms_i2c_bus0 -> Quick Setup).
// */
//void g_hs300x_sensor0_quick_setup(void);
//
///* Quick setup for g_hs300x_sensor0. */
//void g_hs300x_sensor0_quick_setup(void)
//{
//    fsp_err_t err;
//
//    /* Open HS300X sensor instance, this must be done before calling any HS300X API */
//    err = g_hs300x_sensor0.p_api->open(g_hs300x_sensor0.p_ctrl, g_hs300x_sensor0.p_cfg);
//    assert(FSP_SUCCESS == err);
//
//#if RM_HS300X_CFG_PROGRAMMING_MODE
//    /* Enter the programming mode. This must be called within 10ms after applying power. */
//    err = g_hs300x_sensor0.p_api->programmingModeEnter(g_hs300x_sensor0.p_ctrl);
//    assert(FSP_SUCCESS == err);
//
//#if G_HS300X_SENSOR0_NON_BLOCKING
//    while (!g_hs300x_completed)
//    {
//        ;
//    }
//    g_hs300x_completed = false;
//#endif
//
//    /* Delay 120us. Entering the programming mode takes 120us. */
//    R_BSP_SoftwareDelay(120, BSP_DELAY_UNITS_MICROSECONDS);
//
//    /* Get the sensor ID */
//    err = g_hs300x_sensor0.p_api->sensorIdGet(g_hs300x_sensor0.p_ctrl, (uint32_t *)&g_hs300x_sensor_id);
//    assert(FSP_SUCCESS == err);
//
//    /* Change the humidity resolution to 8 bit */
//    err = g_hs300x_sensor0.p_api->resolutionChange(g_hs300x_sensor0.p_ctrl, RM_HS300X_HUMIDITY_DATA, RM_HS300X_RESOLUTION_8BIT);
//    assert(FSP_SUCCESS == err);
//
//#if G_HS300X_SENSOR0_NON_BLOCKING
//    while (!g_hs300x_completed)
//    {
//        ;
//    }
//    g_hs300x_completed = false;
//#endif
//
//    /* Delay 14ms. Failure to comply with these times may result in data corruption and introduce errors in sensor measurements. */
//    R_BSP_SoftwareDelay(14, BSP_DELAY_UNITS_MILLISECONDS);
//
//    /* Change the temperature resolution to 8 bit */
//    err = g_hs300x_sensor0.p_api->resolutionChange(g_hs300x_sensor0.p_ctrl, RM_HS300X_TEMPERATURE_DATA, RM_HS300X_RESOLUTION_8BIT);
//    assert(FSP_SUCCESS == err);
//
//#if G_HS300X_SENSOR0_NON_BLOCKING
//    while (!g_hs300x_completed)
//    {
//        ;
//    }
//    g_hs300x_completed = false;
//#endif
//
//    /* Delay 14ms. Failure to comply with these times may result in data corruption and introduce errors in sensor measurements. */
//    R_BSP_SoftwareDelay(14, BSP_DELAY_UNITS_MILLISECONDS);
//
//    /* Exit the programming mode */
//    err = g_hs300x_sensor0.p_api->programmingModeExit(g_hs300x_sensor0.p_ctrl);
//    assert(FSP_SUCCESS == err);
//
//#if G_HS300X_SENSOR0_NON_BLOCKING
//    while (!g_hs300x_completed)
//    {
//        ;
//    }
//    g_hs300x_completed = false;
//#endif
//#endif
//}
//
//
///* Quick getting humidity and temperature values for g_hs300x_sensor0.
// * - g_hs300x_sensor0 must be setup before calling this function.
// */
//void g_hs300x_sensor0_quick_getting_humidity_and_temperature(rm_hs300x_data_t * p_data);
//
///* Quick getting humidity and temperature for g_hs300x_sensor0. */
//void g_hs300x_sensor0_quick_getting_humidity_and_temperature(rm_hs300x_data_t * p_data)
//{
//    fsp_err_t            err;
//    rm_hs300x_raw_data_t hs300x_raw_data;
//    bool is_valid_data = false;
//
//    /* Start the measurement */
//    err = g_hs300x_sensor0.p_api->measurementStart(g_hs300x_sensor0.p_ctrl);
//    assert(FSP_SUCCESS == err);
//#if G_HS300X_SENSOR0_NON_BLOCKING
//    while (!g_hs300x_completed)
//    {
//        ;
//    }
//    g_hs300x_completed = false;
//#endif
//
//    do
//    {
//        /* Read ADC data from HS300X sensor */
//        err = g_hs300x_sensor0.p_api->read(g_hs300x_sensor0.p_ctrl, &hs300x_raw_data);
//        assert(FSP_SUCCESS == err);
//#if G_HS300X_SENSOR0_NON_BLOCKING
//        while (!g_hs300x_completed)
//        {
//            ;
//        }
//        g_hs300x_completed = false;
//#endif
//
//        /* Calculate humidity and temperature values from ADC data */
//        err = g_hs300x_sensor0.p_api->dataCalculate(g_hs300x_sensor0.p_ctrl, &hs300x_raw_data, p_data);
//        if (FSP_SUCCESS == err)
//        {
//            is_valid_data = true;
//        }
//        else if (FSP_ERR_SENSOR_INVALID_DATA == err)
//        {
//            is_valid_data = false;
//        }
//        else
//        {
//            assert(false);
//        }
//    }
//    while (false == is_valid_data);
//}
//
//volatile rm_hs300x_data_t hs300x_data;
//float temp_float = 0;
//float hum_float = 0;
//
//void hal_entry(){
//    // Lưu ý: Nếu vẫn bị lỗi FSP_ERR_IN_USE, hãy comment dòng g_comms_i2c này lại nhé
//        g_comms_i2c_bus0_quick_setup();
//        g_hs300x_sensor0_quick_setup();
//
//
//
//        /* --- GIAI ĐOẠN 1: CHỜ DỮ LIỆU HỢP LỆ --- */
//        // Chương trình sẽ kẹt ở vòng lặp này CHỪNG NÀO nhiệt độ = 0 HOẶC độ ẩm = 0
//        do {
//            g_hs300x_sensor0_quick_getting_humidity_and_temperature((rm_hs300x_data_t *) &hs300x_data);
//
//            // Ép kiểu để lấy phần nguyên.
//            // (Lưu ý: Nếu struct của FSP trả về kiểu int32_t đã nhân 100, bạn có thể cần chia 100 ở đây: hs300x_data.temperature / 100)
//            temp_float = (float)hs300x_data.temperature.integer_part + (float)hs300x_data.temperature.decimal_part / 100.0f;
//            hum_float  = (float)hs300x_data.humidity.integer_part + (float)hs300x_data.humidity.decimal_part / 100.0f;
//            R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);
//
//        } while (hs300x_data.temperature.integer_part == 0 || hs300x_data.humidity.integer_part == 0);
//
//}

/// Test case có ring buffer trích đặc trưng
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
    fsp_err_t err;

    // Kéo chân P307 (RES_N) xuống mức LOW để ép ZMOD4410 reset phần cứng
    R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_03_PIN_07, BSP_IO_LEVEL_LOW);
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);
    // Kéo chân P307 lên mức HIGH để cảm biến thức dậy
    R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_03_PIN_07, BSP_IO_LEVEL_HIGH);
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


    /* Mở UART3 */
    err = g_uart3.p_api->open(g_uart3.p_ctrl, g_uart3.p_cfg);
    assert(FSP_SUCCESS == err);

    /* Mở và bắt đầu Timer 0 đếm 3 giây */
    err = g_timer0.p_api->open(g_timer0.p_ctrl, g_timer0.p_cfg);
    assert(FSP_SUCCESS == err);
    err = g_timer0.p_api->start(g_timer0.p_ctrl);
    assert(FSP_SUCCESS == err);

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
        g_uart3.p_api->write(g_uart3.p_ctrl, (uint8_t*) tx_buf, (uint32_t) len);
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

