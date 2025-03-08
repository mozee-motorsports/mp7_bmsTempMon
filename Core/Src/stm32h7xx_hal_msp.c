/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32h7xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* USER CODE BEGIN Includes */
#include "stm32h7xx_hal_conf.h"
#include "stm32h7xx_it.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{

  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  /* System interrupt init*/

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/**
  * @brief ADC MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hadc: ADC handle pointer
  * @retval None
  */
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(hadc->Instance==ADC1)
  {
    /* USER CODE BEGIN ADC1_MspInit 0 */

    /* USER CODE END ADC1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInitStruct.PLL2.PLL2M = 4;
    PeriphClkInitStruct.PLL2.PLL2N = 10;
    PeriphClkInitStruct.PLL2.PLL2P = 2;
    PeriphClkInitStruct.PLL2.PLL2Q = 2;
    PeriphClkInitStruct.PLL2.PLL2R = 2;
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
    PeriphClkInitStruct.PLL2.PLL2FRACN = 0.0;
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_ADC12_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PC0     ------> ADC1_INP10
    PC1     ------> ADC1_INP11
    PA0     ------> ADC1_INP16
    PA2     ------> ADC1_INP14
    PA3     ------> ADC1_INP15
    PA6     ------> ADC1_INP3
    PC4     ------> ADC1_INP4
    PB0     ------> ADC1_INP9
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ADC1 interrupt Init */
    HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC_IRQn);
    /* USER CODE BEGIN ADC1_MspInit 1 */

    /* USER CODE END ADC1_MspInit 1 */

  }

}

/**
  * @brief ADC MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hadc: ADC handle pointer
  * @retval None
  */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance==ADC1)
  {
    /* USER CODE BEGIN ADC1_MspDeInit 0 */

    /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC12_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PC0     ------> ADC1_INP10
    PC1     ------> ADC1_INP11
    PA0     ------> ADC1_INP16
    PA2     ------> ADC1_INP14
    PA3     ------> ADC1_INP15
    PA6     ------> ADC1_INP3
    PC4     ------> ADC1_INP4
    PB0     ------> ADC1_INP9
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0);

    /* ADC1 interrupt DeInit */
    HAL_NVIC_DisableIRQ(ADC_IRQn);
    /* USER CODE BEGIN ADC1_MspDeInit 1 */

    /* USER CODE END ADC1_MspDeInit 1 */
  }

}

/**
  * @brief FDCAN MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hfdcan: FDCAN handle pointer
  * @retval None
  */
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* hfdcan)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(hfdcan->Instance==FDCAN1)
  {
    /* USER CODE BEGIN FDCAN1_MspInit 0 */

    /* USER CODE END FDCAN1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_FDCAN_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* FDCAN1 interrupt Init */
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
    /* USER CODE BEGIN FDCAN1_MspInit 1 */

    /* USER CODE END FDCAN1_MspInit 1 */

  }

}

/**
  * @brief FDCAN MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hfdcan: FDCAN handle pointer
  * @retval None
  */
void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* hfdcan)
{
  if(hfdcan->Instance==FDCAN1)
  {
    /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

    /* USER CODE END FDCAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_FDCAN_CLK_DISABLE();

    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* FDCAN1 interrupt DeInit */
    HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
    /* USER CODE BEGIN FDCAN1_MspDeInit 1 */

    /* USER CODE END FDCAN1_MspDeInit 1 */
  }

}

/* USER CODE BEGIN 1 */
#define MAX_3BIT 0b111
#define MAX_4BIT 0b1111
#define MAX_FDCAN_RETRIES 3
HAL_StatusTypeDef fdcanWrite(FDCAN_HandleTypeDef* hfdcan, FDCAN_TxHeaderTypeDef txHeader, int8_t* txData, uint8_t len,
		Module module, Direction direction, uint8_t priority, Command command) {
	// Verify that the actual payload (txData) is not greater than 8 bytes (max for classic CAN)
	if(len > FDCAN_DLC_BYTES_8)
		return HAL_ERROR;

	// Verify that priority, module, and direction are not greater than 3 bits, and command isn't greater than 4 bits
	if(priority > MAX_3BIT || module > MAX_3BIT || direction > MAX_3BIT || command > MAX_4BIT)
		return HAL_ERROR;

	// Set DLC
	txHeader.DataLength = len;

	// Create 11b id
	txHeader.Identifier = 0x1839F380;
	// Transmit message by putting it into 10 element TxFIFO queue
  int retry = 0;
  while (retry < MAX_FDCAN_RETRIES)
  {
    if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &txHeader, txData) == HAL_OK)
    {
      return HAL_OK; // Success
    }
    retry++;
    HAL_Delay(10); // Small delay before retrying
  }

  return HAL_ERROR;
}

HAL_StatusTypeDef fdcanInit(FDCAN_HandleTypeDef* hfdcan)
{
	// FDCAN 1 - Rx FIFO0
	if (HAL_FDCAN_Start(hfdcan) != HAL_OK)
		return HAL_ERROR;

	// Triggers interrupt when new message appears in RX_FIFO0
	if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,0) != HAL_OK)
		return HAL_ERROR;

	return HAL_OK;

}

// CANFD macros
#define FILTER 	0b00001000000		// What to look for i.e. id (0b10) and direction (0b0) XXX_010_X_XXXX										// Steering Wheel ID
#define MASK 	0b00011110000		// 11b id. Look at Module bits [7..5] and direction (should be set to "to" = 0)


HAL_StatusTypeDef fdcanFilterInit(FDCAN_HandleTypeDef* hfdcan, FDCAN_TxHeaderTypeDef* txHeader)
{
	FDCAN_FilterTypeDef fdcan_filter_config;

	// 11b register with 11b id. We only care about bits [7..5]
	// FDCAN1 Mask Filter
	fdcan_filter_config.IdType = FDCAN_EXTENDED_ID;						// Using standard IDs, not extended IDs
	fdcan_filter_config.FilterIndex = 0;								// We are only using 1 filter so index = 0
	fdcan_filter_config.FilterType = FDCAN_FILTER_MASK;					// Using mask filter
	fdcan_filter_config.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;			// Messages that pass through the filter should be sent to RX FIFO 0
	fdcan_filter_config.FilterID1 = FILTER;								// We only care about bits [7..5]
	fdcan_filter_config.FilterID2 = MASK;								// Mask bits
	if (HAL_FDCAN_ConfigFilter(hfdcan, &fdcan_filter_config) != HAL_OK)
	  return HAL_ERROR; // Filter configuration Error

	// Configure TX Header for FDCAN1
//	txHeader.Identifier = SW_MODULE;									// Recall header format - changed dynamically
	txHeader->IdType = FDCAN_STANDARD_ID;								// Using standard IDs, not extended IDs
	txHeader->TxFrameType = FDCAN_DATA_FRAME;							// Sending a Data frame not a Remote frame
//	txHeader.DataLength = FDCAN_DLC_BYTES_8;							// Data length is classic CAN - 8 bytes
	txHeader->ErrorStateIndicator = FDCAN_ESI_ACTIVE;					// Notify us if there is any error in transmission
	txHeader->BitRateSwitch = FDCAN_BRS_OFF;							// Will use same bit rate for both Arbitration and Data fields
	txHeader->FDFormat = FDCAN_CLASSIC_CAN;							// Using standard CAN not FDCAN
	txHeader->TxEventFifoControl = FDCAN_NO_TX_EVENTS;					// Not using TxEvent
	txHeader->MessageMarker = 0;										// Not using MessageMarker
	return HAL_OK;
}
/* USER CODE END 1 */

