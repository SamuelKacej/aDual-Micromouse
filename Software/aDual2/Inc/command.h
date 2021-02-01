/*
 * command.h
 *
 *  Created on: Feb 19, 2020
 *      Author: xkacejs
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#include "main.h"
#include "maze.h"
#include "stdio.h"


typedef enum{

	CMD_STOP 		= 0,
	CMD_STRAIGHT 	= 0x10,
	CMD_TURN 		= 0x30,
	CMD_DIAGONAL 	= 0x40,
	CMD_TRANSITON 	= 0x60,

	// straight forward N cell
	CMD_FWD0		= CMD_STRAIGHT + 0,
	CMD_FWD1,
	CMD_FWD2,
	CMD_FWD3,
	CMD_FWD4,
	CMD_FWD5,
	CMD_FWD6,
	CMD_FWD7,
	CMD_FWD8,
	CMD_FWD9,
	CMD_FWD10,
	CMD_FWD11,
	CMD_FWD12,
	CMD_FWD13,
	CMD_FWD14,
	CMD_FWD15,

	// in place turn
	CMD_IP45R 		= CMD_TURN + 0,
	CMD_IP45L,
	CMD_IP90R,
	CMD_IP90L,
	CMD_IP135R,
	CMD_IP135L,
	CMD_IP180R,
	CMD_IP180L,

	// diagonal for N cell
	CMD_DIA0 		= CMD_DIAGONAL + 0,
	CMD_DIA1,
	CMD_DIA2,
	CMD_DIA3,
	CMD_DIA4,
	CMD_DIA5,
	CMD_DIA6,
	CMD_DIA7,
	CMD_DIA8,
	CMD_DIA9,
	CMD_DIA10,
	CMD_DIA11,
	CMD_DIA12,
	CMD_DIA13,
	CMD_DIA14,
	CMD_DIA15,
	CMD_DIA16,
	CMD_DIA17,
	CMD_DIA18,
	CMD_DIA19,
	CMD_DIA20,
	CMD_DIA21,
	CMD_DIA22,
	CMD_DIA23,
	CMD_DIA24,
	CMD_DIA25,
	CMD_DIA26,
	CMD_DIA27,
	CMD_DIA28,
	CMD_DIA29,
	CMD_DIA30,
	CMD_DIA31,

	// commands for connection between two command {STR,DIA}
	// Straight -> Straight
	CMD_SS90SR 		= CMD_TRANSITON + 0 , 	//smooth
	CMD_SS90SL,
	CMD_SS90ER,								//explore
	CMD_SS90EL,
	CMD_SS180R,								// U turn
	CMD_SS180L,
	CMD_SD45R,
	CMD_SD45L,
	CMD_SD135R,
	CMD_SD135L,
	CMD_DS45R,
	CMD_DS45L,
	CMD_DS135R,
	CMD_DS135L,
	CMD_DD90R,
	CMD_DD90L




} CMD_T;


#define CMD_LIST_SIZE 256

#define CMD_STATE_STOP 		0
#define CMD_STATE_START 	1
#define CMD_STATE_ORTHO		2
#define CMD_STATE_ORTHO_R	3
#define CMD_STATE_ORTHO_RR	4
#define CMD_STATE_ORTHO_L	5
#define CMD_STATE_ORTHO_LL	6
#define CMD_STATE_DIAG_LR	7
#define CMD_STATE_DIAG_LL	8
#define CMD_STATE_DIAG_RL	9
#define CMD_STATE_DIAG_RR	10


#define CELL_DIMENSION 180 //mm

// relative directions
// used for MAZE_ABSOLUTE -> CMD_RELATIVE

typedef enum{

	CMD_S =	0,	// stop
	CMD_R =	1,	// right
	CMD_F =	2,	// forward
	CMD_L =	3,	// left
	CMD_B = 4	// backward used only for translation

}CMD_DIRECTIONS_T;

typedef union{
struct{
	// TODO IMPLEMENT this
		uint8_t back : 1;
		uint8_t right : 1;
		uint8_t front : 1;
		uint8_t left : 1;

} WALL;
uint8_t wall;
}CMD_WALLS_RELATIVE;


typedef struct{
	CMD_T cmd;
	uint16_t dist; //mm

	// position and rotation after command will be executed
	MAZE_DIRECTIONS* path;


}CMD_COMMAND;


// list off command;
CMD_COMMAND CMD_commandList[CMD_LIST_SIZE];

// list of relative directions
CMD_DIRECTIONS_T CMD_directionList[CMD_LIST_SIZE];

// list of absolute direction to list of commands
void CMD_PathToCommand(CMD_DIRECTIONS_T*, CMD_COMMAND*, MAZE_DIRECTIONS* );


// convert
uint8_t CMD_DirectionMazeToCmd(uint8_t);

uint8_t CMD_RelativeWallToAbsolute(CMD_WALLS_RELATIVE dirRel, uint8_t rotation);
void CMD_AbsolutePathToRelative(MAZE_DIRECTIONS*, CMD_DIRECTIONS_T*, MAZE_ABSOLUTE_DIRECTION_T);

uint8_t CMD_DirectionRotate(int8_t dir,	int8_t* rotation);

// clear command list
void CMD_clearList();

uint8_t CMD_AbsoluteRotToDirection(MAZE_ABSOLUTE_DIRECTION_T absR);

#endif /* COMMAND_H_ */
