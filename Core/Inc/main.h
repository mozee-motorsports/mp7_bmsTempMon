/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define muxC_Pin GPIO_PIN_0
#define muxC_GPIO_Port GPIOD
#define muxB_Pin GPIO_PIN_1
#define muxB_GPIO_Port GPIOD
#define muxA_Pin GPIO_PIN_2
#define muxA_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */
typedef enum {
	saftey_system = 0,
	broadcast = 1,
	throttle_control_board = 2,
	pedal_box = 3,
	steering_wheel = 4,
	thermo_control_board = 5

} Module;

typedef enum {
	to = 0, from = 1
} Direction;

typedef enum {
	shutdown = 0,
	status_report = 1,
	send_error = 2,
	throttle_percentage = 3
	// calibrate = 4
} Command;

typedef struct{
	FDCAN_RxHeaderTypeDef rx_header;
	uint8_t data[8];
}CANMessage;

typedef enum
{
  ok                  = 0x00U,
  generic             = 0x01U,
  mismatch_dlc        = 0x02U,
  invalid_command     = 0x03U,
  fdcan_init_failure  = 0x04U,
  fdcan_rx_failure    = 0x05U,
  fdcan_tx_failure    = 0x06U,
  adc_failure         = 0x07U,
}Error;
/* USER CODE END Private defines */

/* USER CODE BEGIN Prototypes */
HAL_StatusTypeDef fdcanWrite(FDCAN_HandleTypeDef* hfdcan, FDCAN_TxHeaderTypeDef txHeader, int8_t* txData, uint8_t len, Module module, Direction direction, uint8_t priority, Command command);
HAL_StatusTypeDef fdcanInit(FDCAN_HandleTypeDef* hfdcan);
HAL_StatusTypeDef fdcanFilterInit(FDCAN_HandleTypeDef* hfdcan, FDCAN_TxHeaderTypeDef* txHeader);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
