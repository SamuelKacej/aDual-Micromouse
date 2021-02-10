/*
 * correction.c
 *
 *  Created on: Feb 9, 2021
 *      Author: samok
 */

#include "correction.h"

#define _ABS(x) ( ((x)<0)? -x: x )


uint8_t CORR_FindCornerInRotation(){
	// this function will tell if robot will in future turn to right/left
	// return 0 -> nor right/ left turn
	// return 1 -> R
	// return 2 -> L


	// is whole list updated?
	if(INSTR_ListAlreadyUpdated == 1){

		const uint8_t arrayLength = INSTR_LIST_SIZE;

		//TODO not sure if start from 1 or 0, probably from 1
		for(uint8_t i = 1 ; i < arrayLength; i++){


			const CMD_T cmd = MOTION_GetNextInstruction(i)->command->cmd;


			// is instruaction rotate command?
			if(cmd >= CMD_TRANSITON && cmd <= CMD_DD90L){
				return 1 + (cmd%2);
				// return 1=R or 2=L

			}
		}
	}


	return 0;
	// in list is only forward/ diagonal/ stop => none turns

}


void CORR_ForwardCorner(){
	//TODO
	/*
	 * Ak na dokoncenie FWD ti zostave menej ako 150mm
	 * 	pozri sa do mapy ci mas na jednej strane stenu (nesmie byt prazdna)
	 * 		ak ano tak pozoruj kedy nastane vyrazne znizenie  snimaca ukazujuceho na tu stranu
	 * 			ked nastane nastav INSTR->EndDist = CONST
	 * 			nastav slowDown o ten rozdiel o ktory to posunies (treba preratat na %)
	 * */
}
void CORR_PrepareToStart(){

	// Nastav pociatocne natocenie podla prvej a druhej pozicie bunky v maze path


	const uint8_t  requireDirection = MAZE_path[0].absoluteDirection;

	// transform mouse 8 directional model, to maze 4 directional model
	const uint8_t actualDirection = 1 << ((MOUSE_CellOrientation-1)>>1);

	const int8_t difference = actualDirection - requireDirection ;

	switch(difference){
		case 0:
			return;
			break;

		case -7:
		case  4:
		case  2:
		case  1:
			//IP  right turn, -90
			MOTION_inPlaceRotation(-90);
			break;

		case  7:
		case -4:
		case -2:
		case -1:
			//IP left turn, 90
			MOTION_inPlaceRotation(90);
			break;

		case  6:
		case -6:
		case  3:
		case -3:
			// IP 180
			MOTION_inPlaceRotation(180);
			break;

	}



}

void CORR_PerpendicularToForward(){
	// this function puts you perpendiculary to the frant wall
	// call it only when you are in stop;

	const uint8_t reqDistFromWall = 35; //mm
	float transError, angleError;
	float prevTransError = 0;
	float prevAngleError = 0;

	do{
		const uint8_t distL = SENSORS_irDistance[5];
		const uint8_t distR = SENSORS_irDistance[0];

		transError = (distL+distR)/2 - reqDistFromWall;
		angleError = -(distL-distR); // 0  is req

		float transVel = 1.0/1  *(5*transError + 2*(transError - prevTransError));
		float angleVel = 1.0/30 *(0.5*angleError + 0.05*(angleError - prevAngleError));

		// saturation
		transVel = (transVel > 30)? 30 : ( (transVel < -30) ? -30 : transVel);
		angleVel = (angleVel > 6)? 6 : ( (angleVel < -6) ? -6 : angleVel);

		MOTION_ExternalTransCorrection = transVel;
		MOTION_ExternalAngCorrection   = angleVel;

		HAL_Delay(1);

	}while( _ABS(transError) > 5 || _ABS(angleError) > 3 );

	MOTION_ExternalTransCorrection = 0;
	MOTION_ExternalAngCorrection = 0;
}

void CORR_ParallelToSide(){
	// call this fcn when you are in command_finsh_cell

	// get finish address of actual cell
	//const uint8_t addr = INSTR_InstrList[MOTION_instrID].command->path->cell;

	//TODO: add existance of the wall from map

	// use [2] [3] or [1] [4]
	const uint8_t disR = SENSORS_irDistance[2];
	const uint8_t disL = SENSORS_irDistance[3];



	//static uint32_t prevTime = 0;
	//static int8_t orientationErrorPrev = 0;
	int8_t error = 0;


	const uint8_t distanceFromWall = 65; //mm
	if( disR < distanceFromWall && disL < distanceFromWall){
		error = disL - disR;
	}else if(disR < distanceFromWall){
		error = CORR_DISTANCE_FROM_WALL - disR;
	}else if(disL < distanceFromWall){
		error = disL - CORR_DISTANCE_FROM_WALL;
	}

	/*
	char s[30];
	const uint8_t len = sprintf(s, "%u, \t%u, \t%i, \t%.3f \r\n",disL, disR, error, MOTION_ExternalAngCorrection );
	HAL_UART_Transmit(&huart3, (uint8_t*)s, len, 1000);
	*/
	const uint32_t time = MAIN_GetMicros();
	//const float D_component = ((float)(error - orientationErrorPrev))/(time - prevTime) *1e6;
	const float angleVel = (float)error/53.0 ;//+ 5e-4*D_component;

	MOTION_ExternalAngCorrection = (angleVel > 3)? 3 : ( (angleVel < -3) ? -3 : angleVel);

	//orientationErrorPrev = error;
	//prevTime = time;
}

uint8_t CORR_isPositionForSideCorrection(){

	if( MOUSE_CURRENT_INSTR.command->cmd == CMD_FWD1){
		const uint8_t cont = MOUSE_CURRENT_INSTR.continuance ;
		if( (cont > 20 && cont < 30) || (cont > 60 && cont < 70) ){
			return 1;
		}
	}
	return 0;

}

void CORR_CorrectionRotation(){
	// TODO: Correction with front or side wall
}

void CORR_Diagonal(){

}
