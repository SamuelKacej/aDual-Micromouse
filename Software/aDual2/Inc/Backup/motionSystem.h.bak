/*
 * motionSystem.h
 *
 *  Created on: Feb 25, 2020
 *      Author: xkacejs
 */

#ifndef MOTIONSYSTEM_H_
#define MOTIONSYSTEM_H_


/*
 * this lbr should :
 * 	periodicaly>
 * - take which instraction to use
 * - when to switch to next instraction
 * - read instraction and translate it to motor velocity
 * - priary use angular and translational velocity
 * - manage acceleration
 *
 *
 *	in free time
 *	send to tanslate half of list;
 * this can be done only if is calculating >second id in half
 *
 * */

#include "main.h"
#include "instructions.h"
#include "command.h"
#include "motor.h"

#define MOTION_RESERVE_DISTANCE // mm

PID_CONTROLLER MOTION_angularController;
PID_CONTROLLER MOTION_translationController;

float MOTION_ExternalAngCorrection;		// external correction,(for wall correction)
float MOTION_ExternalTransCorrection;
uint8_t MOTION_RequestNewInstruction; 	// external request
INSTR_INSTRUCTION* insList;

// TOTO ZMAZ TO JE LEN PRE TESTOVANIE
void MOTION_uTurnTest();

void MOTION_UpdateList();

void MOTION_ChcekForNewInstraction();

void MOTION_Init(float tP,float tI,float tD,float aP,float aI,float aD);

void MOTION_Update();

uint8_t MOTION_NewInstraction();

void MOTION_ProcessedInstraction(INSTR_INSTRUCTION* instrActual);

void MOTION_StepVelocity(INSTR_INSTRUCTION* instr, float* transVel, float* angularVel);

void MOTION_SetVelocity(float transV, float angularV);

void MOTION_MoveInstrId();

void MOTION_inPlaceRotation(int);

INSTR_INSTRUCTION* MOTION_GetNextInstruction(uint8_t idOffset);

uint8_t MOTION_instrID;		// points to actual instraction
uint32_t MOTION_uTimePeriod;	// micro seconds used for acceleration calc



#endif /* MOTIONSYSTEM_H_ */
