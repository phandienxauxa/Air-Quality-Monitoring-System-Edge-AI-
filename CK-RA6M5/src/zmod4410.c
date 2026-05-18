/*
 * zmod4410.c
 *
 *  Created on: 23 Apr 2026
 *      Author: admin
 */
#include "Include/zmod4410.h"
#include <stdio.h>
/* Các macro cấu hình nội bộ của ZMOD4410 */
#define G_ZMOD4XXX_SENSOR0_NON_BLOCKING    (1)
#define G_ZMOD4XXX_SENSOR0_IRQ_ENABLE      (0)  /* Disable IRQ */
#define G_ZMOD4XXX_DELAY_50                (50)
#define G_ZMOD4XXX_DELAY_3000              (3000)

/* Các cờ (flag) nội bộ phục vụ ngắt I2C */
#if G_ZMOD4XXX_SENSOR0_NON_BLOCKING
volatile bool                g_zmod4xxx_i2c_completed = false;
volatile rm_zmod4xxx_event_t g_zmod4xxx_i2c_callback_event;
#endif

#if G_ZMOD4XXX_SENSOR0_IRQ_ENABLE
volatile bool g_zmod4xxx_irq_completed = false;
#endif

/* =======================================================
 * CÁC HÀM CALLBACK CỦA CẢM BIẾN VÀ I2C
 * ======================================================= */
void zmod4xxx_comms_i2c_callback(rm_zmod4xxx_callback_args_t * p_args)
{
#if G_ZMOD4XXX_SENSOR0_NON_BLOCKING
    g_zmod4xxx_i2c_callback_event = p_args->event;
    if (RM_ZMOD4XXX_EVENT_ERROR != p_args->event)
    {
        g_zmod4xxx_i2c_completed = true;
    }
#else
    FSP_PARAMETER_NOT_USED(p_args);
#endif
}

void zmod4xxx_irq_callback(rm_zmod4xxx_callback_args_t * p_args)
{
#if G_ZMOD4XXX_SENSOR0_IRQ_ENABLE
    if (RM_ZMOD4XXX_EVENT_MEASUREMENT_COMPLETE == p_args->event)
    {
        g_zmod4xxx_irq_completed = true;
    }
#else
    FSP_PARAMETER_NOT_USED(p_args);
#endif
}

/* =======================================================
 * CÁC HÀM SETUP & ĐỌC DỮ LIỆU
 * ======================================================= */
void g_comms_i2c_bus0_quick_setup(void)
{
    fsp_err_t err;
    i2c_master_instance_t * p_driver_instance =
        (i2c_master_instance_t *) g_comms_i2c_bus0_extended_cfg.p_driver_instance;

    err = p_driver_instance->p_api->open(p_driver_instance->p_ctrl, p_driver_instance->p_cfg);
    assert(FSP_SUCCESS == err);
}

void g_zmod4xxx_sensor0_quick_setup(void)
{
    fsp_err_t err;
    err = g_zmod4xxx_sensor0.p_api->open(g_zmod4xxx_sensor0.p_ctrl, g_zmod4xxx_sensor0.p_cfg);
    assert(FSP_SUCCESS == err);
}

bool g_zmod4xxx_sensor0_quick_getting_iaq_2nd_gen_data(
    rm_zmod4xxx_iaq_2nd_data_t * p_gas_data,
    float temperature,
    float humidity)
{
    fsp_err_t              err;
    rm_zmod4xxx_raw_data_t zmod4xxx_raw_data;
    bool stabilization_complete = false;

    /* Clear flags */
#if G_ZMOD4XXX_SENSOR0_IRQ_ENABLE
    g_zmod4xxx_irq_completed = false;
#endif
#if G_ZMOD4XXX_SENSOR0_NON_BLOCKING
    g_zmod4xxx_i2c_completed = false;
#endif

    /* Start measurement */
    err = g_zmod4xxx_sensor0.p_api->measurementStart(g_zmod4xxx_sensor0.p_ctrl);
    assert(FSP_SUCCESS == err);
#if G_ZMOD4XXX_SENSOR0_NON_BLOCKING
    while (!g_zmod4xxx_i2c_completed) { ; }
    g_zmod4xxx_i2c_completed = false;
#endif

    /* Delay 3s cho IAQ 2nd Gen measurement cycle */
    R_BSP_SoftwareDelay(G_ZMOD4XXX_DELAY_3000, BSP_DELAY_UNITS_MILLISECONDS);

    /* Poll cho đến khi measurement xong */
    do
    {
#if G_ZMOD4XXX_SENSOR0_IRQ_ENABLE
        while (!g_zmod4xxx_irq_completed) { ; }
        g_zmod4xxx_irq_completed = false;
#else
        err = g_zmod4xxx_sensor0.p_api->statusCheck(g_zmod4xxx_sensor0.p_ctrl);
        assert(FSP_SUCCESS == err);
#if G_ZMOD4XXX_SENSOR0_NON_BLOCKING
        while (!g_zmod4xxx_i2c_completed) { ; }
        g_zmod4xxx_i2c_completed = false;
#endif
#endif

        /* Kiểm tra lỗi device */
        err = g_zmod4xxx_sensor0.p_api->deviceErrorCheck(g_zmod4xxx_sensor0.p_ctrl);
        assert(FSP_SUCCESS == err);
#if G_ZMOD4XXX_SENSOR0_NON_BLOCKING
        while (!g_zmod4xxx_i2c_completed) { ; }
        g_zmod4xxx_i2c_completed = false;

        if ((RM_ZMOD4XXX_EVENT_DEV_ERR_POWER_ON_RESET  == g_zmod4xxx_i2c_callback_event) ||
            (RM_ZMOD4XXX_EVENT_DEV_ERR_ACCESS_CONFLICT == g_zmod4xxx_i2c_callback_event))
        {
            while (1) { ; } /* Device error — cần reset */
        }
#endif

        /* Đọc ADC data */
        err = g_zmod4xxx_sensor0.p_api->read(g_zmod4xxx_sensor0.p_ctrl, &zmod4xxx_raw_data);
        if (FSP_ERR_SENSOR_MEASUREMENT_NOT_FINISHED == err)
        {
            R_BSP_SoftwareDelay(G_ZMOD4XXX_DELAY_50, BSP_DELAY_UNITS_MILLISECONDS);
        }
    }
    while (FSP_ERR_SENSOR_MEASUREMENT_NOT_FINISHED == err);
    assert(FSP_SUCCESS == err);

#if G_ZMOD4XXX_SENSOR0_NON_BLOCKING
    while (!g_zmod4xxx_i2c_completed) { ; }
    g_zmod4xxx_i2c_completed = false;
#endif

    /* Kiểm tra lỗi device lần 2 sau read */
    err = g_zmod4xxx_sensor0.p_api->deviceErrorCheck(g_zmod4xxx_sensor0.p_ctrl);
    assert(FSP_SUCCESS == err);
#if G_ZMOD4XXX_SENSOR0_NON_BLOCKING
    while (!g_zmod4xxx_i2c_completed) { ; }
    g_zmod4xxx_i2c_completed = false;

    if ((RM_ZMOD4XXX_EVENT_DEV_ERR_POWER_ON_RESET  == g_zmod4xxx_i2c_callback_event) ||
        (RM_ZMOD4XXX_EVENT_DEV_ERR_ACCESS_CONFLICT == g_zmod4xxx_i2c_callback_event))
    {
        while (1) { ; }
    }
#endif

    /* Set nhiệt độ & độ ẩm */
    err = g_zmod4xxx_sensor0.p_api->temperatureAndHumiditySet(
              g_zmod4xxx_sensor0.p_ctrl, temperature, humidity);
    assert(FSP_SUCCESS == err);

    /* Tính IAQ 2nd Gen */
    err = g_zmod4xxx_sensor0.p_api->iaq2ndGenDataCalculate(
              g_zmod4xxx_sensor0.p_ctrl, &zmod4xxx_raw_data, p_gas_data);

    if (FSP_SUCCESS == err)
        stabilization_complete = true;
    else if (FSP_ERR_SENSOR_IN_STABILIZATION == err || FSP_ERR_SENSOR_INVALID_DATA == err)
        stabilization_complete = false;
    else
        assert(false);

    return stabilization_complete;
}

