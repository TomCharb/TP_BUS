/*
 * driver.h
 *
 *  Created on: Oct 10, 2023
 *      Author: Tomtogaz
 */

#ifndef INC_DRIVER_H_
#define INC_DRIVER_H_


int checkID(void);
int BMP280_config(void);
void BMP_etallonage(void);
int BMP_get_temp(void);
int BMP_get_pres(void);

#endif /* INC_DRIVER_H_ */
