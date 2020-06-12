/*
 * actuator.c
 *
 *  Created on: Feb 13, 2020
 *      Author: xkacejs
 */

#include "actuator.h"




void ACTUATOR_Init(){
	HAL_TIM_PWM_Start(&ACT_HTIM_LED, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&ACT_HTIM_LED, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&ACT_HTIM_LED, TIM_CHANNEL_3);

	HAL_TIM_PWM_Start(&ACT_HTIM_BUZZER, TIM_CHANNEL_2);


	HAL_TIM_PWM_Start(&ACT_HTIM_MOTOR_L, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&ACT_HTIM_MOTOR_R, TIM_CHANNEL_1);

	ACTUATOR_BuzzerStop();
	ACTUATOR_LED(0, 10, 5);



}

void ACTUATOR_BuzzerStart(uint16_t freq){
	// in init it should be set to 100KHz counter
	// it is for CH2

	if(freq==0){
		ACTUATOR_BuzzerStop();
		return;
	}

	uint32_t period = 10000/freq;
	ACT_BUZZER_PERIOD = period;
	ACT_BUZZER_CCR = period /2; // 33% duty cycle

}

//
void ACTUATOR_BuzzerStop(){
	ACT_BUZZER_CCR = 0;
}

// value 0-255; if value<0 parameter is not changed
void ACTUATOR_LED(int16_t red, int16_t green, int16_t blue){
	//period should be 255


	if (red>=0)
		ACT_LED_CCR_RED = (ACT_LED_PERIOD * (255-red)) / 255;
	if (green>=0)
		ACT_LED_CCR_GREEN = (ACT_LED_PERIOD * (255-green)) / 255;
	if (blue>=0)
		ACT_LED_CCR_BLUE = (ACT_LED_PERIOD * (255-blue)) / 255;


}


void ACTUATOR_MotorSetPWM(uint8_t motorSelect, uint8_t dutyCycle){
	// motor select 'R' or 'L' motor
	// dutyCycle 0-255

	if(motorSelect == ACT_MOTOR_L){
		ACT_MOTOR_L_CCR =(ACT_MOTOR_L_PERIOD*dutyCycle)/255;
	}
	if(motorSelect == ACT_MOTOR_R){
		ACT_MOTOR_R_CCR =(ACT_MOTOR_R_PERIOD*dutyCycle)/255;
	}
}
