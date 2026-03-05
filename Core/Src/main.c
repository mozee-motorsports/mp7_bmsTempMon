/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <stdbool.h>
#include "stm32h7xx_hal.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define V_REF (float) 3.3
#define FEEDBACK (float) 120.0
#define THERM_B (float) 3380.0
#define R0 (float) 10000.0
#define T0 (float) 298.15
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

FDCAN_HandleTypeDef hfdcan1;

IWDG_HandleTypeDef hiwdg1;

/* USER CODE BEGIN PV */
uint32_t adc_value = 0;   // ADC value

float voltage = 0.0;      // Voltage corresponding to the ADC value
float resistance = 0.0;   // Resistance based on voltage
float temperature = 0.0;  // Temperature in Celsius
float total_temperature = 0.0;
float average_temperature = 0.0;
float min_temperature = 100.0;
float max_temperature = 0.0;
float temp_counter = 0.0;

temp_Lookup_Table vt_Lookup[33] = {
		{2.44, -40},
		{2.42, -35},
		{2.40, -30},
		{2.38, -25},
		{2.35, -20},
		{2.32, -15},
		{2.27, -10},
		{2.23, -5},
		{2.17, 0},
		{2.11, 5},
		{2.05, 10},
		{1.99, 15},
		{1.92, 20},
		{1.86, 25},
		{1.80, 30},
		{1.74, 35},
		{1.68, 40},
		{1.63, 45},
		{1.59, 50},
		{1.55, 55},
		{1.51, 60},
		{1.48, 65},
		{1.45, 70},
		{1.43, 75},
		{1.40, 80},
		{1.38, 85},
		{1.37, 90},
		{1.35, 95},
		{1.34, 100},
		{1.33, 105},
		{1.32, 110},
		{1.31, 115},
		{1.30, 120}
};
FDCAN_TxHeaderTypeDef tx_Header;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_IWDG1_Init(void);
/* USER CODE BEGIN PFP */
void sendCan(void);
HAL_StatusTypeDef CAN_init();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
HAL_StatusTypeDef CAN_init()
{
	hfdcan1.Instance = FDCAN1; // Use FDCAN1 or FDCAN2
	hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
	hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
	hfdcan1.Init.AutoRetransmission = ENABLE;
	hfdcan1.Init.TransmitPause = DISABLE;
	hfdcan1.Init.ProtocolException = DISABLE;

	hfdcan1.Init.NominalPrescaler = 8;
	hfdcan1.Init.NominalSyncJumpWidth = 1;
	hfdcan1.Init.NominalTimeSeg1 = 12;
	hfdcan1.Init.NominalTimeSeg2 = 2;

	hfdcan1.Init.DataPrescaler = 1;
	hfdcan1.Init.DataSyncJumpWidth = 1;
	hfdcan1.Init.DataTimeSeg1 = 1;
	hfdcan1.Init.DataTimeSeg2 = 1;

	if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
		return HAL_ERROR;

	FDCAN_FilterTypeDef sFilterConfig;
	sFilterConfig.IdType = FDCAN_EXTENDED_ID;
	sFilterConfig.FilterIndex = 0;
	sFilterConfig.FilterType = FDCAN_FILTER_MASK;
	sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	sFilterConfig.FilterID1 = 0x0000;
	sFilterConfig.FilterID2 = 0x0000;

	if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)
	    return HAL_ERROR;

	if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
	    return HAL_ERROR;

	uint32_t CAN_ID = 0x1839F380;

	tx_Header.Identifier = 0x1839F380;
	tx_Header.IdType = FDCAN_EXTENDED_ID;
	tx_Header.TxFrameType = FDCAN_DATA_FRAME;
	tx_Header.DataLength = FDCAN_DLC_BYTES_8;
	tx_Header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	tx_Header.BitRateSwitch = FDCAN_BRS_OFF;
	tx_Header.FDFormat = FDCAN_CLASSIC_CAN;
	tx_Header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	tx_Header.MessageMarker = 0;

	return HAL_OK;


//	debug_msg.StdId = 0x00;
//	debug_msg.ExtId = 0x7;
//	debug_msg.IDE = CAN_ID_EXT;
//	debug_msg.RTR = CAN_RTR_DATA;
//	debug_msg.DLC = 1;
//	debug_msg.TransmitGlobalTime = DISABLE;

}
void SetMuxChannel(uint8_t channel) {
    // Assuming each control pin is connected to one of the 3 GPIO pins
    HAL_GPIO_WritePin(muxA_GPIO_Port, muxA_Pin, (channel & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(muxB_GPIO_Port, muxB_Pin, (channel & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(muxC_GPIO_Port, muxC_Pin, (channel & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void setADCChannel(uint8_t channel) {
	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	sConfig.OffsetSignedSaturation = DISABLE;

	switch (channel) {
		case 0:
			sConfig.Channel = ADC_CHANNEL_3;
			break;
		case 1:
			sConfig.Channel = ADC_CHANNEL_4;
			break;
		case 2:
			sConfig.Channel = ADC_CHANNEL_9;
			break;
		case 3:
			sConfig.Channel = ADC_CHANNEL_10;
			break;
		case 4:
			sConfig.Channel = ADC_CHANNEL_11;
			break;
		case 5:
			sConfig.Channel = ADC_CHANNEL_14;
			break;
		case 6:
			sConfig.Channel = ADC_CHANNEL_15;
			break;
		case 7:
			sConfig.Channel = ADC_CHANNEL_16;
			break;
		default:
			Error_Handler();
			break;
	}

	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();  // Handle ADC configuration failure
	}
}

void Read_ADC(void) {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    adc_value = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    voltage = (adc_value * V_REF) / 65536.0;  // Convert ADC value to voltage
}

int8_t getClosestTemp(float voltage) {
    int16_t closestTemp = vt_Lookup[0].temperature;

    for (size_t i = 1; i < 33; i++) {
        if (vt_Lookup[i].voltage < voltage) {
            closestTemp = vt_Lookup[i].temperature;
            break;
        }
    }
    closestTemp = (closestTemp - 32) * 5 / 9;
    int8_t retTemp =  closestTemp & 0x00FF;
    return retTemp;
}

/*

float Get_Resistance(float voltage) {
    return FEEDBACK * (voltage / (V_REF - voltage));
}

float Get_Temperature(float resistance) {
    float temp_kelvin = 1.0 / ((1.0 / T0) + (1.0 / THERM_B) * log(resistance / R0));
    return temp_kelvin - 273.15;  // Convert to Celsius
}*/

void Calculate_Average_Temperature(void) {
    total_temperature = 0.0;
    temp_counter = 0;

    for(uint8_t adcChannel = 0; adcChannel < 8; adcChannel++) {
    	setADCChannel(adcChannel);

    	for (uint8_t muxChannel = 0; muxChannel < 8; muxChannel++) {
    		SetMuxChannel(muxChannel);  // Select mux channel (mux1 - mux8)

			Read_ADC();
			temperature = getClosestTemp(voltage);

        	if(adcChannel == 0 && (muxChannel == 1 || muxChannel == 6 || muxChannel == 7)){
        		continue; // Skip y1, y6, and y7 mux 0
        	}
        	if(adcChannel == 1){
        		continue; // skip mux 1
        	}
        	if(adcChannel == 2){
        		continue; // Skip mux 2
        	}
        	if(adcChannel == 3 && (muxChannel == 4)){
				continue; // skip y4 mux 3
			}
        	if(adcChannel == 4 && (muxChannel == 0)){
        		continue; // Skip y1 mux 4
        	}
        	if(adcChannel == 5 && (muxChannel == 7)){
				continue; // Skip y7 on mux 5
			}
        	if(adcChannel == 7 && (muxChannel > 3)){
        		continue; // skip last 4
        	}

			if(temperature < 0 ){
				continue;
			}


			if(temperature < min_temperature){
				min_temperature = temperature;
			}
			if(temperature > max_temperature){
				max_temperature = temperature;
			}
			total_temperature += temperature;
			temp_counter++;
		}
    }

    // Calculate the average temperature
    if (temp_counter < 1) {
		min_temperature = max_temperature = average_temperature = 0.0;
		temp_counter = 0;

	} else {
	    average_temperature = total_temperature / temp_counter;
	}
}

void sendCan(void){
    //uint8_t txData[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
	uint8_t txData[8];

    // Ensure temperatures stay within valid int8_t range
    int8_t min_temp = (int8_t)roundf(fmaxf(fminf(min_temperature, 127), -128));
    int8_t max_temp = (int8_t)roundf(fmaxf(fminf(max_temperature, 127), -128));
    int8_t avg_temp = (int8_t)roundf(fmaxf(fminf(average_temperature, 127), -128));

    txData[0] = 0x00;
    txData[1] = min_temp;
    txData[2] = max_temp;
    txData[3] = avg_temp;
    txData[4] = 0x28;
    txData[5] = 0x54;
    txData[6] = 0x01;
    txData[7] = txData[0] + txData[1] + txData[2] + txData[3] + txData[4] + txData[5] + txData[6] + 0x39 + 0x08;


    // Send CAN message
    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &tx_Header, txData) != HAL_OK) {
        Error_Handler(); // Handle transmission failure
    }
}

/*void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0) {
        FDCAN_RxHeaderTypeDef RxHeader;
        uint8_t RxData[8];

        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK) {

        }
    }
}*/
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

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
  MX_ADC1_Init();
  MX_FDCAN1_Init();
  MX_IWDG1_Init();
  /* USER CODE BEGIN 2 */
  if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDG1RST))
  {
      // WDT triggered the last reset
      __HAL_RCC_CLEAR_RESET_FLAGS();
      // Optional: indicate via LED or UART
  }

  if (CAN_init() != HAL_OK)
	  Error_Handler();
  //HAL_TIM_Base_Start_IT(&htim1);


/*  volatile bool ms200_flag = false;

  void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
  {
	  ms200_flag = true;

  }*/

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//	  for(int i = 0; i < 20000; i++){ }
	  //if(ms200_flag)
	  Calculate_Average_Temperature();
	  sendCan();
      HAL_IWDG_Refresh(&hiwdg1);               // Refresh watchdog before 3s
	  HAL_Delay(10);


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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_16B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.Oversampling.Ratio = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_11;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = ENABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 8;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 12;
  hfdcan1.Init.NominalTimeSeg2 = 2;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.MessageRAMOffset = 0;
  hfdcan1.Init.StdFiltersNbr = 1;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.RxFifo0ElmtsNbr = 0;
  hfdcan1.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxFifo1ElmtsNbr = 1;
  hfdcan1.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxBuffersNbr = 0;
  hfdcan1.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.TxEventsNbr = 0;
  hfdcan1.Init.TxBuffersNbr = 0;
  hfdcan1.Init.TxFifoQueueElmtsNbr = 32;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_12;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

/**
  * @brief IWDG1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG1_Init(void)
{

  /* USER CODE BEGIN IWDG1_Init 0 */

  /* USER CODE END IWDG1_Init 0 */

  /* USER CODE BEGIN IWDG1_Init 1 */

  /* USER CODE END IWDG1_Init 1 */
  hiwdg1.Instance = IWDG1;
  hiwdg1.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg1.Init.Window = 4095;
  hiwdg1.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG1_Init 2 */

  /* USER CODE END IWDG1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, muxC_Pin|muxB_Pin|muxA_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : muxC_Pin muxB_Pin muxA_Pin */
  GPIO_InitStruct.Pin = muxC_Pin|muxB_Pin|muxA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

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
