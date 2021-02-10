/*
 * mouse.h
 *
 *  Created on: May 12, 2020
 *      Author: xkacejs
 */

#ifndef MOUSE_H_
#define MOUSE_H_

#include "main.h"
#include "instructions.h"
#include "command.h"
#include "maze.h"
#include "motionSystem.h"
#include "stdio.h"
#include "usart.h"
#include "mapping.h"
#include "correction.h"

uint16_t MOUSE_CellPosition;
//uint16_t MOUSE_CellPositionPrev; -no idea why is this used
MAZE_ABSOLUTE_DIRECTION_T MOUSE_CellOrientation;
uint8_t MOUSE_pathIdx;

#define  MOUSE_CURRENT_INSTR INSTR_InstrList[MOTION_instrID]

uint8_t MOUSE_ReturnToStart( float avgVel );

uint8_t MOUSE_SearchRun( float avgVel );

uint8_t MOUSE_SpeedRun( float avgVel );

uint8_t MOUSE_ChcekForNewComand();


void MAUSE_Square( const uint8_t finalDest );

#endif /* MOUSE_H_ */
