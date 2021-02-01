/*
 * mouse.c
 *
 *  Created on: May 12, 2020
 *      Author: xkacejs
 */


#include "mouse.h"


uint8_t orientationErrorPrev;
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


	const uint8_t finalDest = 0x21;

	uint16_t iterrationCnt = 0;
	MOUSE_CellPosition = 0;
	uint8_t actualPosition = 0x00;


	while(1){

		INSTR_CommandListIndex = 0;
		 MAZE_updatePath(actualPosition, finalDest); // 77, 78, 87, 88 are finsh cells
		 MAZE_updatePath(actualPosition, MAZE_path[1].cell->address); // move just one cell
		 CMD_AbsolutePathToRelative( (MAZE_DIRECTIONS*) MAZE_path, CMD_directionList, MOUSE_CellOrientation);
		 // you are searching so u know what's only one cell in front of you
		 //CMD_directionList[1] = CMD_S;
		 CMD_PathToCommand(CMD_directionList, CMD_commandList, (MAZE_DIRECTIONS*) MAZE_path);

		 INSTR_AverageVelocity = avgVel;
//		 MOUSE_PrepareToStart();

		// MOUSE_ChcekForNewComand();
		 MOTION_UpdateList();
		 MOTION_instrID = 0;
		 do{ //cakam kym sa dokonci cely cmd

			 //TODO : crash detection



			 if(MOUSE_isMouseInMiddleOfCell()){
				 CMD_WALLS_RELATIVE detectedWalls ;
				 detectedWalls.wall =  MOUSE_LookForWalls();
				 MOUSE_WriteWalls(detectedWalls);

			 }


			 //caka kym sa vykona potrebny pocet komandov teda kym neprijde posledny stop
			 //Pozor nie 1 instrukcia ale vsetky instrukcie daneho cmd
			MOUSE_ChcekForNewComand();
		 }while(INSTR_InstrListUsedInstr!=MOTION_instrID );

		 actualPosition = MOUSE_CellPosition;
		 //finish // x77
		 if(     MOUSE_CellPosition == finalDest ||\
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
	 CMD_AbsolutePathToRelative((MAZE_DIRECTIONS*)MAZE_path, CMD_directionList, MOUSE_CellOrientation);
	 CMD_PathToCommand(CMD_directionList, CMD_commandList,(MAZE_DIRECTIONS*)MAZE_path);

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

	 return 0;
}

uint8_t MOUSE_ReturnToStart( float avgVel ){
	/*
	 * Moves robot from current cell to begin
	 *
	 */


	MAZE_updatePath(MOUSE_CellPosition, 0x00);
	CMD_AbsolutePathToRelative((MAZE_DIRECTIONS*)MAZE_path, CMD_directionList, MOUSE_CellOrientation);
	CMD_PathToCommand(CMD_directionList, CMD_commandList, (MAZE_DIRECTIONS*)MAZE_path);

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

	return 0;
}

uint8_t MOUSE_UpdateAbsoluOrientation(){

	/*
	 * This function update orientation in cell
	 * and update actuall cell position of mouse in maze
	 * return 0 if data was not updated
	 * return 1 if data was updated
	 */


//.T.ODO HIGHEST PRIORITY
	// .T.ODO add corection values for new instruction
	// corection will be made from mistake of position made in instruction processing

	//if (insList[MOTION_instrID].command != MOUSE_LastCMD){


	/*
	 * To tu sa ma volat prave vtedy ked sa robot presunie z bunky do druhej bunky
	 * a upadtne sa jej pozicia!
	 *
	 * */

	MOUSE_LastCMD = insList[MOTION_instrID].command->cmd;
			MOUSE_CellPosition = insList[MOTION_instrID].command->path->cell->address;
			MOUSE_CellOrientation = (insList[MOTION_instrID].command->path->absoluteDirection == ROT_NULL)?\
									MOUSE_CellOrientation : insList[MOTION_instrID].command->path->absoluteDirection;
		return 1;
	//}

	//return 0;
}


static uint8_t prevContinuance;



uint8_t MOUSE_ChcekForNewComand(){

	 /*
	  * return 0 if cmd was not updated
	  * return 1 if cmd was updated
	 */

	const INSTR_INSTRUCTION* currentInstruction = &insList[MOTION_instrID];






	//static uint8_t prevContinuance = 0;

	uint8_t cmdEnds = 0;
	if (( prevContinuance > 50 && currentInstruction->continuance <50))// instruction was finished
		cmdEnds = 1;

	prevContinuance = currentInstruction->continuance;


	if(cmdEnds)
		MOUSE_UpdateAbsoluOrientation();
	else if(currentInstruction->command->cmd == CMD_STOP)
		return 1;
	else
	     return 0;









	// TODO
	/* Tato funkcia sa bude zaoberat presnim ukonceninm CMD, by CMD skoncil PRESNE na hrane.
	 * Bez nej je to priblizne na hrane, na toto potrebujem merania v bludisku
	 * */


	//const INSTR_INSTRUCTION* currentInstruction = &insList[MOTION_instrID];
	const INSTR_INSTRUCTION* nextInstruction =  MOTION_GetNextInstruction(1);
	//pozrisa aky je comand
	//pozri ci je vzdialenost od end distance 18 cm pred koncom
	//pozri sa do mapy a hladaj roh
	// ak je nasledujuci cmd STOP
	// tak zastav v strede bunky
	// v inom pripade hladaj
// TODO add correction


	if(currentInstruction->command == CMD_STOP){
		// STOP
		// Robot should stop in the middle of the cell
		//TODO: CALC middle of the cell
		return 1;

	}else if(currentInstruction->command >= CMD_FWD0 && currentInstruction->command <= CMD_FWD15){
		// TODO  if next is stop

		// find last corner
		// after you find it travel 90mm + c (c will be measured)
		//
		/* not requred anymore, instructions ware modified to include -90mm if stop
		if(nextInstruction->command == CMD_STOP){
			// stop 90mm before end -> middle of the cell
		}
		*/
		//todo
		//if robot travel > required distance - 180mm, it wil start to searching for border!


		// FORWARD
		/* Robot should return 1 (to start processing new command),
		 * when front pairs of wheel wil be in next cell
		 * and back pair of wheel will be in previous cell
		 * (Robot center will be on the border, of the cells)
		 */


		// IF OK MOMENT MOTION_RequestNewInstruction = 1




	}else if(currentInstruction->command >= CMD_DIA0 && currentInstruction->command <= CMD_DIA31){
		// DIAGONAL
	}

	// jednasa o kratke vzdialenosti takze tam sa velka chyba nenaintegruje
	// zatial moze zostat bez presneho prepinania

	// TODO:
	// Stright right turn

	// Striaght left turn

	// Diagonal right turn

	// Diagonal left turn

	// In place turns

}

uint8_t MOUSE_FindCornerInRotation(){
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

void MOUSE_PrepareToStart(){

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
	// TODO: Correction with front or side wall
}

uint8_t MOUSE_isMouseInMiddleOfCell(){

	return 1;
	//TODO:
	// return 1 if mouse is in middle of the cell

}

uint8_t MOUSE_LookForWalls(){
	// retrun walls that robot can see


	// filter
	// yn = yn(t-T_smpl) + (y(t) - yn(t-T_smpl))/k_filtCoeficient;
	static uint8_t sensorsDist[6] = {80, 80, 80, 80, 80, 80};
	const uint8_t k_filterCoeficient = 3;

	for(uint8_t i = 0 ; i < 6 ; i++){
		sensorsDist[i] = sensorsDist[i] + (SENSORS_irDistance[i]-sensorsDist[i])/k_filterCoeficient;
	}

	// decide if is there wall
	// TODO fuzzy or neural network
	CMD_WALLS_RELATIVE wall;

	wall.wall = 0;


	//left
	if(sensorsDist[2] < MOUSE_WALL_DISTRANCE_TRESHOLD)
		wall.WALL.left = 1;

	// right
	if(sensorsDist[3] < MOUSE_WALL_DISTRANCE_TRESHOLD)
		wall.WALL.right = 1;

	if(sensorsDist[0] < MOUSE_WALL_DISTRANCE_TRESHOLD || //
		sensorsDist[5] < MOUSE_WALL_DISTRANCE_TRESHOLD)
		wall.WALL.front = 1;

	return wall.wall;
}

void MOUSE_WriteWallsToMaze(uint8_t wallid){
	const uint8_t absWall = CMD_RelativeWallToAbsolute((CMD_WALLS_RELATIVE)wallid, MOUSE_CellOrientation);
	const uint8_t newWall = absWall | MAZE_maze[MOUSE_CellPosition].wall;
	MAZE_writeCell(MOUSE_CellPosition,newWall, 0xFF);
}

uint8_t MOUSE_WriteWalls(CMD_WALLS_RELATIVE wall){


	// pozri ci su 2 za sebou rovnake spravi o tom ze tam je wall

	static CMD_WALLS_RELATIVE prevWall;


				// nemam naladu to pisat pekne... sorry
	//right
	if(prevWall.wall & wall.wall & 2)
		MOUSE_WriteWallsToMaze(2);

	//front
	if(prevWall.wall & wall.wall & 4)
		MOUSE_WriteWallsToMaze(4);

	//left
	if(prevWall.wall & wall.wall & 8)
		MOUSE_WriteWallsToMaze(8);

	prevWall.wall = wall.wall;
	//TODO: return if was wall writen in the cell
	return 0;

}

