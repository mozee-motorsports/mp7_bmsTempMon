/*
 * adcRead.h
 *
 *  Created on: Mar 23, 2026
 *      Author: dodger
 */

#ifndef INC_ADCREAD_H_
#define INC_ADCREAD_H_

void readOut(uint8_t output);

/* -----------------------------------------------------------------------
 * ADC CONFIGURATION
 * The single ADC input reads from the currently-selected MUX COM output.
 * Update ADC_CHANNEL to match your CubeMX ADC channel assignment.
 *
 * Architecture recap:
 *   - 8x SN74LV4051 MUXes share the same MUX[2:0] select lines
 *   - Each MUX has its own COM output → each connected to one ADC channel
 *   - Since OUTPUT[7:0] are all read per MUX select state, we need 8 ADC
 *     channels OR a second level of multiplexing feeding 1 ADC channel.
 *
 * Since you confirmed 1 ADC channel: OUTPUT[7:0] must be further muxed
 * externally before reaching the STM32 ADC pin. This driver reads that
 * single ADC channel once per MUX select state (8 reads per full scan).
 * ----------------------------------------------------------------------- */
#define ADC_TIMEOUT_MS          100

/* Number of MUX groups (one per SN74LV4051, U1–U8) */
#define NUM_MUX_GROUPS          8

/* Number of channels per MUX group (SN74LV4051 is 8-channel) */
#define NUM_MUX_CHANNELS        8

/* -----------------------------------------------------------------------
 * TEMPERATURE FAULT THRESHOLDS
 * Raw 12-bit ADC values (0–4095). Adjust to match your sensor calibration.
 * For NTC thermistors: higher temp = lower resistance = higher ADC reading
 * (assuming a pull-up divider). Swap min/max if your circuit is inverted.
 * ----------------------------------------------------------------------- */
#define TEMP_ADC_MIN            100     /* Below this = sensor open / too cold */
#define TEMP_ADC_MAX            3800    /* Above this = overtemp / sensor short */

/* CAN message ID for temperature fault alert — update to your ID scheme */
#define CAN_TEMP_FAULT_ID       0x100

/* Stores one averaged ADC reading per MUX group after a full scan */
extern uint32_t groupAverages[NUM_MUX_GROUPS];

void ADC_Init(ADC_HandleTypeDef *hadc);
uint32_t ADC_ReadChannel(ADC_HandleTypeDef *hadc);
void readAllMuxGroups(ADC_HandleTypeDef *hadc, FDCAN_HandleTypeDef *hfdcan);
#endif /* INC_ADCREAD_H_ */
