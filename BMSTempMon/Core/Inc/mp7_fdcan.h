/*
 * mp7_fdcan.h
 *
 *  Library for setting up FDCAN for mp7 projects
 *
 *  Last Modified: Jan 21, 2026
 *      By Author: gundlachj
 *
 *  Created on: Jan 3, 2026
 *      Author: gundlachj
 */

#ifndef SRC_MP7_FDCAN_H_
#define SRC_MP7_FDCAN_H_

#include <stdint.h>

#if __has_include("stm32g4xx_hal.h")
#include "stm32g4xx_hal.h"
#elif __has_include("stm32h7xx_hal.h")
#include "stm32h7xx_hal.h"
#else
#error "HAL Library Files Not Found"
#endif

/*************************/
// CAN Messages
/*************************/
/*
 * CAN DLCs
 */
#define MP7_FDCAN_MAX_DLC             8

/*
 * CAN IDs
 */
// High Priority Range
#define MP7_FDCAN_HIGH_PRIORITY_MIN_ID  0x000
#define MP7_FDCAN_HIGH_PRIORITY_MAX_ID  0x3FF

// Error ID Range
#define MP7_FDCAN_ERROR_MIN_ID  0x000
#define MP7_FDCAN_ERROR_MAX_ID  0x1E5

// Normal ID Range
#define MP7_FDCAN_NORMAL_MIN_ID 0x1E6
#define MP7_FDCAN_NORMAL_MAX_ID 0x3CC

// Inverter IDs (offset of 0x3DD)
#define MP7_FDCAN_INVERTER_ID_MIN_ID  0x3DD
#define MP7_FDCAN_INVERTER_ID_MAX_ID  0x3FF

// Battery Management System (BMS) IDs
#define MP7_FDCAN_BMS_MIN_ID  0x3CD
#define MP7_FDCAN_BMS_MAX_ID  0x3DC

// Low Priority Range
#define MP7_FDCAN_LOW_PRIORITY_MIN_ID 0x400
#define MP7_FDCAN_LOW_PRIORITY_MAX_ID 0x7FF



HAL_StatusTypeDef MP7_FDCAN_ConfigureGlobalFilter(FDCAN_HandleTypeDef *hfdcan);

HAL_StatusTypeDef MP7_FDCAN_ConfigureFilter(FDCAN_HandleTypeDef *hfdcan,
                                            uint32_t FilterIDRangeLow,
                                            uint32_t FilterIDRangeHigh,
                                            uint32_t FilterIndex,
                                            uint32_t FilterDestinationConfig);

uint8_t MP7_FDCAN_ConfigureTxHeader(FDCAN_TxHeaderTypeDef *TxHeader,
                                 uint32_t Identifier,
                                 uint32_t DataLength);


#endif /* SRC_MP7_FDCAN_H_ */
