/*
 * maze.c
 *
 *  Created on: Feb 17, 2020
 *      Author: xkacejs
 */

#include "maze.h"

uint8_t MAZE_wasChanged  = 0;


void MAZE_writeCell(uint8_t address, uint8_t walls, uint8_t cost){

	if(MAZE_maze[address].wall != walls){

		MAZE_maze[address].wall = walls;

		const uint8_t addrX = address / MAZE_SIZE_X;
		const uint8_t addrY = address % MAZE_SIZE_X;


		//write wall in to cell on other side of the wall

		//south cell
		if(addrY >1 && walls & 0b0001){
			MAZE_maze[MAZE_ADDR(addrX, addrY-1)].wall |= 0b0100;
		}
		//east cell
		if(addrX < (MAZE_SIZE_X-1) && walls & 0b0010){
			MAZE_maze[MAZE_ADDR(addrX+1, addrY)].wall |= 0b1000;
		}
		//north cell
		if(addrY < (MAZE_SIZE_Y-1) && walls & 0b0100){
			MAZE_maze[MAZE_ADDR(addrX, addrY+1)].wall |= 0b0001;
		}
		//west cell
		if(addrX >1 && walls & 0b1000){
			MAZE_maze[MAZE_ADDR(addrX-1, addrY)].wall |= 0b0010;
		}

		MAZE_wasChanged = 1;

	}
	if( cost != 0xFF){
		MAZE_maze[address].cost = cost;
	}
}

void MAZE_updatePath(uint8_t fromCell, uint8_t toCell){

	MAZE_floodfill(MAZE_maze, MAZE_path, fromCell, toCell);
}

void MAZE_floodfill( MAZE_CELL* maze, MAZE_DIRECTIONS* path, uint8_t fromCell, uint8_t toCell){

	// fromCell and toCell must be in form 0xXY

	uint16_t cellsInMaze = MAZE_SIZE_X*MAZE_SIZE_Y;

	// reset cost of the maze cells
	for(uint16_t i = 0; i < cellsInMaze; i++)
		maze[i].cost = 0xFF;


	// this floodfill DON'T apply cost of diagonal, straight..
	// every cell has equal cost

	uint8_t prevCost = 0;
	maze[fromCell].cost = 0;


	//evaluate cells
	while(1)
	{


		// ohodnoti susedne bunky poslednych hodnotenych
		for(uint16_t i = 0; i < cellsInMaze ; i++){

			//was this cell edited in last iteration?
			if(maze[i].cost == prevCost)
				MAZE_SetCostOfNeighbors(maze, i);
		}

		prevCost++;

		if(maze[toCell].cost < 0xFF)
			break;
	}

	//back propagation
	// begin at finish and find cheapest neighbor... until u come to start
	MAZE_CELL* pMazeCell = &maze[toCell];
	uint8_t direction = 0;

	for(int16_t i = 255; i>=0 ;i--){

		MAZE_path[i].cell = pMazeCell;
		MAZE_path[i].absoluteDirection = MAZE_reverseDirection(direction);

		// if u  are at the start
		if(pMazeCell->address== fromCell){


			// This removes empty directions at begin of array
			// ... maze_path was filled from the end 255->0
			if(i>0)
			for(uint16_t j = 0; j < 256  ; j++){

				if(i+j < 256)
					MAZE_path[j] = MAZE_path[i+j];
				else
					MAZE_path[j] = MAZE_path[255];// target finish cell

			}
			break;
		}

		pMazeCell = MAZE_findCheapestNeighbor(pMazeCell, &direction);



	}
}

uint8_t MAZE_reverseDirection(uint8_t dir){
	// switch two pairs of bits
	/*
	 * 0bXXXXDCAB -> 0b0000BADC
	 * up to down
	 * down to up
	 * left to right
	 * right to left
	 * */
	dir &= 0xF;
	return (dir<<2 | dir>>2) & 0xF;
}

MAZE_CELL* MAZE_findCheapestNeighbor(MAZE_CELL* mazeCell, uint8_t* direction){
	//mazeCell - pointer to maze cell
	//direction - address at which will be store direction to cheapest neighbor

	uint8_t addrX = (mazeCell->address & 0xF0)>>4;
	uint8_t addrY = mazeCell->address &0x0F;
	MAZE_CELL* tmp;
	MAZE_CELL* resoult = mazeCell; // return value


	// is the wall the boarder wall?
	// get neighbor cell
	// is NOT wall between origin cell and neighbor cell?
	// and is neighbor cell cheaper then cheapest cell?
	// then return pointer to cheapest cell and store direction in &direction

	// COMPILER SHOULD AUTOMATICLE move pointer by size of struct!!

	if (addrY-1 >= 0){
		tmp = mazeCell - 1; //- sizeof(MAZE_CELL); // [addrX][addrY-1];
		if( (mazeCell->wall & 0b0001)==0 && tmp->cost < resoult->cost ){
			resoult = tmp ;
			*direction = 0b1;
		}
	}

	if (addrX+1 < MAZE_SIZE_X){
		tmp = mazeCell + MAZE_SIZE_Y;//*sizeof(MAZE_CELL); // [addrX+1][addrY];
		if( (mazeCell->wall & 0b0010)==0 && tmp->cost < resoult->cost ){
			resoult = tmp;
			*direction = 0b10;
		}
	}

	if (addrY+1 < MAZE_SIZE_Y){
		tmp = mazeCell + 1;// sizeof(MAZE_CELL); // [addrX][addrY+1];
		if( (mazeCell->wall & 0b0100)==0 && tmp->cost < resoult->cost ){
			resoult = tmp;
			*direction = 0b100;
		}
	}

	if (addrX-1 >=0){
		tmp = mazeCell - MAZE_SIZE_Y;//*sizeof(MAZE_CELL); // [addrX-1][addrY];
		if( (mazeCell->wall & 0b1000)==0 && tmp->cost < resoult->cost ){
			resoult = tmp;
			*direction = 0b1000;
		}
	}

	return resoult;

}

MAZE_CELL* MAZE_getNeighbor(MAZE_CELL* mazeCell, MAZE_ABSOLUTE_DIRECTION_T direction){
	//@mazeCell - pointer to maze cell
	//@direction - direction to the retuned cell from @mazeCell
	// fcn returns pointer to  set  cell, if there is no cell return null
	const int8_t X = (mazeCell->address & 0xF0)>>4;
	const int8_t Y =  mazeCell->address & 0x0F;


	switch(direction){
	case ROT_NULL:  return mazeCell;

	case ROT_SOUTH: return (Y-1 >= 0)? mazeCell-1: NULL;
	case ROT_EAST:  return (X+1 < MAZE_SIZE_X)? mazeCell + MAZE_SIZE_Y: NULL;
	case ROT_NORTH: return (Y+1 < MAZE_SIZE_Y)? mazeCell+1: NULL;
	case ROT_WEST:  return (X-1 >= 0)? mazeCell-1: NULL;

	case ROT_SE: return (Y-1 >= 0 && X+1 < MAZE_SIZE_X)? mazeCell-1+ MAZE_SIZE_Y: NULL;
	case ROT_NE: return (Y+1 < MAZE_SIZE_Y && X+1 < MAZE_SIZE_X )?mazeCell + MAZE_SIZE_Y +1: NULL;
	case ROT_NW: return (Y-1 >= 0)? mazeCell-1: NULL;
	case ROT_SW: return (Y-1 >= 0)? mazeCell-1: NULL;
	default: return NULL;

	}
}

void MAZE_SetCostOfNeighbors(MAZE_CELL* maze, uint8_t addr){
	//borders are expected in maze !!!
	//maze - pointer to maze
	// addr - address of cell around which we will set cost

	uint8_t addrX = (0xF0 & addr)>>4;
	uint8_t addrY = 0x0F & addr;
	uint8_t setCost = maze[addr].cost+1;

	// border overflow is not chcek, because borders are expected!
	// is transition free? has the neighbor cell lower cost as setCost ?

	if( (maze[addr].wall & 0b0001)==0 && maze[MAZE_ADDR(addrX,addrY-1)].cost > setCost)
		maze[MAZE_ADDR(addrX,addrY-1)].cost = setCost;

	if( (maze[addr].wall & 0b0010)==0 && maze[MAZE_ADDR(addrX+1,addrY)].cost > setCost)
		maze[MAZE_ADDR(addrX+1,addrY)].cost = setCost;

	if( (maze[addr].wall & 0b0100)==0 && maze[MAZE_ADDR(addrX,addrY+1)].cost > setCost)
		maze[MAZE_ADDR(addrX,addrY+1)].cost = setCost;

	if( (maze[addr].wall & 0b1000)==0 && maze[MAZE_ADDR(addrX-1,addrY)].cost > setCost)
		maze[MAZE_ADDR(addrX-1,addrY)].cost = setCost;

}

void MAZE_ClearMaze(MAZE_CELL* maze){

	for(uint16_t i = 0 ; i < MAZE_PATH_SIZE ; i++){
		MAZE_path[i].cell = &maze[0];
	}

	for(uint16_t i = 0 ; i < MAZE_SIZE_X ; i++){
		for(uint16_t j = 0 ; j<MAZE_SIZE_Y ; j++){

			maze[MAZE_ADDR(i,j)].cost = 0;
			maze[MAZE_ADDR(i,j)].wall = 0;
			maze[MAZE_ADDR(i,j)].address = MAZE_ADDR(i,j);

			// borders
			if( i == 0)
				maze[MAZE_ADDR(i,j)].wall |= 0b1000;

			if( i == MAZE_SIZE_X-1)
				maze[MAZE_ADDR(i,j)].wall |= 0b0010;

			if( j == 0)
				maze[MAZE_ADDR(i,j)].wall |= 0b0001;

			if( j == MAZE_SIZE_Y-1)
				maze[MAZE_ADDR(i,j)].wall |= 0b0100;
		}
	}
}


