/*
 * mouse.c
 *
 *  Created on: May 12, 2020
 *      Author: xkacejs
 */


#include "mouse.h"

uint8_t orientationErrorPrev = 0;
CMD_T MOUSE_LastCMD;

uint8_t MOUSE_SearchRun( float avgVel ){
	/*
	 * return 0 = robot is at the begin
	 * return x = robot is cant find way;
	 *
	 * Robot must be at the begin of the maze
	 * avgVel is average translation velocity of the robot (~500mm/s)
	 *
	 *  Robot write walls in the cell
	 *  make floodfill
	 *  move one cell
	 *  repaet until u are in the finish
	 *
	 *  same process but from finish to start
	 */



	uint16_t iterrationCnt = 0;

	uint8_t actualPosition = 0x00;


	while(1){

		 MAZE_updatePath(actualPosition, 0x88); // 77, 78, 87, 88 are finsh cells
		 CMD_AbsolutePathToRelative(MAZE_path, CMD_directionList);
		 CMD_directionList[1] = CMD_S;
		 CMD_PathToCommand(CMD_directionList, CMD_commandList);

		 INSTR_AverageVelocity = avgVel;
		 MOUSE_PrepareToStart();

		 MOUSE_ChcekForNewComand();

		 while(!MOUSE_ChcekForNewComand()){

			 //TODO : crash detection
			 MOTION_UpdateList();


		 }

		 //finish
		 if(     MOUSE_CellPosition == 0x77 ||\
				 MOUSE_CellPosition == 0x78 ||\
				 MOUSE_CellPosition == 0x87 ||\
				 MOUSE_CellPosition == 0x88 ){

			 ACTUATOR_LED(-1, -1, 200);
			 HAL_Delay(2000);
			 ACTUATOR_LED(-1, -1, 0);
			 break;
		 }
	}



	return 0;

}

uint8_t MOUSE_SpeedRun( float avgVel ){
	/*
	 * Robot must be at the begin of the maze
	 * avgVel is average ttranslation velocity of the robot (~500mm/s)
	 */

	 MAZE_updatePath(0x00, 0x88); // 77, 78, 87, 88 are finsh cells
	 CMD_AbsolutePathToRelative(MAZE_path, CMD_directionList);
	 CMD_PathToCommand(CMD_directionList, CMD_commandList);

	 INSTR_AverageVelocity = avgVel;
     MOUSE_PrepareToStart();

	 while(1){

		 //TODO : crash detection
		 MOUSE_ChcekForNewComand();


		 MOTION_UpdateList();

		 //finish
		 if(     MOUSE_CellPosition == 0x77 ||\
				 MOUSE_CellPosition == 0x78 ||\
				 MOUSE_CellPosition == 0x87 ||\
				 MOUSE_CellPosition == 0x88 ){

			 ACTUATOR_LED(-1, -1, 200);
			 HAL_Delay(2000);
			 ACTUATOR_LED(-1, -1, 0);
			 break;
		 }

	 }


}

uint8_t MOUSE_ReturnToStart( float avgVel ){
	/*
	 * Moves robot from current cell to begin
	 *
	 */


	MAZE_updatePath(MOUSE_CellPosition, 0x00);
	CMD_AbsolutePathToRelative(MAZE_path, CMD_directionList);
	CMD_PathToCommand(CMD_directionList, CMD_commandList);

	INSTR_AverageVelocity = avgVel;
	MOUSE_PrepareToStart();

	while(1){

		MOTION_UpdateList();
		MOUSE_ChcekForNewComand();


		//u are at the start
		if(MOUSE_CellPosition == 0x00 ){

			ACTUATOR_LED(-1, -1, 200);
			HAL_Delay(1500);
			ACTUATOR_LED(-1, -1, 0);
			break;
		 }

	 }

}

uint8_t MOUSE_UpdateAbsoluOrientation(){

	/*
	 * This function update orientation in cell
	 * and update actuall cell position of mouse in maze
	 * return 0 if data was not updated
	 * return 1 if data was updated
	 */



	if (insList[MOTION_instrID].command != MOUSE_LastCMD){
		MOUSE_LastCMD = insList[MOTION_instrID].command;
		MOUSE_pathIdx++;
		MOUSE_CellPositionPrev = MOUSE_CellPosition;
		MOUSE_CellPosition = MAZE_path[MOUSE_pathIdx].cell->address;
		MOUSE_CellOrientation = MAZE_path[MOUSE_pathIdx].absoluteDirection;
		return 1;
	}

	return 0;
}

uint8_t MOUSE_ChcekForNewComand(){

	 /*
	  * return 0 if cmd was not updated
	  * return 1 if cmd was updated
	 */
	return MOUSE_UpdateAbsoluOrientation();
	// TODO
	/* Tato funkcia sa bude zaoberat presnim ukonceninm CMD, by CMD skoncil PRESNE na hrane.
	 * Beznej je to priblizne na hrane, an toto potrebujem merania v bludisku
	 * */


	//pozrisa aky je comand
	//pozri ci je vzdialenost od end distance 18 cm pred koncom
	//pozri sa do mapy a hladaj roh
	// ak je nasledujuci cmd STOP
	// tak zastav v strede bunky
	// v inom pripade hladaj


	// if next cmd is stop


	// forward



	// diagonal


	// jednasa o kratke vzdialenosti takze tam sa velka chyba nenaintegruje
	// zatial moze zostat bez presneho prepinania

	// TODO:
	// Stright right turn

	// Striaght left turn

	// Diagonal right turn

	// Diagonal left turn

	// In place turns

}

void MOUSE_PrepareToStart(){

	// Nastav pociatocne natocenie podla prvej a druhej pozicie bunky v maze path


	const uint8_t  requireDirection = MAZE_path[0].absoluteDirection;

	// transform mouse 8 directional model, to maze 4 directional model
	const uint8_t actualDirection = 1 << ((MOUSE_CellOrientation-1)>>1);

	const uint8_t difference = actualDirection - requireDirection ;

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

void MOUSE_CorrectionDiagonal(){

}

void MOUSE_CorrectionForward(){


	// use [0] [5] or [1] [4]
	const uint8_t disR = SENSORS_irDistance[0];
	const uint8_t disL = SENSORS_irDistance[5];

	uint8_t error = 0;

	// TODO: map chcek
	if( disR < 60 && disL < 60){
		error = disL - disR;
	}else if(disR < 60){
		error = MOUSE_DISTANCE_FROM_WALL - disR;
	}else if(disL < 60){
		error = disL - MOUSE_DISTANCE_FROM_WALL;
	}

	MOTION_ExternalAngCorrection = 0.1 * (error + 0.5*(error - orientationErrorPrev));
}

void MOUSE_CorrectionRotation(){

}

