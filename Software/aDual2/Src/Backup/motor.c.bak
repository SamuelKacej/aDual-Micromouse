/*
 * motor.c
 *
 *  Created on: Feb 25, 2020
 *      Author: xkacejs
 */


#include "motor.h"


void MOTOR_ControllerReset(PID_CONTROLLER* pid){


	pid->E = 0;
	pid->E_Prev = 0;
	pid->E_Sum = 0;

	pid->W = 0;
	pid->FB = 0;
	pid->U = 0;

	pid->b = 1;
	pid->c = 1;

}
void MOTOR_INIT(){
	// set structure of controllers

	// output is required current
	MOTOR_ControllerReset(&MOTOR_currentController[MOTOR_ML]);
	MOTOR_currentController[MOTOR_ML].Tx =0;
	MOTOR_currentController[MOTOR_ML].maxU = +255;
	MOTOR_currentController[MOTOR_ML].minU = -255;

	MOTOR_ControllerReset(&MOTOR_currentController[MOTOR_MR]);
	MOTOR_currentController[MOTOR_MR].Tx = 0;
	MOTOR_currentController[MOTOR_MR].maxU = +255;
	MOTOR_currentController[MOTOR_MR].minU = -255;

	// output is requred moment
	MOTOR_ControllerReset(&MOTOR_velocityController[MOTOR_ML]);
	MOTOR_velocityController[MOTOR_ML].Tx = 0;
	MOTOR_velocityController[MOTOR_ML].maxU = +0.005;
	MOTOR_velocityController[MOTOR_ML].minU = -0.005;

	MOTOR_ControllerReset(&MOTOR_velocityController[MOTOR_MR]);
	MOTOR_velocityController[MOTOR_MR].Tx = 0;
	MOTOR_velocityController[MOTOR_MR].maxU = +0.005;
	MOTOR_velocityController[MOTOR_MR].minU = -0.005;

}

void MOTOR_ControllerSet( PID_CONTROLLER* pid,\
			float P, float I, float D){
	pid->P = P;
	pid->I = I;
	pid->D = D;
	pid->E_Sum = 0;
}


volatile float MOTOR_ControllerUpdate( PID_CONTROLLER* pid, float requiredValue, float feedback){
	// return is otuput of PID

	pid->W	= requiredValue;
	pid->FB 	= feedback;
	pid->E = requiredValue - feedback;

	const float eP 	= pid->b*requiredValue - feedback;
	const float eD	= pid->c*requiredValue - feedback;
	pid->E_Sum = pid->E_Sum + pid->E;

	pid->U = pid->P*eP + pid->I*pid->E_Sum + pid->D*(eD - pid->E_Prev);
	pid->E_Prev = eD;

	// antiwind-up
	if( pid->U > pid->maxU ) {
		pid->E_Sum += (pid->maxU-pid->U)*pid->Tx;
		pid->U = pid->maxU;
	}else if( pid->U < pid->minU ){
		pid->E_Sum += (pid->minU-pid->U)*pid->Tx;
		pid->U = pid->minU;
	}

	return pid->U;




}

void MOTOR_SetVoltage(uint8_t motorSelect, int16_t voltage){
	// left motor = 0 ; right motor = 1
	// voltage  +-255

	// get absolute value
	uint8_t pwm = (voltage <0)? -voltage : voltage;
	pwm = (pwm>254)? 254:pwm;
	// In 100% will driver get stucked in OC protection


	// set direction
	if(motorSelect == MOTOR_ML){

		if(voltage <0)
			HAL_GPIO_WritePin(MOTOR_ML_DIR_PORT, MOTOR_ML_DIR_PIN, GPIO_PIN_RESET);
		else
			HAL_GPIO_WritePin(MOTOR_ML_DIR_PORT, MOTOR_ML_DIR_PIN, GPIO_PIN_SET);

	}else{

		if(voltage <0)
			HAL_GPIO_WritePin(MOTOR_MR_DIR_PORT, MOTOR_MR_DIR_PIN, GPIO_PIN_SET);
		else
			HAL_GPIO_WritePin(MOTOR_MR_DIR_PORT, MOTOR_MR_DIR_PIN, GPIO_PIN_RESET);

	}


	// set pwm;
	ACTUATOR_MotorSetPWM(motorSelect, pwm);
}

void MOTOR_SetMoment(uint8_t motorSelect, float requiredMoment ){
	// [Nm]

	const float requiredCurrent = requiredMoment*MOTOR_CONST_M_I;// unit is A

	float tmpU;
	if(requiredMoment == 0)
		tmpU = 0;
	else
		 tmpU = MOTOR_ControllerUpdate(&MOTOR_currentController[motorSelect], \
				requiredCurrent, ((float)SENSORS_motorI[motorSelect])/1000);	// mA->A

	MOTOR_SetVoltage(motorSelect, tmpU);


}

void MOTOR_SetVelocity(uint8_t motorSelect, float requiredVelocity){

	// TODO ADAPTIVE Controller

	MOTOR_ControllerUpdate(&MOTOR_velocityController[motorSelect], \
			requiredVelocity, SENSORS_velocity[motorSelect]);

	if(requiredVelocity == 0 ){
		MOTOR_velocityController[motorSelect].U = 0;

	}
	MOTOR_SetMoment(motorSelect, MOTOR_velocityController[motorSelect].U);

	//TOTO TU PADA ten printf
	/*if(motorSelect = 1)
	printf(" %i,\t %.2f, \t %.2f, \t %.2f, \t %.2f, \t %.2f\n\r",\
				MAIN_GetMicros()/1000, requiredVelocity,  MOTOR_velocityController[0].FB,\
				 MOTOR_velocityController[0].U*1000, MOTOR_currentController[0].FB*1000,\
				 MOTOR_currentController[0].U);
*/
	//MOTOR_SetVoltage(motorSelect, MOTOR_velocityController[motorSelect].U);

}

