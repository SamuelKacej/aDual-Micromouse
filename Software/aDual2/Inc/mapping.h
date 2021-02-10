/*
 * mapping.h
 *
 *  Created on: Feb 9, 2021
 *      Author: samok
 */

#ifndef MAPPING_H_
#define MAPPING_H_

#include "main.h"
#include "instructions.h"
#include "command.h"
#include "maze.h"
#include "motionSystem.h"
#include "stdio.h"
#include "usart.h"

#define MAPPING_WALL_TRESHOLD_FRONT 120 //mm
#define MAPPING_WALL_TRESHOLD_SIDE 90 //mm

uint8_t MAPPING_isTimeToReadSideWall();

uint8_t MAPPING_isTimeToReadFrontWall();

uint8_t MAPPING_WriteWalls(CMD_WALLS_RELATIVE wall,uint8_t pos, MAZE_ABSOLUTE_DIRECTION_T absOrientation);

uint8_t MAPPING_LookForWalls( uint8_t sensorSelect);

uint8_t MAPPING_isPositionForSideCorrection();


void MAPPING_PrintMaze(uint16_t mousePos);

#endif /* MAPPING_H_ */
