/*
 * correction.c
 *
 *  Created on: Feb 9, 2021
 *      Author: samok
 */

#include "correction.h"

#define _ABS(x) ( ((x)<0)? -x: x )

uint8_t gWallFilterRightLong = 10;
uint8_t gWallFilterRightShort = 10;
uint8_t gWallFilterLeftLong = 10;
uint8_t gWallFilterLeftShort = 10;
uint8_t gCorrAlreadyDone = 0;
int16_t gPrevWallFilterLeftDiff = 0;
int16_t gPrevWallFilterRightDiff = 0;

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

void CORR_CorenrFilterReset(){
	gWallFilterLeftLong = CORR_CORNER_DEFAULT_VAL;
	gWallFilterLeftShort = CORR_CORNER_DEFAULT_VAL;
	gWallFilterRightLong = CORR_CORNER_DEFAULT_VAL;
	gWallFilterRightShort = CORR_CORNER_DEFAULT_VAL;

	gPrevWallFilterLeftDiff = 0;
	gPrevWallFilterRightDiff = 0;
	gCorrAlreadyDone = 0;
}
void CORR_ForwardCorner(){
	/*
	 * Ak na dokoncenie FWD ti zostave menej ako 150mm
	 * 	pozri sa do mapy ci mas na jednej strane stenu (nesmie byt prazdna)
	 * 		ak ano tak pozoruj kedy nastane vyrazne znizenie  snimaca ukazujuceho na tu stranu
	 * 			ked nastane nastav INSTR->EndDist = CONST
	 * 			nastav slowDown o ten rozdiel o ktory to posunies (treba preratat na %)
	 * */

	if(gCorrAlreadyDone > 0){
		return;
	}
	// if next CMD is fwd dont do correction
	if(MOUSE_CURRENT_INSTR.next->command->cmd >= CMD_FWD0 && MOUSE_CURRENT_INSTR.next->command->cmd <= CMD_FWD15){
		return;
	}

	// is current instr FWD?
	if(MOUSE_CURRENT_INSTR.command->cmd >= CMD_FWD0 && MOUSE_CURRENT_INSTR.command->cmd <= CMD_FWD15){

		// remaining distance till end of instruction
		const float remainingDistance = MOUSE_CURRENT_INSTR.distEnd - SENSORS_transPos;// [mm]


		// are you in last 80mm?
		if( remainingDistance < 80){

			// ------------- is in map front wall? TODO
			if(1 || MOUSE_GetRelativeWalls().WALL.front ){ // during search run you might not know if there is wall

				const uint8_t frontDistance = (SENSORS_irDistance[0]+SENSORS_irDistance[5])/2;

				// make sure distance is filtered
				if(frontDistance < CORR_FWD_FRONT_TRESHOLD && frontDistance > CORR_FWD_FRONT_REMAINING_DIST){
					MOUSE_CURRENT_INSTR.distEnd = SENSORS_transPos + CORR_FWD_FRONT_REMAINING_DIST;
					gCorrAlreadyDone = 1;
					ACTUATOR_LED(0, 0, 150);
					printf("UPDATE F %u ; rem %f\r\n", CORR_FWD_FRONT_REMAINING_DIST, remainingDistance);
				}
			}
		}



		// TODO is there previos wall?
		// corrner
		if(remainingDistance > 5 && remainingDistance < 130){
			// ------------- is there non right wall?



			if( !MOUSE_GetRelativeWalls().WALL.right ){

				gWallFilterRightLong  += (SENSORS_irDistance[2] - gWallFilterRightLong ) / CORR_CORNER_LONG_COEF;
				gWallFilterRightShort += (SENSORS_irDistance[2] - gWallFilterRightShort) / CORR_CORNER_SHORT_COEF;
				const int16_t filterDiff = (int16_t)gWallFilterRightLong - (int16_t)gWallFilterRightShort;

				if(gPrevWallFilterRightDiff > 0 && filterDiff<0){
					const uint8_t stopsAtCenter = (MOUSE_CURRENT_INSTR.next->command->cmd == CMD_STOP)? +90: +0; //extra 90mm if you want end cmd at transition of the cells
					MOUSE_CURRENT_INSTR.distEnd = stopsAtCenter +  SENSORS_transPos + CORR_CORNER_REMAINING_DIST;
					gCorrAlreadyDone = 1;
					ACTUATOR_LED(0, 150, 0);
					printf("UPDATE R %u ; rem %f\r\n", stopsAtCenter+CORR_CORNER_REMAINING_DIST, remainingDistance);
				}
				gPrevWallFilterRightDiff = filterDiff;


			}


			// ------------- isn't in map left wall?
			if( !MOUSE_GetRelativeWalls().WALL.left ){


				gWallFilterLeftLong  += (SENSORS_irDistance[3] - gWallFilterLeftLong ) / CORR_CORNER_LONG_COEF;
				gWallFilterLeftShort += (SENSORS_irDistance[3] - gWallFilterLeftShort) / CORR_CORNER_SHORT_COEF;
				const int16_t filterDiff = (int16_t)gWallFilterLeftLong - (int16_t)gWallFilterLeftShort;



				if(gPrevWallFilterLeftDiff > 0 && filterDiff<0){
					const uint8_t stopsAtCenter = (MOUSE_CURRENT_INSTR.next->command->cmd == CMD_STOP)? +90: +0;//extra 90mm if you want end cmd at transition of the cells
					MOUSE_CURRENT_INSTR.distEnd = stopsAtCenter +  SENSORS_transPos + CORR_CORNER_REMAINING_DIST;
					printf("UPDATE L %u ; rem %f\r\n", stopsAtCenter+CORR_CORNER_REMAINING_DIST, remainingDistance);
					gCorrAlreadyDone = 1;
					ACTUATOR_LED(150, 0, 0);
				}
				gPrevWallFilterLeftDiff = (filterDiff == 0)? gPrevWallFilterLeftDiff : filterDiff; // there can by only + or -

			 }
		 }


	}
}
void CORR_PrepareToStart(){

	// TODO
	/*
	 * Funkcia sa narovna na stred bunky, 4x ( sa otoci sa o 90deg a zarovna )
	 * staci sa zarovnat na protilahlu stenu pri kazdej otocke, netreba podla bocnych stie
	 * sucasne korekcia vo viacercyh osiach potlaci vpliv rotacneho vychylenia
	 *
	 * Na konci musi mat robot orientaciu na North
	 */

	if(MOUSE_CellPosition == 0x00){ // mouse is in start cell

		//in start cell you must have 3 walls in shape of U

		const uint8_t walls = MAZE_maze[0].wall;

		for(uint8_t i=0 ; i<4 ; i++){
			if(MOUSE_CellOrientation & walls){ // is there wall?
				CORR_PerpendicularToForward();
			}else{
				break;
			}
			CORR_InPlace90R(800);
		}
	}



}


void CORR_InPlace90L(uint16_t avgVel){
	// This fcn rotate robot 90deg to the left  in place

	INSTR_ResetInstrList(INSTR_InstrList, INSTR_LIST_SIZE);
	MOTION_instrID = 0;

	int id = 0;
	MOTION_resetList(id);
	id++;

	INSTR_AddArc(&insList[id], -90, 0, avgVel, CMD_IP90L, 0);
	id++;

	MOTION_resetList(id);
	id++;

	INSTR_InstrListUsedInstr = 3;

	while(INSTR_InstrListUsedInstr!=MOTION_instrID)
		;// wait

	MOUSE_CellOrientation = CMD_directionRotate8(MOUSE_CellOrientation,(MAZE_ABSOLUTE_DIRECTION_T) 0b0010);


}

void CORR_InPlace90R(uint16_t avgVel){
	// This fcn rotate robot 90deg to the left  in place

	INSTR_ResetInstrList(INSTR_InstrList, INSTR_LIST_SIZE);
	MOTION_instrID = 0;

	int id = 0;
	MOTION_resetList(id);
	id++;

	INSTR_AddArc(&insList[id], 90, 0, avgVel, CMD_IP90L, 0);
	id++;

	MOTION_resetList(id);
	id++;

	INSTR_InstrListUsedInstr = id;

	while(INSTR_InstrListUsedInstr!=MOTION_instrID)
		;// wait

	MOUSE_CellOrientation = CMD_directionRotate8(MOUSE_CellOrientation,(MAZE_ABSOLUTE_DIRECTION_T) 0b1000);

}

void CORR_PerpendicularToForward(){
	// this function puts you perpendiculary to the frant wall
	// call it only when you are in stop;

	const uint8_t reqDistFromWall = 35; //mm
	float transError, angleError;
	float prevTransError = 0;
	float prevAngleError = 0;

	const uint32_t startTime = MAIN_GetMicros();

	do{
		const uint8_t distL = SENSORS_irDistance[5];
		const uint8_t distR = SENSORS_irDistance[0];

		transError = (distL+distR)/2 - reqDistFromWall;
		angleError = -(distL-distR); // 0  is req

		float transVel = 1.2  *(5*transError + 2*(transError - prevTransError));
		float angleVel = 1.0/10 *(angleError + 0.1*(angleError - prevAngleError));

		// saturation
		transVel = (transVel > 50)? 50 : ( (transVel < -50) ? -50 : transVel);
		angleVel = (angleVel > 6)? 6 : ( (angleVel < -6) ? -6 : angleVel);

		MOTION_ExternalTransCorrection = transVel;
		MOTION_ExternalAngCorrection   = angleVel;

		HAL_Delay(1);

		// do correction max 500ms
		if(startTime+5e5 < MAIN_GetMicros())
			break;

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


	//printf("> %u %u \r\n", disL, disR);

	//static uint32_t prevTime = 0;
	static int8_t orientationErrorPrev = 0;
	int8_t error = 0;


	const uint8_t distanceFromWall = 90; //mm
	if( disR < distanceFromWall && disL < distanceFromWall){
		error = disL - disR;
	}else if(disR < distanceFromWall){
		error = CORR_DISTANCE_FROM_WALL - disR;
	}else if(disL < distanceFromWall){
		error = disL - CORR_DISTANCE_FROM_WALL;
	}


	//const uint32_t time = MAIN_GetMicros();							time
	const float D_component = ((float)(error - orientationErrorPrev))/(2.5e-3) ;
	const float angleVel = (float)error/20.0 + 5e-4*D_component;


	// saturation
	const float max = 5;
	MOTION_ExternalAngCorrection = (angleVel > max)? max : ( (angleVel < -max) ? -max : angleVel);

	orientationErrorPrev = error;
	//prevTime = time;
}

uint8_t CORR_isPositionForSideCorrection(){

	if( MOUSE_CURRENT_INSTR.command->cmd == CMD_FWD1){
		const uint8_t cont = MOUSE_CURRENT_INSTR.continuance ;
		if( (cont > 5 && cont < 18) || (cont > 55 && cont < 75) ){
			return 1;
		}
	}else if(MOUSE_CURRENT_INSTR.command->cmd > CMD_FWD1 && MOUSE_CURRENT_INSTR.command->cmd <= CMD_FWD15){
		return 1;
	}
	return 0;

}

void CORR_CorrectionRotation(){
	// TODO: Correction with front or side wall during rotation
}

void CORR_Diagonal(){

}
