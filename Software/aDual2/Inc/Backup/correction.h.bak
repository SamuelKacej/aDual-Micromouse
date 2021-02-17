/*
 * correction.h
 *
 *  Created on: Feb 9, 2021
 *      Author: samok
 */

#ifndef CORRECTION_H_
#define CORRECTION_H_

#include "main.h"
#include "instructions.h"
#include "command.h"
#include "maze.h"
#include "motionSystem.h"
#include "stdio.h"
#include "usart.h"
#include "mouse.h"




#define CORR_DISTANCE_FROM_WALL 53 // keep this distance from L/R wall


void CORR_PrepareToStart();

// call only during STOP CMD and only if in front of you is wall
void CORR_PerpendicularToForward();

// call only during forward movement, sensors should not be next to
void CORR_ParallelToSide();

uint8_t CORR_isPositionForSideCorrection();

void CORR_Diagonal();

void CORR_CorrectionRotation();

uint8_t CORR_FindCornerInRotation();

#endif /* CORRECTION_H_ */
