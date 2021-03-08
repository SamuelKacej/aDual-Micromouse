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
MAZE_ABSOLUTE_DIRECTION_T MOUSE_CellOrientation;

#define  MOUSE_CURRENT_INSTR INSTR_InstrList[MOTION_instrID]

uint8_t MOUSE_SearchRun( float avgVel, uint8_t , const uint8_t );

uint8_t MOUSE_ReturnToStart( float avgVel );

uint8_t MOUSE_SpeedRun( float avgVel, uint8_t , const uint8_t );

uint8_t MOUSE_ChcekForNewComand();

void MOUSE_Test();

void MAUSE_Square( const uint8_t finalDest );

CMD_WALLS_RELATIVE MOUSE_GetRelativeWalls();

#endif /* MOUSE_H_ */
