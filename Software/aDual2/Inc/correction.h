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




#define CORR_DISTANCE_FROM_WALL 56 // keep this distance from L/R wall

// when you see front wall in less then TRESHOLD, then travel just REMAINING_DIST
#define CORR_FWD_FRONT_TRESHOLD 80
#define CORR_FWD_FRONT_REMAINING_DIST 40

// default value at start of the filter
#define CORR_CORNER_DEFAULT_VAL 120
#define CORR_CORNER_LONG_COEF 30
#define CORR_CORNER_SHORT_COEF 5

#define CORR_CORNER_TRESHOLD 120
#define CORR_CORNER_REMAINING_DIST 10

void CORR_PrepareToStart();

// call only during STOP CMD and only if in front of you is wall
void CORR_PerpendicularToForward();

// call only during forward movement, sensors should not be next to
void CORR_ParallelToSide();

uint8_t CORR_isPositionForSideCorrection();

void CORR_Diagonal();

void CORR_CorrectionRotation();

uint8_t CORR_FindCornerInRotation();

void CORR_CorenrFilterReset();

void CORR_InPlace90R(uint16_t);

void CORR_InPlace90L(uint16_t);

#endif /* CORRECTION_H_ */
