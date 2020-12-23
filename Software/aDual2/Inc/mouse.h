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

uint16_t MOUSE_CellPosition;
uint16_t MOUSE_CellPositionPrev;
CMD_ABSOLUTE_ROTATION_T MOUSE_CellOrientation; //TODO HIGH priority
uint8_t MOUSE_pathIdx;

#define MOUSE_DISTANCE_FROM_WALL 53 // keep this distance from L/R wall
#define MOUSE_WALL_DISTRANCE_TRESHOLD 100 //mm
/*
 *   y
 * 	|	6  5  4
 * 	|	 \ | /
 * 	|  7---+----3
 * 	|  	 / | \
 * 	|  	8  1  2
 *	0-------------> x
 */



uint8_t MOUSE_ReturnToStart( float avgVel );

uint8_t MOUSE_SearchRun( float avgVel );

uint8_t MOUSE_SpeedRun( float avgVel );

uint8_t MOUSE_ChcekForNewComand();

void MOUSE_PrepareToStart();

void MOUSE_CorrectionDiagonal();

void MOUSE_CorrectionForward();

void MOUSE_CorrectionRotation();

uint8_t MOUSE_WriteWalls(CMD_WALLS_RELATIVE wall);
uint8_t MOUSE_LookForWalls();
uint8_t MOUSE_isMouseInMiddleOfCell();

#endif /* MOUSE_H_ */
