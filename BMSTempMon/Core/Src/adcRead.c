/*
 * adcRead.c
 *
 *  Created on: Mar 23, 2026
 *      Author: dodger
 *
 *  Updated: Revised readAllMuxGroups() to reconfigure the ADC channel
 *           for each MUX group, reflecting the confirmed hardware topology:
 *           OUTPUT[7:0] from the 8x SN74LV4051A MUXes are each buffered
 *           by a dedicated LMV324LIDT op-amp (U10/U11) and routed to
 *           separate ADC inputs ADC1–ADC8 (IN1–IN8) on the STM32G4.
 */

#include <stdint.h>
#include "adcRead.h"
#include "mp7_fdcan.h"
#include "usart.h"

/* Stores the averaged ADC result for each of the 8 MUX groups after a full scan */
uint32_t groupAverages[NUM_MUX_GROUPS] = {0};

/*
 * Maps MUX group index (0–7) to the corresponding STM32G4 ADC channel.
 *
 * Hardware topology (confirmed via schematic):
 *   Each SN74LV4051A COM output → LMV324LIDT unity-gain buffer → STM32G4 ADC pin
 *
 *   Group 0  →  ADC1  →  PA0   →  ADC_CHANNEL_1
 *   Group 1  →  ADC2  →  PA1   →  ADC_CHANNEL_2
 *   Group 2  →  ADC3  →  PA2   →  ADC_CHANNEL_3
 *   Group 3  →  ADC4  →  PA3   →  ADC_CHANNEL_4
 *   Group 4  →  ADC5  →  PB14  →  ADC_CHANNEL_5
 *   Group 5  →  ADC6  →  PC0   →  ADC_CHANNEL_6
 *   Group 6  →  ADC7  →  PC1   →  ADC_CHANNEL_7
 *   Group 7  →  ADC8  →  PC2   →  ADC_CHANNEL_8
 */
static const uint32_t groupChannelMap[NUM_MUX_GROUPS] = {
    ADC_CHANNEL_1,   /* Group 0 — ADC1, PA0  */
    ADC_CHANNEL_2,   /* Group 1 — ADC2, PA1  */
    ADC_CHANNEL_3,   /* Group 2 — ADC3, PA2  */
    ADC_CHANNEL_4,   /* Group 3 — ADC4, PA3  */
    ADC_CHANNEL_5,   /* Group 4 — ADC5, PB14 */
    ADC_CHANNEL_6,   /* Group 5 — ADC6, PC0  */
    ADC_CHANNEL_7,   /* Group 6 — ADC7, PC1  */
    ADC_CHANNEL_8,   /* Group 7 — ADC8, PC2  */
};

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
 * @brief  Reconfigures the ADC to the specified channel, then triggers a single
 *         blocking conversion and returns the raw 12-bit result.
 *
 *         Channel is reconfigured each call because all 8 MUX group outputs
 *         share a single ADC peripheral (ADC1), requiring software channel
 *         switching between groups.
 *
 *         Sampling time is set to 47.5 cycles — conservative but fast enough
 *         given the op-amp buffers drive the ADC input at low impedance.
 *         Increase further if noise is observed on high-resistance thermistors.
 *
 * @param  hadc     Pointer to the ADC handle.
 * @param  channel  ADC channel to read (e.g. ADC_CHANNEL_1).
 * @retval Raw ADC value (0–4095), or 0 on configuration/timeout error.
 */
uint32_t ADC_ReadChannel(ADC_HandleTypeDef *hadc, uint32_t channel) {
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel      = channel;
    sConfig.Rank         = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_47CYCLES_5; /* Safe margin with op-amp buffer */
    sConfig.SingleDiff   = ADC_SINGLE_ENDED;           /* NTC thermistors are single-ended */
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset       = 0;

    if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK) {
        return 0;
    }
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
 *         Hardware topology:
 *           - 8x SN74LV4051A MUXes share 3-bit select lines (MUX[2:0])
 *           - Each MUX COM output is buffered by a dedicated LMV324LIDT op-amp
 *           - Each op-amp output feeds a separate STM32G4 ADC input (ADC1–ADC8)
 *           - All 8 group signals are therefore readable in parallel at each
 *             MUX channel state, with no second-stage MUX required
 *
 *         Scan sequence:
 *           For each MUX channel (0–7):
 *             1. Set MUX select lines to select thermistor input [ch] on all groups
 *             2. For each group (0–7):
 *                  a. Reconfigure ADC1 to the group's dedicated channel (IN1–IN8)
 *                  b. Trigger a single conversion and accumulate the result
 *           After all 8 channels:
 *             3. Compute average per group (sum / NUM_MUX_CHANNELS)
 *             4. Compare against TEMP_ADC_MIN / TEMP_ADC_MAX
 *             5. Send CAN fault if out of range
 *
 *         Result is stored in groupAverages[0..7] after each call.
 *         Total conversions per scan: 8 channels × 8 groups = 64 ADC reads.
 *
 * @param  hadc    Pointer to the ADC handle (ADC1).
 * @param  hfdcan  Pointer to the FDCAN handle for sending fault alerts.
 */
void readAllMuxGroups(ADC_HandleTypeDef *hadc, FDCAN_HandleTypeDef *hfdcan) {
    uint32_t accumulator[NUM_MUX_GROUPS] = {0};

    for (uint8_t ch = 0; ch < NUM_MUX_CHANNELS; ch++) {

        /* Step 1: Drive MUX select lines to current thermistor input */
        setMux(ch);

        /*
         * Step 2: Read each group's dedicated ADC channel.
         *
         * At this MUX channel state, all 8 MUX groups simultaneously expose
         * thermistor input [ch] on their respective COM outputs. Each COM is
         * buffered by an op-amp and wired to a unique ADC input, so we
         * reconfigure ADC1 to each group's channel and read them in sequence.
         */
        for (uint8_t grp = 0; grp < NUM_MUX_GROUPS; grp++) {
            accumulator[grp] += ADC_ReadChannel(hadc, groupChannelMap[grp]);
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
    }
}
