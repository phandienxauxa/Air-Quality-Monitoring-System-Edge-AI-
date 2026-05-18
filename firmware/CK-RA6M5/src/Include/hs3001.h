/*
 * hs3001.h
 *
 *  Created on: 2 May 2026
 *      Author: admin
 */

#ifndef INCLUDE_HS3001_H_
#define INCLUDE_HS3001_H_

#include "hal_data.h"
#include <stdbool.h>

void g_hs300x_sensor0_quick_setup(void);
void g_hs300x_sensor0_quick_getting_humidity_and_temperature(rm_hs300x_data_t * p_data);

#endif /* INCLUDE_HS3001_H_ */
