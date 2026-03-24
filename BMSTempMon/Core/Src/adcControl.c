/*
 * adcControl.c
 *
 *  Created on: Mar 10, 2026
 *      Author: dodger
 */

#include <stdint.h>

#include "adcControl.h"

/**
 * @brief  Initializes the GPIO pins used for MUX select lines (MUX0, MUX1, MUX2).
 *         Call this once during setup, after MX_GPIO_Init().
 *         Update pin definitions in adcControl.h to match your hardware.
 */
void MUX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable clocks for MUX GPIO ports (add others if pins span multiple ports) */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin   = MUX0_GPIO_PIN | MUX1_GPIO_PIN | MUX2_GPIO_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(MUX0_GPIO_PORT, &GPIO_InitStruct);

    /* Default to channel 0 on startup */
    setMux(0);
}

/**
 * @brief  Sets the 3-bit MUX select lines (MUX0, MUX1, MUX2) to select
 *         the given channel on all 8x SN74LV4051 multiplexers simultaneously.
 *
 *         Channel mapping (SN74LV4051 truth table):
 *           MUX2 MUX1 MUX0 | Selected input
 *             0    0    0  |  Y0
 *             0    0    1  |  Y1
 *             0    1    0  |  Y2
 *             0    1    1  |  Y3
 *             1    0    0  |  Y4
 *             1    0    1  |  Y5
 *             1    1    0  |  Y6
 *             1    1    1  |  Y7
 *
 * @param  muxChannel  Channel to select (0–7). Values > 7 are masked to 3 bits.
 */
void setMux(uint8_t muxChannel) {
    muxChannel &= 0x07; /* Mask to 3 bits, just in case */

    HAL_GPIO_WritePin(MUX0_GPIO_PORT, MUX0_GPIO_PIN,
                      (muxChannel & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(MUX1_GPIO_PORT, MUX1_GPIO_PIN,
                      (muxChannel & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(MUX2_GPIO_PORT, MUX2_GPIO_PIN,
                      (muxChannel & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    /*
     * Small settling delay after switching MUX channel.
     * The SN74LV4051 has a typical propagation delay of ~10ns, but the analog
     * signal on the COM line may need longer to settle depending on source
     * impedance. 1ms is conservative and safe for most NTC thermistor setups.
     * Reduce if scan rate needs to be faster.
     */
    HAL_Delay(1);
}
