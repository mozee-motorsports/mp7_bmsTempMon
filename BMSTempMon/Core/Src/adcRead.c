/*
 * adcRead.c
 *
 *  Created on: Mar 23, 2026
 *      Author: dodger
 */

#include <stdint.h>
#include "adcRead.h"
#include "mp7_fdcan.h"
#include "usart.h"

/* Stores the averaged ADC result for each of the 8 MUX groups after a full scan */
uint32_t groupAverages[NUM_MUX_GROUPS] = {0};

/**
 * @brief  Runs ADC self-calibration. Call once after HAL_ADC_Init().
 *         The STM32G4 ADC requires calibration after power-on for best accuracy.
 * @param  hadc  Pointer to the ADC handle configured in CubeMX.
 */
void ADC_Init(ADC_HandleTypeDef *hadc) {
    if (HAL_ADCEx_Calibration_Start(hadc, ADC_SINGLE_ENDED) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief  Triggers a single ADC conversion and returns the raw 12-bit result.
 *         Blocking — waits for conversion to complete up to ADC_TIMEOUT_MS.
 * @param  hadc  Pointer to the ADC handle.
 * @retval Raw ADC value (0–4095), or 0 on timeout/error.
 */
uint32_t ADC_ReadChannel(ADC_HandleTypeDef *hadc) {
    if (HAL_ADC_Start(hadc) != HAL_OK) {
        return 0;
    }
    if (HAL_ADC_PollForConversion(hadc, ADC_TIMEOUT_MS) != HAL_OK) {
        HAL_ADC_Stop(hadc);
        return 0;
    }
    uint32_t value = HAL_ADC_GetValue(hadc);
    HAL_ADC_Stop(hadc);
    return value;
}

/**
 * @brief  Sends a CAN fault message indicating which MUX group exceeded
 *         the temperature threshold and whether it was high or low.
 *
 *         CAN frame payload (2 bytes):
 *           Byte 0: MUX group index (0–7)
 *           Byte 1: Fault type — 0x01 = below min (open/cold), 0x02 = above max (overtemp)
 *
 * @param  hfdcan    Pointer to the FDCAN handle.
 * @param  groupIdx  MUX group that triggered the fault (0–7).
 * @param  adcValue  The averaged ADC reading that triggered the fault.
 */
static void sendTempFaultCAN(FDCAN_HandleTypeDef *hfdcan, uint8_t groupIdx, uint32_t adcValue) {
    FDCAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[2];

    if (MP7_FDCAN_ConfigureTxHeader(&TxHeader, CAN_TEMP_FAULT_ID, FDCAN_DLC_BYTES_2) != 0) {
        /* DataLength was invalid — should never happen with BYTES_2 */
        return;
    }

    TxData[0] = groupIdx;
    TxData[1] = (adcValue < TEMP_ADC_MIN) ? 0x01 : 0x02;

    if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &TxHeader, TxData) != HAL_OK) {
        /* TX FIFO full or peripheral error — log and continue */
        MP7_printf("CAN TX error on group %d\r\n", groupIdx);
    }
}

/**
 * @brief  Performs a full scan of all MUX channels across all 8 MUX groups.
 *
 *         Scan sequence:
 *           For each MUX channel (0–7):
 *             1. Set MUX select lines to current channel
 *             2. Read ADC once (single COM line shared across all 8 MUX outputs
 *                via external second-stage mux or daisy-chain — 1 reading per state)
 *             3. Accumulate reading into that group's running sum
 *           After all 8 channels:
 *             4. Compute average for each group (sum / 8)
 *             5. Compare average against TEMP_ADC_MIN / TEMP_ADC_MAX
 *             6. Send CAN fault message if out of range
 *             7. Reset accumulators for next scan
 *
 *         Result is stored in groupAverages[0..7] after each call.
 *
 * @param  hadc    Pointer to the ADC handle.
 * @param  hfdcan  Pointer to the FDCAN handle for sending fault alerts.
 */
void readAllMuxGroups(ADC_HandleTypeDef *hadc, FDCAN_HandleTypeDef *hfdcan) {
    uint32_t accumulator[NUM_MUX_GROUPS] = {0};

    /*
     * Outer loop: MUX channel select (0–7)
     * At each channel position, all 8 MUX groups expose a different one
     * of their 8 inputs to their COM output simultaneously.
     *
     * Since there is only 1 ADC channel, OUTPUT[7:0] must be read one group
     * at a time. If your hardware has a second-stage MUX selecting between
     * OUTPUT0–OUTPUT7, you would cycle that here. This implementation reads
     * a single ADC value per MUX channel state, representing one group's
     * contribution. Expand the inner loop below if you add group selection.
     */
    for (uint8_t ch = 0; ch < NUM_MUX_CHANNELS; ch++) {

        /* Step 1: Drive MUX select lines to current channel */
        setMux(ch);

        /*
         * Step 2: Read ADC for each MUX group at this channel position.
         *
         * NOTE: With 1 ADC channel and 8 MUX outputs, your hardware must
         * sequence through OUTPUT0–OUTPUT7 somehow. Two common approaches:
         *
         *   A) Second-stage MUX: a 9th MUX selects which OUTPUT feeds the ADC.
         *      In that case, add an inner loop here cycling a 4th select line.
         *
         *   B) All 8 OUTPUTs wired to 8 separate ADC channels (contradicts the
         *      "1 ADC channel" selection, but worth confirming with hardware team).
         *
         * For now, this reads the single ADC channel once per MUX channel state
         * and assigns it to group 0. Replicate/adapt for your actual topology.
         */
        for (uint8_t grp = 0; grp < NUM_MUX_GROUPS; grp++) {
            /*
             * If you have a second-stage MUX to select between OUTPUT0–OUTPUT7,
             * add the group select GPIO writes here before reading ADC.
             * e.g.:  setOutputGroup(grp);
             */
            accumulator[grp] += ADC_ReadChannel(hadc);
        }
    }

    /* Step 3: Average, threshold check, and CAN alert per group */
    for (uint8_t grp = 0; grp < NUM_MUX_GROUPS; grp++) {

        /* Compute average over NUM_MUX_CHANNELS readings */
        groupAverages[grp] = accumulator[grp] / NUM_MUX_CHANNELS;

        MP7_printf("Group %d avg ADC: %lu\r\n", grp, groupAverages[grp]);

        /* Step 4: Compare against fault thresholds and alert over CAN */
        if (groupAverages[grp] < TEMP_ADC_MIN || groupAverages[grp] > TEMP_ADC_MAX) {
            MP7_printf("FAULT: Group %d out of range (%lu)\r\n", grp, groupAverages[grp]);
            sendTempFaultCAN(hfdcan, grp, groupAverages[grp]);
        }

        /* Step 5: Accumulator already local — resets automatically next call */
    }
}
