/*
 * mouse.c
 *
 *  Created on: May 12, 2020
 *      Author: xkacejs
 */


#include "mouse.h"


CMD_T MOUSE_LastCMD;

void MOUSE_printSensDist(){

	char s [46];//7*(3+2+1)+3 =45
	const uint8_t len = sprintf(s,"%i,\t %i,\t %i,\t %i,\t %i,\t %i,\t %i\t \r\n",\
			SENSORS_irDistance[0],\
			SENSORS_irDistance[1],\
			SENSORS_irDistance[2],\
			SENSORS_irDistance[3],\
			SENSORS_irDistance[4],\
			SENSORS_irDistance[5],\
			(int)	INSTR_InstrList[MOTION_instrID].command->cmd);

	HAL_UART_Transmit(&huart3, (uint8_t*) s , len, 10);
}
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

	MOUSE_CellPosition = 0;
	MOUSE_CellOrientation = ROT_NORTH;


	CMD_WALLS_RELATIVE detectedWalls ;
	for(uint8_t i = 0; i<35; i++){
		detectedWalls.wall =  MAPPING_LookForWalls(0b111);
	 }
	detectedWalls.wall = MAPPING_LookForWalls(0b000);
	MAPPING_WriteWalls(detectedWalls, MOUSE_CellPosition, MOUSE_CellOrientation);
	//MOUSE_PrintMaze();

	while(1){

		 INSTR_CommandListIndex = 0;
		 //MOUSE_PrintMaze();
		 HAL_Delay(50);

		 MAZE_updatePath(MOUSE_CellPosition, finalDest); // 77, 78, 87, 88 are finsh cells
		 MAZE_updatePath(MOUSE_CellPosition, MAZE_path[1].cell->address); // move just one cell
		 CMD_AbsolutePathToRelative( (MAZE_DIRECTIONS*) MAZE_path, CMD_directionList, MOUSE_CellOrientation);
		 CMD_PathToCommand(CMD_directionList, CMD_commandList, (MAZE_DIRECTIONS*) MAZE_path);

		 INSTR_AverageVelocity = avgVel;

		 MOTION_UpdateList();
		 MOTION_instrID = 0;
		 detectedWalls.wall = 0;

		 do{

			 //TODO : crash detection

			 // writing walls

			 if(MAPPING_isTimeToReadSideWall()){
				 detectedWalls.wall |= 0b1010 & MAPPING_LookForWalls(0b100);
			 }
			 if(MAPPING_isTimeToReadFrontWall()){
				 detectedWalls.wall |= 0b0100 & MAPPING_LookForWalls(0b001);
			 }

			 if(CORR_isPositionForSideCorrection()){
				 ACTUATOR_LED(0, 30, -1);
				 CORR_ParallelToSide();
			 }else{
				 ACTUATOR_LED(0, 0, -1);
			 }

			 HAL_Delay(1);
			 //printf(" %u %u %u\r\n", detectedWalls.WALL.left, detectedWalls.WALL.front, detectedWalls.WALL.right);

			 //caka kym sa vykona potrebny pocet komandov teda kym neprijde posledny stop
			 //Pozor nie 1 instrukcia ale vsetky instrukcie daneho cmd
			MOUSE_ChcekForNewComand();
		 }while(INSTR_InstrListUsedInstr!=MOTION_instrID );


		 detectedWalls.wall = MAPPING_LookForWalls(0b000);
		// mozno bude treba pridat positon
		 MAPPING_WriteWalls(detectedWalls, MOUSE_CellPosition, MOUSE_CellOrientation);

		 // front wall corection at place if there is wall;
		 if(detectedWalls.wall & 0b0100 ){
			 ACTUATOR_LED(150, -1, -1);
			 CORR_PerpendicularToForward();
			 ACTUATOR_LED(0, -1, -1);
		 }

		 //MAPPING_PrintMaze(MOUSE_CellPosition);
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


 	// TODO add corection values for new instruction
	// corection will be made from mistake of position made in instruction processing
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

}

uint8_t MOUSE_ChcekForNewComand(){

	 /*
	  * return 0 if cmd was not updated
	  * return 1 if cmd was updated
	 */

	const INSTR_INSTRUCTION* currentInstruction = &insList[MOTION_instrID];
	//static uint8_t prevContinuance = 0;
	static uint8_t prevContinuance = 0;

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
