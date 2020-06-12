/*
 * instructions.h
 *
 *  Created on: Feb 20, 2020
 *      Author: xkacejs
 */

#ifndef INSTRUCTIONS_H_
#define INSTRUCTIONS_H_

#include "main.h"
#include "command.h"
#include "math.h"
// thil lbr translate command to motor instruction and sensor
// instructions should hav acceleration, statble, de accelerate


/*
 * Orthogonal movement stops at N border of cell
 * Orthogonal movement starts at border between two cells
 *
 * Diagonal movement stops at N diagonal of cell
 * Diagonal movement starts at diagonal of cell
 *
 * ___O___
 *|\     /|
 *| D   D |
 *|  \ /  |
 *O   X   O
 *|  / \  |
 *| D   D |
 *|/__O__\|
 * 											    <-
 * Positive angle orientation is anti clockwise __|
 *
 * In proccesing every instruction function will calculate
 *  if the function need to accelerate to get to the target in the time
 * */

#define INSTR_LIST_HALF_SIZE 16

#define CELL_DIMENSION 180 //mm
#define PI 3.1416
#define SQRT2 1.4142
#define MAX_INSTRACTION_PER_COMMAND 4
#define SIGNUM(a) (( (a)<0 )? -1 : 1)

uint16_t INSTR_VelocityPeak	 	;	// mm/s , < 1000 ; 3500 > Translational
uint16_t INSTR_AverageVelocity  ; 	// mm/s , <  250 ; 1500 > Translational

uint16_t INSTR_MaxTransAccel 	;	// mm/s/s    1 g =~ 9807mm/s/s

float INSTR_MaxAngVelocity 		;	// rad/s

typedef struct _INSTRUCTION_{

		CMD_T command;

		// you can't ACCELERATE and ROTATE in the same instruction !!!

	// TRANSLATION
		float 	dist;		//mm	, traveled distance
		float	speed;		//mm/s	, speed at the end of instruction
		float	accel;		//mm/s/s, const acceleration for instraction

		// ussed for synchronization
		float	distBegin;	// mm, dist at begin of instruction
		float	distEnd; 	// mm, distance at the end of instruction

	// ROTATION

		// sinusoidal movement
		float angle;		// rad	, angle to turn in instraction;

		//used for syncronization
		float angleBegin;	// rad	, total angle at the begin of instraction
		float angleEnd;		// rad 	, total angle at the ond of instraction

		float time;			// s	, used for inplace turns

}INSTR_INSTRUCTION;


INSTR_INSTRUCTION INSTR_InstrList[2*INSTR_LIST_HALF_SIZE];
uint8_t INSTR_InstrListUsedInstr[2];	//how many of InstrList is calculated
uint16_t INSTR_CommandListIndex;		// define from which ID wil be comands tgranslated
uint8_t INSTR_ListAlreadyUpdated;		// allows precalculation of list

void INSTR_FillHalfList( uint8_t );
	// motion control shoud call this function
	// to update half of the list (0 or 1)

uint16_t INSTR_CmdToInstr( CMD_T* , uint16_t* , INSTR_INSTRUCTION*);
	// wite max INSTR_LIST_SIZE to list
	// return number of written instractions

void INSTR_ResetInstrList(INSTR_INSTRUCTION* , uint16_t);

float INSTR_CalcAccel(uint16_t vStart, uint16_t vTarget, uint16_t distance);


void INSTR_AddArc(INSTR_INSTRUCTION* insList, int16_t angleDeg, uint16_t radius,\
		uint16_t transVelocity, CMD_T command );


#endif /* INSTRUCTIONS_H_ */