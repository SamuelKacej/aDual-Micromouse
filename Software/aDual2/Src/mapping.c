/*
 * mapping.c
 *
 *  Created on: Feb 9, 2021
 *      Author: samok
 */

#include "mapping.h"



uint8_t MAPPING_isTimeToReadSideWall(){

	// For now you detect walls only
	if( INSTR_InstrList[MOTION_instrID].command->cmd == CMD_FWD1){
			const uint8_t cont = INSTR_InstrList[MOTION_instrID].continuance ;
			if( cont > 50 && cont < 75 ){
				return 1;

		}
	}
	return 0;

}

uint8_t MAPPING_isTimeToReadFrontWall(){

	// For now you detect walls only
	if( INSTR_InstrList[MOTION_instrID].command->cmd == CMD_FWD1){
			const uint8_t cont = INSTR_InstrList[MOTION_instrID].continuance ;
			if( cont > 75 && cont < 99 ){
				return 1;

		}
	}
	return 0;

}


uint8_t MAPPING_LookForWalls( uint8_t sensorSelect){
	/*
	 * 0b001 - front
	 * 0b010 - diagonal
	 * 0b100 - side
	 * 0b000 - reset to deafult values
	 *
	 */
	// retrun walls that robot can see


	// calculate average from reset to reset
	static uint32_t sensorsDist[6] = {0, 0, 0, 0, 0, 0};
	static uint32_t callCnt[6] = {0, 0, 0, 0, 0, 0};

	//const uint8_t k_filterCoeficient = 7;
	CMD_WALLS_RELATIVE wall;

	wall.wall = 0;

	if(sensorSelect & 0b001){

		sensorsDist[0] += SENSORS_irDistance[0];
		sensorsDist[5] += SENSORS_irDistance[5];
		callCnt[0]++;
		callCnt[5]++;
	}

	if(sensorSelect & 0b010){

		sensorsDist[1] += SENSORS_irDistance[1];
		sensorsDist[4] += SENSORS_irDistance[4];
		callCnt[1]++;
		callCnt[4]++;
	}

	if(SENSORS_irDistance[0] > 20 && SENSORS_irDistance[5] >5){
		//side sensors can measure only if front distance is more then 20mm
		if(sensorSelect & 0b100){

			sensorsDist[2] += SENSORS_irDistance[2];
			sensorsDist[3] += SENSORS_irDistance[3];
			callCnt[2]++;
			callCnt[3]++;
		}
	}
	// reset
	if(sensorSelect == 0){

		uint8_t leftAvg = 0xff;
		uint8_t rightAvg = 0xff;
		uint8_t fwdAvgR = 0xff;
		uint8_t fwdAvgL = 0xff;

		if(callCnt[3] != 0 ){
			//left
			leftAvg = sensorsDist[3]/callCnt[3];
			if( leftAvg < MAPPING_WALL_TRESHOLD_SIDE)
				wall.WALL.left = 1;
		}

		if(callCnt[2] != 0){
			//right
			rightAvg = sensorsDist[2]/callCnt[2];
			if( rightAvg < MAPPING_WALL_TRESHOLD_SIDE)
				wall.WALL.right = 1;
		}

		if(callCnt[0] != 0 && callCnt[5] != 0){
			//forward
			fwdAvgR = sensorsDist[0]/callCnt[0];
			fwdAvgL = sensorsDist[5]/callCnt[5];


			// One of them must have lower value then treshold_1 and average from both must be lower treshold_2
			if(fwdAvgL < MAPPING_WALL_TRESHOLD_FRONT1 && (fwdAvgL+fwdAvgR)/2 < MAPPING_WALL_TRESHOLD_FRONT2 )
				wall.WALL.front = 1;
			if(fwdAvgR < MAPPING_WALL_TRESHOLD_FRONT1 && (fwdAvgL+fwdAvgR)/2 < MAPPING_WALL_TRESHOLD_FRONT2 )
				wall.WALL.front = 1;
		}


		sensorsDist[0] = 0;
		sensorsDist[1] = 0;
		sensorsDist[2] = 0;
		sensorsDist[3] = 0;
		sensorsDist[4] = 0;
		sensorsDist[5] = 0;
		callCnt[0] = 0;
		callCnt[1] = 0;
		callCnt[2] = 0;
		callCnt[3] = 0;
		callCnt[4] = 0;
		callCnt[5] = 0;
	}


	return wall.wall;
}

void MAPPING_WriteWallsToMaze(uint8_t wallid, uint8_t position, MAZE_ABSOLUTE_DIRECTION_T orientation){

	const uint8_t absWall = CMD_RelativeWallToAbsolute((CMD_WALLS_RELATIVE)wallid, orientation);

	const uint8_t newWall = absWall | MAZE_maze[position].wall;
	MAZE_writeCell(position, newWall, 0xFF);
}

uint8_t MAPPING_WriteWalls(CMD_WALLS_RELATIVE wall, uint8_t pos, MAZE_ABSOLUTE_DIRECTION_T absOrientation){
	// returns 0 if walls were writen and 1 if they are not writen

	const uint8_t alreadyWritten = MAZE_maze[pos].written;
	//right
	if( wall.wall & 2){
		MAZE_maze[pos].written = alreadyWritten;
		MAPPING_WriteWallsToMaze(2, pos, absOrientation);
	}

	//front
	if(wall.wall & 4){
		MAZE_maze[pos].written = alreadyWritten;
		MAPPING_WriteWallsToMaze(4, pos, absOrientation);
	}
	//left
	if( wall.wall & 8){
		MAZE_maze[pos].written = alreadyWritten;
		MAPPING_WriteWallsToMaze(8, pos, absOrientation);
	}

	return 	alreadyWritten;

}


void MAPPING_PrintMaze(uint16_t mousePos){


	const uint8_t size = MAZE_SIZE_X;

	//wall Horizontal line
	uint16_t wH[size];

	//wall Vertical line
	uint16_t wV[size];

	// set's array to 0
	for(uint8_t i = 0; i<size; i++){
		wH[i] = 0;
		wV[i] = 0;
	}


	for(uint16_t i = 0 ; i < size ; i++){
		for(uint16_t j = 0 ; j < size ; j++){

			wH[i] |= (uint32_t)((MAZE_maze[MAZE_ADDR(j,i)].wall & 0b0001)>>0)<<j;

			wV[i] |= (uint32_t)((MAZE_maze[MAZE_ADDR(i,j)].wall & 0b1000)>>3)<<j;

		}
	}

	// chcek the wall from other cell
	// OR
	for(uint16_t i = 0 ; i < size-1 ; i++){
		for(uint16_t j = 0 ; j < size-1 ; j++){

			wH[i+1] |= (uint32_t)((MAZE_maze[MAZE_ADDR(j,i)].wall & 0b0100)>>2)<<(j);

			wV[i+1] |= (uint32_t)((MAZE_maze[MAZE_ADDR(i,j)].wall & 0b0010)>>1)<<(j);

		}
	}

	// printing
	char s[39] = "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\r\n";
	HAL_UART_Transmit(&huart3, (uint8_t*)s, 38, 1000);

	for(uint16_t i = 0 ; i < 2*size ; i++){

			for(uint16_t j = 0 ; j < size ; j++){

				if(i%2==0){
					s[2*j +0] = ((wV[j] & (1<<(size-1-i/2)))>0)? '|':' ';
					s[2*j +1] = (mousePos == MAZE_ADDR(j,size-1-i/2))? 'X': ' ';
				}else{
					//├
					s[2*j +1] = ((wH[size-1-i/2] & (1<<j))>0)? '-':' ';
					s[2*j +0] = '+';
				}
			}
			s[32] = '|';
			s[33] = '\n';
			s[34] = '\r';
			s[35] = '\0';

			HAL_UART_Transmit(&huart3, (uint8_t*)s, 36, 1000);
		}


	const char str[] = "#\r\n";
	HAL_UART_Transmit(&huart3, (uint8_t*)str, 3, 1000);


}
