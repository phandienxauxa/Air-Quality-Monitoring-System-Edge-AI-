/*
 * sensor.h
 *
 *  Created on: 23 Apr 2026
 *      Author: admin
 */

#ifndef ZMOD4410_H_
#define ZMOD4410_H_

#include "hal_data.h"
#include <stdbool.h>

/* Khởi tạo bus I2C */
void g_comms_i2c_bus0_quick_setup(void);

/* Khởi tạo cảm biến ZMOD4410 */
void g_zmod4xxx_sensor0_quick_setup(void);

/* Hàm đọc và tính toán thuật toán IAQ 2nd Gen */
bool g_zmod4xxx_sensor0_quick_getting_iaq_2nd_gen_data(
    rm_zmod4xxx_iaq_2nd_data_t * p_gas_data,
    float temperature,
    float humidity);

#endif /* ZMOD4410_H_ */
