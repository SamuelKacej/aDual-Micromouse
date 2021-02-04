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

uint16_t MOUSE_CellPosition;
//uint16_t MOUSE_CellPositionPrev; -no idea why is this used
MAZE_ABSOLUTE_DIRECTION_T MOUSE_CellOrientation;
uint8_t MOUSE_pathIdx;

#define MOUSE_DISTANCE_FROM_WALL 53 // keep this distance from L/R wall
#define MOUSE_WALL_TRESHOLD_FRONT 95 //mm
#define MOUSE_WALL_TRESHOLD_SIDE 80 //mm

uint8_t MOUSE_ReturnToStart( float avgVel );

uint8_t MOUSE_SearchRun( float avgVel );

uint8_t MOUSE_SpeedRun( float avgVel );

uint8_t MOUSE_ChcekForNewComand();

void MOUSE_PrepareToStart();

void MOUSE_CorrectionDiagonal();

void MOUSE_CorrectionForward();

void MOUSE_CorrectionRotation();

void MOUSE_CorrectionSide();

uint8_t MOUSE_WriteWalls(CMD_WALLS_RELATIVE wall);

uint8_t MOUSE_LookForWalls();

uint8_t MOUSE_isMouseInMiddleOfCell();

uint8_t MOUSE_isPositionForSideCorrection();

void MOUSE_PrintMaze();

void MAUSE_Square( const uint8_t finalDest );

#endif /* MOUSE_H_ */
