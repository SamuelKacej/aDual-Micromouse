/*
 * maze.h
 *
 *  Created on: Feb 17, 2020
 *      Author: xkacejs
 */

#ifndef MAZE_H_
#define MAZE_H_

#include "main.h"
/*
 * ORIENTATION OF MAZE INDEXING
 *
 * [Y]
 * A
 * |_ _ _ _ _ _ _ _ _
 * |   |_| .. _| |  *|
 * | |   |.|_..|.._ _|
 * |_ _| ._  |.|.|   |
 * |  _| ..| | ._  | |
 * |_ _ _|.| |_  |_| |
 * |. .|. .|  _ _ _  |
 * |_|_._|_|_|_ _ _ _|
 * --------------------------> [X]
 *
 *  ___C___
 * |       |
 * D       B
 * |___A___|
 *
 *
 */


/*
 * 1) write cells
 * 2) update path
 * 3) cmd absolut -> realative
 * 4) comd-> instr
 * 5) motionUpdate( takes from instr)
 *
 */

#define MAZE_SIZE_X 16
#define MAZE_SIZE_Y 16
#define MAZE_PATH_SIZE 256

#define MAZE_ADDR(x,y) (MAZE_SIZE_X*(x)+(y))

extern uint8_t MAZE_wasChanged;
// 1 if maze was change
// 0 by default or if user change value;


//TODO: wall/absoluteDirection enum with 0x0000 WNES
typedef struct
{
	uint8_t wall; 	// b0000 DCBA
				 	// wall of the cell

	uint8_t cost; 	// price to cell for maze solving algorithm

	uint8_t address;// absolute address of cell in maze
					// MAZE_SIZE_X * actualX  + actualY // 0xXY
					// |y
					// |__ __ x
	/*    4
	   ___C___
	  |       |
	8 D       B 2
	  |___A___|
	  	  1
	 */



}MAZE_CELL;

typedef struct{
	MAZE_CELL* cell;  			// pointer to maze cell
								//											 |y
	uint8_t absoluteDirection; 	// Direction to next cell in absolute coord |__ __x
								// value can be  0b0000 DCBA
								// 0x00 mean STOP
}MAZE_DIRECTIONS;

typedef	struct{
	// TODO IMPLEMENT this
		uint8_t south : 1;
		uint8_t east : 1;
		uint8_t north : 1;
		uint8_t west : 1;

} MAZE_CELL_WALLS;

volatile MAZE_CELL MAZE_maze[MAZE_SIZE_X * MAZE_SIZE_Y];			// maze
volatile MAZE_DIRECTIONS MAZE_path[MAZE_PATH_SIZE];				// Maze path from position to position
volatile uint8_t MAZE_pathStart;									// Absolute start coord of path
volatile uint8_t MAZE_pathFinish;									// Absolute finish coord of path


void MAZE_writeCell(uint8_t, uint8_t, uint8_t);
void MAZE_updatePath(uint8_t, uint8_t);
void MAZE_floodfill( MAZE_CELL*, MAZE_DIRECTIONS*, uint8_t, uint8_t);
uint8_t MAZE_reverseDirection(uint8_t);
MAZE_CELL* MAZE_findCheapestNeighbor(MAZE_CELL*, uint8_t*);
void MAZE_SetCostOfNeighbors(MAZE_CELL*, uint8_t);
void MAZE_ClearMaze(MAZE_CELL*);

#endif /* MAZE_H_ */
