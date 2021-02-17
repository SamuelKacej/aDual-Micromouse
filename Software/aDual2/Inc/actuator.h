/*
 * actuator.h
 *
 *  Created on: Feb 12, 2020
 *      Author: xkacejs
 */

#ifndef ACTUATOR_H_
#define ACTUATOR_H_

#include "stm32f4xx_hal.h"
#include "main.h"
#include "tim.h"

#define ACT_HTIM_LED htim1
#define ACT_HTIM_BUZZER htim4
#define ACT_HTIM_MOTOR_L htim9
#define ACT_HTIM_MOTOR_R htim14


#define ACT_LED_CCR_RED ACT_HTIM_LED.Instance->CCR1
#define ACT_LED_CCR_GREEN ACT_HTIM_LED.Instance->CCR2
#define ACT_LED_CCR_BLUE ACT_HTIM_LED.Instance->CCR3
#define ACT_LED_PERIOD ACT_HTIM_LED.Init.Period

#define ACT_BUZZER_CCR ACT_HTIM_BUZZER.Instance->CCR2
#define ACT_BUZZER_PERIOD ACT_HTIM_BUZZER.Instance->ARR//CNT
// ->CNT mozno nie je to co si myslis

#define ACT_MOTOR_L_PERIOD ACT_HTIM_MOTOR_L.Init.Period
#define ACT_MOTOR_L_CCR	ACT_HTIM_MOTOR_L.Instance->CCR1
#define ACT_MOTOR_L_DIR

#define ACT_MOTOR_R_PERIOD ACT_HTIM_MOTOR_R.Init.Period
#define ACT_MOTOR_R_CCR ACT_HTIM_MOTOR_R.Instance->CCR1
#define ACT_MOTOR_R_DIR

#define ACT_MOTOR_L 0
#define ACT_MOTOR_R 1

void ACTUATOR_Init();

//100-15000 Hz
void ACTUATOR_BuzzerStart(uint16_t freq);

//
void ACTUATOR_BuzzerStop();

// value 0-255; if value<0 parameter is not changed
void ACTUATOR_LED(int16_t red, int16_t green, int16_t blue);

//
void ACTUATOR_BuzzerStart(uint16_t);

//
void ACTUATOR_MotorSetPWM(uint8_t motorSelect, float duty);


#endif /* ACTUATOR_H_ */
