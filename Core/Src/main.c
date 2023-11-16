/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "driver.h"
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int32_t nc_temp=0;
int32_t nc_pres=0;
int32_t temp=0;
int32_t old_temp=0;
uint32_t pres=0;
extern uint8_t RxBuff[RX_BUFF_SIZE];
CAN_TxHeaderTypeDef pHeader;
uint8_t aData[2]={0x20,0x01};
uint32_t pTxMailbox;
uint32_t coef = 3;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART2_UART_Init();
	MX_CAN1_Init();
	MX_I2C1_Init();
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */

	HAL_CAN_Start(&hcan1);

	pHeader.StdId=0x61;
	//pHeader->ExtId=0x61;
	pHeader.IDE=CAN_ID_STD;
	pHeader.RTR=CAN_RTR_DATA;
	pHeader.DLC=2;
	pHeader.TransmitGlobalTime=DISABLE;


	if(checkID()==0){
		printf("Connection réussie\r\n");
	}

	if(BMP280_config()==0){
		printf("Configuration réussie\r\n");
	}
	BMP280_etalonnage();

	//HAL_Delay(1000);
	HAL_UART_Receive_IT(&huart1, RxBuff,RX_BUFF_SIZE);

	//HAL_CAN_AddTxMessage(&hcan1, pHeader, aData, pTxMailbox);

	nc_temp = BMP280_get_temp();
	temp=bmp280_compensate_T_int32(nc_temp);
	old_temp=temp;
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		//aData[1]=0x01;
		//HAL_CAN_AddTxMessage(&hcan1, &pHeader, aData, &pTxMailbox);

		//printf("%lu\r\n",(long unsigned)HAL_CAN_IsTxMessagePending(&hcan1, pTxMailbox));
		//HAL_Delay(2000);
		//aData[1]=0x00;
		//HAL_CAN_AddTxMessage(&hcan1, &pHeader, aData, &pTxMailbox);
		//HAL_Delay(2000);

		nc_temp = BMP280_get_temp();
		temp=bmp280_compensate_T_int32(nc_temp);
		//printf("T=%d%d.%d%d_C\r\n",(temp/1000)%10,(temp/100)%10,(temp/10)%10,temp%10);
		if(fabs(temp-old_temp)>40){

			if(temp>old_temp){//On bouge dans le sens 0x00 +
				aData[1]=0x00;
				aData[0]=(int)((temp-old_temp)/coef);
				HAL_CAN_AddTxMessage(&hcan1, &pHeader, aData, &pTxMailbox);
				//printf("adata0= %d\r\n",aData[0]);
				old_temp=temp;
			}
			else{//On bouge dans le sens 0x01 -
				aData[1]=0x01;
				aData[0]=(int)((old_temp-temp)/coef);
				HAL_CAN_AddTxMessage(&hcan1, &pHeader, aData, &pTxMailbox);
				//printf("adata0= %d\r\n",aData[0]);
				old_temp=temp;
			}
		}
		HAL_Delay(2000);

		//		        printf("ALED\r\n");
		//				nc_temp = BMP280_get_temp();
		//				nc_pres = BMP280_get_pres();
		//				temp=bmp280_compensate_T_int32(nc_temp);
		//				pres=bmp280_compensate_P_int64(nc_pres);
		//				printf("Température non compensée = %x\r\n",nc_temp);
		//				printf("Pression non compensée = %x\r\n",nc_pres);
		//				printf("Température compensée = %x\r\n",temp);
		//				printf("Pression compensée = %x\r\n",pres);
		//		        HAL_Delay(500);
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 180;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Activate the Over-Drive mode
	 */
	if (HAL_PWREx_EnableOverDrive() != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
