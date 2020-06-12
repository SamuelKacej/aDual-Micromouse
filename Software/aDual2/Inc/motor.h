/*
 * motor.h
 *
 *  Created on: Feb 25, 2020
 *      Author: xkacejs
 */

#ifndef MOTOR_H_
#define MOTOR_H_


#include "main.h"
#include "actuator.h"
#include "sensors.h"

// constants are from datasheet to faulhaber1717-003
//				  I to M
#define MOTOR_CONST_I_M 0.00198		// Nm/A
#define MOTOR_CONST_M_I 505			// A/Nm
#define MOTOR_CONST_W_U 0.0002007	// V*min
#define MOTOR_CONST_U_W 4820		// 1/(min*V)

#define MOTOR_ML 0 // motor select
#define MOTOR_MR 1

#define MOTOR_ML_DIR_PORT M1_DIR_GPIO_Port
#define MOTOR_ML_DIR_PIN M1_DIR_Pin

#define MOTOR_MR_DIR_PORT M2_DIR_GPIO_Port
#define MOTOR_MR_DIR_PIN M2_DIR_Pin


typedef struct {


	// ISA form of PID controller

	float P, I, D; 	// controller constants

	// error
	float E;		// error input
	float E_Prev;	// error  in t-1, used for D calculation
	float E_Sum;	// sum of error from start, used for I calculation

	// I/O
	float W;		// input required value
	float FB;		// feedback value
	float U;		// output of controller, action variable


	// strength of P and D
	float b;		// Ep = b*w - fb strength of w
	float c;		// Ed = c*w - fb strength of w
					// for PID b=c=1

	// anti windup
	float Tx;
	float maxU, minU;

} PID_CONTROLLER;


PID_CONTROLLER MOTOR_currentController[2];	// A
PID_CONTROLLER MOTOR_velocityController[2]; // velocity is in mm/s

void MOTOR_INIT();

void MOTOR_SetVoltage(uint8_t, int16_t);

void MOTOR_SetMoment(uint8_t motorSelect, float requiredMoment);

// mm/s ; mS : 0=L, 1=R
void MOTOR_SetVelocity(uint8_t motorSelect, float requiredVelocity);

float MOTOR_ControllerUpdate( PID_CONTROLLER* pid, float requiredValue, float feedback);

void MOTOR_ControllerSet( PID_CONTROLLER*, float, float, float );


#endif /* MOTOR_H_ */
