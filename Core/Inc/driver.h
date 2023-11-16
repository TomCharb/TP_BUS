/*
 * driver.h
 *
 *  Created on: Oct 10, 2023
 *      Author: Tomtogaz
 */

#ifndef INC_DRIVER_H_
#define INC_DRIVER_H_

#include "main.h"

typedef int32_t BMP280_S32_t; //typedef pour la correction des valeurs de température et de pression
typedef uint32_t BMP280_U32_t;
typedef uint64_t BMP280_U64_t;
typedef int64_t BMP280_S64_t;

#define RX_BUFF_SIZE 5

int checkID(void);
int BMP280_config(void);
void BMP280_etalonnage(void);
int BMP280_get_temp(void);
int BMP280_get_pres(void);
BMP280_U32_t bmp280_compensate_P_int64(BMP280_S32_t adc_P);
BMP280_S32_t bmp280_compensate_T_int32(BMP280_S32_t adc_T);
#endif /* INC_DRIVER_H_ */
