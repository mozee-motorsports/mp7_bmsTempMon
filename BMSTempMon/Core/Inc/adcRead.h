/*
 * adcRead.h
 *
 *  Created on: Mar 23, 2026
 *      Author: dodger
 *
 *  Updated: Revised ADC_ReadChannel() signature to accept a channel parameter.
 *           Confirmed hardware topology (schematic): OUTPUT[7:0] from the 8x
 *           SN74LV4051A MUXes are each buffered by a dedicated LMV324LIDT
 *           op-amp (U10/U11) and routed to separate ADC inputs ADC1–ADC8
 *           (IN1–IN8) on the STM32G4. No second-stage MUX is present.
 */

#ifndef INC_ADCREAD_H_
#define INC_ADCREAD_H_

#include "stm32g4xx_hal.h"
#include "adcControl.h"

void readOut(uint8_t output);

/* -----------------------------------------------------------------------
 * ADC CONFIGURATION
 *
 * Architecture (confirmed via schematic):
 *   - 8x SN74LV4051A MUXes share the same MUX[2:0] select lines
 *   - Each MUX COM output is buffered by a dedicated LMV324LIDT op-amp
 *   - Each op-amp output feeds a unique STM32G4 ADC input (ADC1–ADC8)
 *   - All 8 group signals are readable at each MUX channel state
 *   - ADC1 is reconfigured in software to switch between IN1–IN8 per group
 *
 * Channel mapping:
 *   Group 0 → ADC1 → PA0  → ADC_CHANNEL_1
 *   Group 1 → ADC2 → PA1  → ADC_CHANNEL_2
 *   Group 2 → ADC3 → PA2  → ADC_CHANNEL_3
 *   Group 3 → ADC4 → PA3  → ADC_CHANNEL_4
 *   Group 4 → ADC5 → PB14 → ADC_CHANNEL_5
 *   Group 5 → ADC6 → PC0  → ADC_CHANNEL_6
 *   Group 6 → ADC7 → PC1  → ADC_CHANNEL_7
 *   Group 7 → ADC8 → PC2  → ADC_CHANNEL_8
 * ----------------------------------------------------------------------- */
#define ADC_TIMEOUT_MS          10      /* ms to wait for a single conversion (was 100 — reduced,
                                           actual conversion at 100MHz takes well under 1us) */

/* Number of MUX groups (one per SN74LV4051A, each with its own ADC input) */
#define NUM_MUX_GROUPS          8

/* Number of channels per MUX group (SN74LV4051A is 8-channel) */
#define NUM_MUX_CHANNELS        8

/* -----------------------------------------------------------------------
 * TEMPERATURE FAULT THRESHOLDS
 * Raw 12-bit ADC counts (0–4095). Must be integer values — the ADC returns
 * a uint32_t and all comparisons in adcRead.c use integer arithmetic.
 *
 * For NTC thermistors with a pull-up voltage divider:
 *   High resistance (cold) → higher voltage → higher ADC count
 *   Low resistance  (hot)  → lower voltage  → lower ADC count
 *
 * Therefore:
 *   TEMP_ADC_MIN = count at maximum temperature (hottest allowed, ~35°C)
 *   TEMP_ADC_MAX = count at minimum temperature (coldest allowed, ~0°C)
 *
 * Calculate using: ADC_count = (V_out / V_ref) * 4095
 * where V_out = V_ref * R_NTC / (R_fixed + R_NTC)
 * ----------------------------------------------------------------------- */
#define TEMP_ADC_MIN    2159    /* 1.74V (0 C) — lower voltage limit */
#define TEMP_ADC_MAX    2692    /* 2.17V (35 C) — upper voltage limit */

/* CAN message ID for temperature fault alert — update to your ID scheme */
#define CAN_TEMP_FAULT_ID       0x100

/* Stores one averaged ADC reading per MUX group after a full scan */
extern uint32_t groupAverages[NUM_MUX_GROUPS];

/**
 * @brief  Runs ADC self-calibration. Call once after HAL_ADC_Init().
 * @param  hadc  Pointer to the ADC handle configured in CubeMX.
 */
void ADC_Init(ADC_HandleTypeDef *hadc);

/**
 * @brief  Reconfigures ADC1 to the specified channel, triggers a single
 *         blocking conversion, and returns the raw 12-bit result (0–4095).
 *
 * @param  hadc     Pointer to the ADC handle (ADC1).
 * @param  channel  ADC channel to read (e.g. ADC_CHANNEL_1 through ADC_CHANNEL_8).
 * @retval Raw ADC value (0–4095), or 0 on configuration/timeout error.
 */
uint32_t ADC_ReadChannel(ADC_HandleTypeDef *hadc, uint32_t channel);

/**
 * @brief  Performs a full scan of all 8 MUX channels across all 8 MUX groups.
 *         Averages 8 readings per group and sends a CAN fault if any average
 *         falls outside [TEMP_ADC_MIN, TEMP_ADC_MAX].
 *         Results stored in groupAverages[0..7].
 *
 * @param  hadc    Pointer to the ADC handle (ADC1).
 * @param  hfdcan  Pointer to the FDCAN handle for fault transmission.
 */
void readAllMuxGroups(ADC_HandleTypeDef *hadc, FDCAN_HandleTypeDef *hfdcan);

#endif /* INC_ADCREAD_H_ */
