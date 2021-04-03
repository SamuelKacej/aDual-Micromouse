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
uint8_t MOUSE_SearchRun( float avgVel, uint8_t startDest, const uint8_t finalDest ){
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


	MOUSE_CellPosition = startDest;
	INSTR_AverageVelocity = avgVel;



	// detect around walls at the start
	CMD_WALLS_RELATIVE detectedWalls ;
	for(uint8_t i = 0; i<35; i++){
		detectedWalls.wall =  MAPPING_LookForWalls(0b111);
	 }
	detectedWalls.wall = MAPPING_LookForWalls(0b000);
	MAPPING_WriteWalls(detectedWalls, MOUSE_CellPosition, MOUSE_CellOrientation);

	if(startDest == 0x00)
		CORR_PrepareToStart();

	while(1){

		 INSTR_CommandListIndex = 0; // TODO in future list wil be not reseted
		 HAL_Delay(5);

		 // search in map path to finish
		 MAZE_updatePath(MOUSE_CellPosition, finalDest);
		 MAZE_updatePath(MOUSE_CellPosition, MAZE_path[1].cell->address); // move just one cell
		 CMD_AbsolutePathToRelative( (MAZE_DIRECTIONS*) MAZE_path, CMD_directionList, MOUSE_CellOrientation);
		 CMD_PathToCommand(CMD_directionList, CMD_commandList, (MAZE_DIRECTIONS*) MAZE_path);


		 // list with instructions will be updated
		 // from now robot starts moving
		 CORR_CorenrFilterReset();
		 MOTION_UpdateList();
		 MOTION_instrID = 0;

		 detectedWalls.wall = 0;
		 do{


			 // writing walls
			 if(MAPPING_isTimeToReadSideWall()){
				 detectedWalls.wall |= 0b1010 & MAPPING_LookForWalls(0b100);
			 }
			 if(MAPPING_isTimeToReadFrontWall()){
				 detectedWalls.wall |= 0b0100 & MAPPING_LookForWalls(0b001);
			 }

			 // correct position to be parallel to the side walls
			 if(CORR_isPositionForSideCorrection()){

				 CORR_ParallelToSide();
			 }else{

			 }

			 CORR_ForwardCorner();

			 HAL_Delay(2);

			 //caka kym sa vykona potrebny pocet komandov teda kym neprijde posledny stop
			 //Pozor nie 1 instrukcia ale vsetky instrukcie daneho cmd
			MOUSE_ChcekForNewComand();
		 }while(INSTR_InstrListUsedInstr!=MOTION_instrID );
		 ACTUATOR_LED(60, 100, 0);
		 HAL_Delay(10);
		 ACTUATOR_LED(0, 0, 0);
		 // read walls from logger, and reset logger
		 detectedWalls.wall = MAPPING_LookForWalls(0b000);
		 MAPPING_WriteWalls(detectedWalls, MOUSE_CellPosition, MOUSE_CellOrientation);

		 // if there is front wall correct to it
		 if(detectedWalls.wall & 0b0100 ){
			 ACTUATOR_LED(150, -1, -1);
			 CORR_PerpendicularToForward();
			 ACTUATOR_LED(0, -1, -1);
		 }

		 //MAPPING_PrintMaze(MOUSE_CellPosition);

		 if( MOUSE_CellPosition == finalDest ){

			 ACTUATOR_LED(-1, -1, 200);
			 HAL_Delay(1000);
			 ACTUATOR_LED(-1, -1, 0);
			 break;
		 }
	}



	return 0;

}

uint8_t MOUSE_SpeedRun( float avgVel, uint8_t startCell, uint8_t finalCell ){
	/*
	 * Robot must be at the begin of the maze
	 * avgVel is average translation velocity of the robot (~500mm/s)
	 */


	 CORR_PrepareToStart();

	 MAZE_updatePath(startCell, finalCell); // 77, 78, 87, 88 are finsh cells
	 CMD_AbsolutePathToRelative((MAZE_DIRECTIONS*)MAZE_path, CMD_directionList, MOUSE_CellOrientation);
	 INSTR_CommandListIndex = 0;
	 CMD_PathToCommand(CMD_directionList, CMD_commandList,(MAZE_DIRECTIONS*)MAZE_path);

	 INSTR_AverageVelocity = avgVel;

	 // list with instructions will be updated
	 // from now robot starts moving
	 MOTION_UpdateList();
	 MOTION_instrID = 0;


	 while(1){

		 // correct position to be parallel to the side walls
		 if( CORR_isPositionForSideCorrection())
			 CORR_ParallelToSide();

		 if(MOTION_instrID % 2){
			 ACTUATOR_LED(0, 180, 10);
		 }else{
			 ACTUATOR_LED(100, 0, 200);
		 }


		 CORR_ForwardCorner();

		 HAL_Delay(2);
		 MOUSE_ChcekForNewComand();

		 //finish
		 if(     MOUSE_CellPosition == finalCell){


			 ACTUATOR_LED(-1, -1, 200);
			 HAL_Delay(1000);
			 ACTUATOR_LED(-1, -1, 0);
			 CORR_PerpendicularToForward();
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

		INSTR_AverageVelocity = avgVel;
		uint8_t finalDest = 0;


		// detect around walls at the start
		CMD_WALLS_RELATIVE detectedWalls ;
		for(uint8_t i = 0; i<35; i++){
			detectedWalls.wall =  MAPPING_LookForWalls(0b111);
		 }
		detectedWalls.wall = MAPPING_LookForWalls(0b000);
		MAPPING_WriteWalls(detectedWalls, MOUSE_CellPosition, MOUSE_CellOrientation);


		while(1){

			 INSTR_CommandListIndex = 0; // TODO in future list wil be not reseted
			 HAL_Delay(5);

			 // search in map path to finish
			 MAZE_updatePath(MOUSE_CellPosition, finalDest);
			 MAZE_updatePath(MOUSE_CellPosition, MAZE_path[1].cell->address); // move just one cell
			 CMD_AbsolutePathToRelative( (MAZE_DIRECTIONS*) MAZE_path, CMD_directionList, MOUSE_CellOrientation);
			 CMD_PathToCommand(CMD_directionList, CMD_commandList, (MAZE_DIRECTIONS*) MAZE_path);


			 // list with instructions will be updated
			 // from now robot starts moving
			 CORR_CorenrFilterReset();
			 MOTION_UpdateList();
			 MOTION_instrID = 0;

			 detectedWalls.wall = 0;
			 do{


				 // writing walls
				 if(MAPPING_isTimeToReadSideWall()){
					 detectedWalls.wall |= 0b1010 & MAPPING_LookForWalls(0b100);
				 }
				 if(MAPPING_isTimeToReadFrontWall()){
					 detectedWalls.wall |= 0b0100 & MAPPING_LookForWalls(0b001);
				 }

				 // correct position to be parallel to the side walls
				 if(CORR_isPositionForSideCorrection()){

					 CORR_ParallelToSide();
				 }else{

				 }

				 CORR_ForwardCorner();

				 HAL_Delay(2);

				 //caka kym sa vykona potrebny pocet komandov teda kym neprijde posledny stop
				 //Pozor nie 1 instrukcia ale vsetky instrukcie daneho cmd
				MOUSE_ChcekForNewComand();
			 }while(INSTR_InstrListUsedInstr!=MOTION_instrID );
			 ACTUATOR_LED(60, 100, 0);
			 HAL_Delay(10);
			 ACTUATOR_LED(0, 0, 0);
			 // read walls from logger, and reset logger
			 detectedWalls.wall = MAPPING_LookForWalls(0b000);
			 MAPPING_WriteWalls(detectedWalls, MOUSE_CellPosition, MOUSE_CellOrientation);

			 // if there is front wall correct to it
			 if(detectedWalls.wall & 0b0100 ){
				 ACTUATOR_LED(150, -1, -1);
				 CORR_PerpendicularToForward();
				 ACTUATOR_LED(0, -1, -1);
			 }

			 MAPPING_PrintMaze(MOUSE_CellPosition);

			 if( MOUSE_CellPosition == finalDest ){

				 ACTUATOR_LED(-1, -1, 200);
				 HAL_Delay(1000);
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



	if(cmdEnds){
		CORR_CorenrFilterReset();
		return MOUSE_UpdateAbsoluOrientation();

	}else if(currentInstruction->command->cmd == CMD_STOP)
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
/*
void MOUSE_Test(){

	MAZE_writeCell(MAZE_ADDR(0,0), 0b1011, 0xFF);
	MAZE_writeCell(MAZE_ADDR(0,1), 0b1010, 0xFF);
	MAZE_writeCell(MAZE_ADDR(0,3), 0b1100, 0xFF);

	MAZE_writeCell(MAZE_ADDR(1,0), 0b1001, 0xFF);
	MAZE_writeCell(MAZE_ADDR(1,1), 0b1100, 0xFF);
	MAZE_writeCell(MAZE_ADDR(1,2), 0b0101, 0xFF);

	MAZE_writeCell(MAZE_ADDR(2,0), 0b0101, 0xFF);
	MAZE_writeCell(MAZE_ADDR(2,1), 0b0101, 0xFF);
	MAZE_writeCell(MAZE_ADDR(2,2), 0b0101, 0xFF);

	MAZE_writeCell(MAZE_ADDR(3,0), 0b0101, 0xFF);
	MAZE_writeCell(MAZE_ADDR(3,1), 0b0011, 0xFF);
	MAZE_writeCell(MAZE_ADDR(3,2), 0b0110, 0xFF);

	MAZE_writeCell(MAZE_ADDR(4,0), 0b0011, 0xFF);
	MAZE_writeCell(MAZE_ADDR(4,1), 0b1100, 0xFF);
	MAZE_writeCell(MAZE_ADDR(4,2), 0b1111, 0xFF);

	MAZE_writeCell(MAZE_ADDR(5,0), 0b1101, 0xFF);
	MAZE_writeCell(MAZE_ADDR(5,1), 0b0011, 0xFF);
	MAZE_writeCell(MAZE_ADDR(5,2), 0b1100, 0xFF);

	MAZE_writeCell(MAZE_ADDR(6,0), 0b0101, 0xFF);
	MAZE_writeCell(MAZE_ADDR(6,1), 0b1001, 0xFF);
	MAZE_writeCell(MAZE_ADDR(6,2), 0b0110, 0xFF);

	MAZE_writeCell(MAZE_ADDR(7,0), 0b0011, 0xFF);
	MAZE_writeCell(MAZE_ADDR(7,1), 0b0110, 0xFF);

	 *  0 1 2 3 4 5 6 7
	 *  _ _ _ _ _ _ _
	 * |  _ _  |_|   |_
	 * | |  _ _|  _|_  |
	 * |_|_ _ _ _|_ _ _|
	 *

	HAL_Delay(1e3);

	MOUSE_SpeedRun(1200, 0x00, 0x50);
	HAL_Delay(1e3);

};*/


void MOUSE_Test(){

	MAZE_writeCell(MAZE_ADDR(0,0), 0b1011, 0xFF);
	MAZE_writeCell(MAZE_ADDR(0,1), 0b1100, 0xFF);
	MAZE_writeCell(MAZE_ADDR(0,2), 0b1111, 0xFF);

	MAZE_writeCell(MAZE_ADDR(1,0), 0b1001, 0xFF);
	MAZE_writeCell(MAZE_ADDR(1,1), 0b0110, 0xFF);
	MAZE_writeCell(MAZE_ADDR(1,2), 0b1111, 0xFF);

	MAZE_writeCell(MAZE_ADDR(2,0), 0b0101, 0xFF);
	MAZE_writeCell(MAZE_ADDR(2,1), 0b1001, 0xFF);
	MAZE_writeCell(MAZE_ADDR(2,2), 0b1100, 0xFF);

	MAZE_writeCell(MAZE_ADDR(3,0), 0b0101, 0xFF);
	MAZE_writeCell(MAZE_ADDR(3,1), 0b0101, 0xFF);
	MAZE_writeCell(MAZE_ADDR(3,2), 0b0101, 0xFF);

	MAZE_writeCell(MAZE_ADDR(4,0), 0b0011, 0xFF);
	MAZE_writeCell(MAZE_ADDR(4,1), 0b0110, 0xFF);
	MAZE_writeCell(MAZE_ADDR(4,2), 0b0101, 0xFF);

	MAZE_writeCell(MAZE_ADDR(5,0), 0b1001, 0xFF);
	MAZE_writeCell(MAZE_ADDR(5,1), 0b1010, 0xFF);
	MAZE_writeCell(MAZE_ADDR(5,2), 0b0110, 0xFF);

	MAZE_writeCell(MAZE_ADDR(6,0), 0b0001, 0xFF);
	MAZE_writeCell(MAZE_ADDR(6,1), 0b0110, 0xFF);
	MAZE_writeCell(MAZE_ADDR(6,2), 0b1101, 0xFF);

	MAZE_writeCell(MAZE_ADDR(7,0), 0b0101, 0xFF);
	MAZE_writeCell(MAZE_ADDR(7,1), 0b1001, 0xFF);
	MAZE_writeCell(MAZE_ADDR(7,2), 0b0110, 0xFF);

	MAZE_writeCell(MAZE_ADDR(8,0), 0b0011, 0xFF);
	MAZE_writeCell(MAZE_ADDR(8,1), 0b0110, 0xFF);
	MAZE_writeCell(MAZE_ADDR(8,2), 0b1111, 0xFF);
	/*  0 1 2 3 4 5 6 7 8
	 *  _ _ _ _ _ _ _ _ _
	 * |_ _|  _ _  |_  |_|
	 * |   |_ _  | | |_  |
	 * |_|_ _ _ _|_ _ _ _|
	 */

	MAPPING_PrintMaze(0);
	HAL_Delay(1e3);

	MOUSE_SpeedRun(1000, 0x00, 0x62);
	HAL_Delay(1e3);

};


CMD_WALLS_RELATIVE MOUSE_GetRelativeWalls(){
	// return relative walls from orientation of the mouse, of the end cell of current command
	const uint8_t absWall = MOUSE_CURRENT_INSTR.command->path->cell->wall;
	return CMD_AbsoluteWallToRelative(absWall, MOUSE_CellOrientation);

}
