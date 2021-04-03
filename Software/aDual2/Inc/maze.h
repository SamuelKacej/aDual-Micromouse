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
 * 0xXY
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



typedef	struct{
	// TODO IMPLEMENT this


		/*
		 *   	0x0000 WNES
		 * 			 N
		 * 		   ┌─4─┐
		 * 		W 8│ 0 │2 E
		 * 		   └─1─┘
		 * 		     S
		 */
		uint8_t south : 1;
		uint8_t east : 1;
		uint8_t north : 1;
		uint8_t west : 1;

} MAZE_CELL_WALLS;

typedef enum{
	/*		 12     6
	 * 		   ┌─4─┐
	 * 		  8│ 0 │2
	 * 		   └─1─┘
	 *		  9     3
	 */

	//CMD prefix wa removed

	ROT_NULL = 0,
	ROT_SOUTH = 1,
	ROT_EAST = 2,
	ROT_NORTH = 4,
	ROT_WEST = 8,

	ROT_SE = ROT_SOUTH|ROT_EAST, // 3
	ROT_NE = ROT_NORTH|ROT_EAST, // 6
	ROT_NW = ROT_NORTH|ROT_WEST, // 12
	ROT_SW = ROT_SOUTH|ROT_WEST  // 9


}MAZE_ABSOLUTE_DIRECTION_T;

typedef struct
{


	uint8_t wall; 	// b0000 DCBA   // todo change to  MAZE_CELL_WALLS
				 	// wall of the cell

	uint8_t cost; 	// price to cell for maze solving algorithm

	uint8_t address;// absolute address of cell in maze
					// MAZE_SIZE_X * actualX  + actualY // 0xXY
					// |y
					// |__ __ x
	uint8_t written;
	/*    4
	  ┌───C───┐
	  │       │
	8 D       B 2
	  │		  │
	  └───A───┘
	  	  1
	 */



}MAZE_CELL;

typedef struct{
	MAZE_CELL* cell;  			// pointer to maze cell
								//
	MAZE_ABSOLUTE_DIRECTION_T absoluteDirection;
								//											│y
								// Direction to next cell in absolute coord │	 x
								// value can be  0b0000 DCBA				└─────
								// 0x00 mean STOP
}MAZE_DIRECTIONS;



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
