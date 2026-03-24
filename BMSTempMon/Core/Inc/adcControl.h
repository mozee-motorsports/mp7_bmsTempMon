/*
 * adcControl.h
 *
 *  Created on: Mar 10, 2026
 *      Author: dodger
 */

#ifndef INC_ADCCONTROL_H_
#define INC_ADCCONTROL_H_

/* put prototypes here
 */
void setMux(uint8_t muxValue);

/* -----------------------------------------------------------------------
 * MUX SELECT PIN MAPPING
 * Update these to match your actual STM32 pin assignments from CubeMX.
 * The schematic labels MUX0, MUX1, MUX2 are the 3-bit select lines
 * to all 8x SN74LV4051 multiplexers simultaneously.
 * ----------------------------------------------------------------------- */
#define MUX0_GPIO_PORT      GPIOB
#define MUX0_GPIO_PIN       GPIO_PIN_0

#define MUX1_GPIO_PORT      GPIOB
#define MUX1_GPIO_PIN       GPIO_PIN_1

#define MUX2_GPIO_PORT      GPIOB
#define MUX2_GPIO_PIN       GPIO_PIN_2

/* Number of MUX channels (SN74LV4051 is 8-channel, 3-bit select) */
#define MUX_NUM_CHANNELS    8

void MUX_GPIO_Init(void);
void setMux(uint8_t muxChannel);

#endif /* INC_ADCCONTROL_H_ */
