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

bool stable;
volatile float iaq_raw;

/* UART3 callback */
void uart3_callback(uart_callback_args_t * p_args)
{
	FSP_PARAMETER_NOT_USED(p_args);
}


void hal_entry(void)
{
	fsp_err_t err;

	/* 1. KHỞI TẠO J-LINK RTT */
	SEGGER_RTT_Init();
	SEGGER_RTT_WriteString(0, "\r\n--- KHOI DONG HE THONG THU THAP DU LIEU ---\r\n");
	// Header CSV để bạn dễ lưu file sau này
	SEGGER_RTT_WriteString(0, "IAQ,Rel_IAQ,TVOC,EtOH,eCO2,Temp,Hum,Stable\r\n");

	/* 2. RESET PHẦN CỨNG ZMOD4410 (Chân P307) */
	R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_03_PIN_07, BSP_IO_LEVEL_LOW);
	R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);
	R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_03_PIN_07, BSP_IO_LEVEL_HIGH);
	R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);

	/* 3. SETUP CÁC DRIVER FSP */
	g_comms_i2c_bus0_quick_setup();
	g_zmod4xxx_sensor0_quick_setup();
	g_hs300x_sensor0_quick_setup();

	/* 4. CHỜ DỮ LIỆU NHIỆT ẨM HỢP LỆ (Giai đoạn Warm-up HS3001) */
	do {
		g_hs300x_sensor0_quick_getting_humidity_and_temperature((rm_hs300x_data_t *)&hs300x_data);

		temp_float = (float)hs300x_data.temperature.integer_part + (float)hs300x_data.temperature.decimal_part / 100.0f;
		hum_float  = (float)hs300x_data.humidity.integer_part + (float)hs300x_data.humidity.decimal_part / 100.0f;

		R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);
	} while (hs300x_data.temperature.integer_part == 0 || hs300x_data.humidity.integer_part == 0);

	/* 5. KHỞI CHẠY TIMER 3 GIÂY */
	err = g_timer0.p_api->open(g_timer0.p_ctrl, g_timer0.p_cfg);
	assert(FSP_SUCCESS == err);
	err = g_timer0.p_api->start(g_timer0.p_ctrl);
	assert(FSP_SUCCESS == err);

	/* --- VÒNG LẶP CHÍNH --- */
	while (1)
	{

		/* Bước A: Đọc nhiệt ẩm thực tế */
		g_hs300x_sensor0_quick_getting_humidity_and_temperature((rm_hs300x_data_t *)&hs300x_data);
		temp_float = (float)hs300x_data.temperature.integer_part + (float)hs300x_data.temperature.decimal_part / 100.0f;
		hum_float  = (float)hs300x_data.humidity.integer_part + (float)hs300x_data.humidity.decimal_part / 100.0f;

		/* Giải phóng bus I2C ngắn hạn */
		R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);

		/* Bước B: Đọc dữ liệu khí ZMOD với bù trừ nhiệt ẩm */
		stable = g_zmod4xxx_sensor0_quick_getting_iaq_2nd_gen_data(
		(rm_zmod4xxx_iaq_2nd_data_t *)&iaq_2nd_gen_data, temp_float, hum_float);

		/* Bước C: Tách số nguyên/thập phân (4 chữ số) để in ra RTT */
		// IAQ (1.0 - 5.0)
		int iaq_a = (int)iaq_2nd_gen_data.iaq;
		int iaq_b = (int)(iaq_2nd_gen_data.iaq * 10000.0f + 0.5f) % 10000;

		// Rel IAQ
		int rel_a = (int)iaq_2nd_gen_data.rel_iaq;
		int rel_b = (int)(iaq_2nd_gen_data.rel_iaq * 10000.0f + 0.5f) % 10000;

		// TVOC
		int tvoc_a = (int)iaq_2nd_gen_data.tvoc;
		int tvoc_b = (int)(iaq_2nd_gen_data.tvoc * 10000.0f + 0.5f) % 10000;

		// EtOH
		int etoh_a = (int)iaq_2nd_gen_data.etoh;
		int etoh_b = (int)(iaq_2nd_gen_data.etoh * 10000.0f + 0.5f) % 10000;

		// eCO2
		int eco2_a = (int)iaq_2nd_gen_data.eco2;
		int eco2_b = (int)(iaq_2nd_gen_data.eco2 * 10000.0f + 0.5f) % 10000;

		// Temp & Hum
		int t_a = (int)temp_float;
		int t_b = (int)(temp_float * 100.0f + 0.5f) % 100;
		int h_a = (int)hum_float;
		int h_b = (int)(hum_float * 100.0f + 0.5f) % 100;

		/* Xử lý dấu âm cho phần thập phân nếu có */
		if (iaq_b < 0)  iaq_b  = -iaq_b;
		if (tvoc_b < 0) tvoc_b = -tvoc_b;
		if (etoh_b < 0) etoh_b = -etoh_b;
		if (eco2_b < 0) eco2_b = -eco2_b;

		/* Bước D: Xuất dữ liệu CSV ra J-Link RTT Viewer */
		char rtt_msg[256];
		snprintf(rtt_msg, sizeof(rtt_msg),
		"%d.%04d, %d.%04d, %d.%04d, %d.%04d, %d.%04d, %d.%02d, %d.%02d, %d\r\n",
		iaq_a, iaq_b, rel_a, rel_b, tvoc_a, tvoc_b, etoh_a, etoh_b, eco2_a, eco2_b,
		t_a, t_b, h_a, h_b, (int)stable);

		SEGGER_RTT_WriteString(0, rtt_msg);
	}
}