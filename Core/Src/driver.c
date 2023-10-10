/*
 * driver.c
 *
 *  Created on: Oct 10, 2023
 *      Author: Tomtogaz
 */
#include "driver.h"
#include "main.h"
#include "stdio.h"

extern I2C_HandleTypeDef hi2c1;

HAL_StatusTypeDef retour;

uint8_t BMP280_address = 0x77<<1;
uint8_t ID_address = 0xD0;
uint8_t ctrl_meas = 0xF4;
uint8_t buffer[1];

//uint8_t osrs_t = 0b010<<5;
//uint8_t osrs_p = 0b101<<2;
//uint8_t mode = 0b11;

uint8_t config = (0b010<<5)|(0b101<<2)|(0b11);
uint8_t calib = 0x88;
uint8_t temp_add = 0xFA;
uint8_t pres_add = 0xF7;

typedef uint32_t BMP280_S32_t;

int checkID(void){
	retour = HAL_I2C_Master_Transmit(&hi2c1,BMP280_address, &ID_address, 1, HAL_MAX_DELAY);
	if(retour != HAL_OK){
		printf("Problème avec l'I2C\r\n");
		return 1;
	}
	retour = HAL_I2C_Master_Receive(&hi2c1, BMP280_address, buffer, 1, HAL_MAX_DELAY);
	if(retour != HAL_OK){
		printf("Problème avec l'I2C\r\n");
		return 1;
	}
	printf("L'ID du capteur est 0x%x\r\n",buffer[0]);
	return 0;
}


int BMP280_config(void){
	uint8_t buf[2];
	buf[0]= ctrl_meas;
	buf[1]= config;
	retour = HAL_I2C_Master_Transmit(&hi2c1,BMP280_address, buf, 2, HAL_MAX_DELAY);
	if(retour != HAL_OK){
		printf("Problème avec l'I2C\r\n");
		return 1;
	}
	retour = HAL_I2C_Master_Receive(&hi2c1, BMP280_address, buffer, 1, HAL_MAX_DELAY);
	if(retour != HAL_OK){
		printf("Problème avec l'I2C\r\n");
		return 1;
	}

	if(buffer[0] == config){
		printf("La config reçue est bien reçue\r\n");
		return 0;
	}
	return 1;
}

void BMP_etallonage(void){
	uint8_t receive_buf[24];

	retour = HAL_I2C_Master_Transmit(&hi2c1,BMP280_address, &calib, 1, HAL_MAX_DELAY);
	if(retour != HAL_OK){
		printf("Problème avec l'I2C\r\n");
	}
	retour = HAL_I2C_Master_Receive(&hi2c1, BMP280_address, receive_buf, 24, HAL_MAX_DELAY);
	if(retour != HAL_OK){
		printf("Problème avec l'I2C\r\n");
	}
	printf("les valeurs de l'étallonage sont :\r\n");
	for(int i=0;i<24;i++){
		printf("calib %2d = 0x%x\r\n",i,receive_buf[i]);
	}
}

int BMP_get_temp(void){

	uint8_t receive_buf[3];
	retour = HAL_I2C_Master_Transmit(&hi2c1,BMP280_address, &temp_add, 1, HAL_MAX_DELAY);
	if(retour != HAL_OK){
		printf("Problème avec l'I2C\r\n");
	}
	retour = HAL_I2C_Master_Receive(&hi2c1, BMP280_address, receive_buf, 3, HAL_MAX_DELAY);
	if(retour != HAL_OK){
		printf("Problème avec l'I2C\r\n");
	}
	int nc_temp=receive_buf[0]<<12|receive_buf[1]<<4|receive_buf[2]>>4;
	return nc_temp;
}

int BMP_get_pres(void){

	uint8_t receive_buf[3];
	retour = HAL_I2C_Master_Transmit(&hi2c1,BMP280_address, &pres_add, 1, HAL_MAX_DELAY);
	if(retour != HAL_OK){
		printf("Problème avec l'I2C\r\n");
	}
	retour = HAL_I2C_Master_Receive(&hi2c1, BMP280_address, receive_buf, 3, HAL_MAX_DELAY);
	if(retour != HAL_OK){
		printf("Problème avec l'I2C\r\n");
	}
	int nc_pres=receive_buf[0]<<12|receive_buf[1]<<4|receive_buf[2]>>4;
	return nc_pres;
}

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegcC.
// t_fine carries fine temperature as global value
BMP280_S32_t t_fine;
BMP280_S32_t bmp280_compensate_T_int32(BMP280_S32_t adc_T)
{
	BMP280_S32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((BMP280_S32_t)dig_T1<<1))) * ((BMP280_S32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((BMP280_S32_t)dig_T1)) * ((adc_T>>4) - ((BMP280_S32_t)dig_T1))) >> 12) * ((BMP280_S32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}

BMP280_U32_t bmp280_compensate_P_int64(BMP280_S32_t adc_P)
{
	BMP280_S64_t var1, var2, p;
	var1 = ((BMP280_S64_t)t_fine) - 128000;
	var2 = var1 * var1 * (BMP280_S64_t)dig_P6;
	var2 = var2 + ((var1*(BMP280_S64_t)dig_P5)<<17);
	var2 = var2 + (((BMP280_S64_t)dig_P4)<<35);
	var1 = ((var1 * var1 * (BMP280_S64_t)dig_P3)>>8) + ((var1 * (BMP280_S64_t)dig_P2)<<12);
	var1 = (((((BMP280_S64_t)1)<<47)+var1))*((BMP280_S64_t)dig_P1)>>33;
	if (var1 == 0){
		return 0; // avoid exception caused by division by zero
	}
	p = 1048576-adc_P;
	p = (((p<<31)-var2)*3125)/var1;
	var1 = (((BMP280_S64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((BMP280_S64_t)dig_P8) * p) >> 19; p = ((p + var1 + var2) >> 8) + (((BMP280_S64_t)dig_P7)<<4);
	return (BMP280_U32_t)p;
}
