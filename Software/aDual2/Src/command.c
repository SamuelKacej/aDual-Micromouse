/*
 * command.c
 *
 *  Created on: Feb 19, 2020
 *      Author: xkacejs
 */

#include "command.h"


void CMD_clearList(){

	for(uint16_t i = 0 ; i < CMD_LIST_SIZE ; i++){
		CMD_commandList[i].cmd = CMD_STOP;
		CMD_commandList[i].dist = 0;
		CMD_commandList[i].path = &MAZE_path[0];
	}

}
uint8_t CMD_AbsoluteRotToDirection(MAZE_ABSOLUTE_DIRECTION_T absR){

	// prekala do rot
	// tato funkcia robi nieco uplne ine ako ma napisane je to len TMP na jedno pouziteie
	switch(absR){
	case 0: return 0;
	case 1: return 2;
	case 2: return +1;
	case 4: return 0;
	case 8: return -1;
	}
	return -1;
}
void CMD_AbsolutePathToRelative(MAZE_DIRECTIONS* abosoluteList, CMD_DIRECTIONS_T* relativeList, MAZE_ABSOLUTE_DIRECTION_T initRotation ){
	// translate absolute direction made by floodfill to realative list
	// reference direction will be direction from  previous cell

	int8_t lastRotation = CMD_AbsoluteRotToDirection(initRotation);


	for( uint16_t i = 0 ; i < MAZE_PATH_SIZE ; i++ ){

		const int8_t dir =  CMD_DirectionMazeToCmd( abosoluteList[i].absoluteDirection);
		relativeList[i] = CMD_DirectionRotate(dir, &lastRotation );

		// TODO: add optional break after 3 STOP in row

	}

}

uint8_t CMD_DirectionRotate(int8_t dir,	int8_t* rotation){

	/*	Rotate absolut comand coordiance by given rotation,
	 * return relative direction
	 * return *rotation new rotation.
	 * rotacia ma byt +-1,2 ????? TO JE OTAZKA!
	 * 			_2_
	 * 		 1 | 0 |3  -- toto je asi zle
	 * 			-4-
		///////////////
		 *
	 */



	if(dir == 0x00)
		return CMD_S;

	// +4 to move it in possitive integers
	const uint8_t command = (dir+*rotation + 4)%4;
	*rotation = 2-dir;

	// move command is from <1;4>
	if (command == 0 )
		return 4;
	else
		return command;


}

uint8_t CMD_RelativeWallToAbsolute(CMD_WALLS_RELATIVE dirRel, MAZE_ABSOLUTE_DIRECTION_T rotation){
	/*
	 * transform relative
	 *
	 *
	 *	      4
		   ___C___
		  |       |
		8 D       B 2
		  |___A___|
			  1
	 */
	 /* dirRel =  1,2,4,8

	  * rotation = 8 : 2481
	  * rotation = 4 : 1248
	  * rotation = 2 : 8124
	  * rotation = 1 : 4812
	  * 	  * 	  *

	  * */

	uint8_t out = 0;

	//sorry for this, i know it can be written more in pro style...
	switch (rotation){
		case ROT_SOUTH: // backward
			out |= dirRel.WALL.right << 3;
			out |= dirRel.WALL.front << 0;
			out |= dirRel.WALL.left  << 1;
			break;
		case ROT_EAST: // right
			out |= dirRel.WALL.right << 0;
			out |= dirRel.WALL.front << 1;
			out |= dirRel.WALL.left  << 2;
			break;
		case ROT_NORTH: // forward
			out |= dirRel.WALL.right << 1;
			out |= dirRel.WALL.front << 2;
			out |= dirRel.WALL.left  << 3;
			break;
		case ROT_WEST: // left
			out |= dirRel.WALL.right << 2;
			out |= dirRel.WALL.front << 3;
			out |= dirRel.WALL.left  << 0;
			break;
		default:
			break;
	}
	return out;
}
uint8_t CMD_DirectionMazeToCmd(uint8_t dir){
// translate ABSOLUET maze coordinace to ABSOLUTE command cordinance
	if(dir == 0x00) return CMD_S;//0

	if(dir == 0x01) return CMD_B;//4

	if(dir == 0x02) return CMD_R;//1

	if(dir == 0x04) return CMD_F;//2

	if(dir == 0x08) return CMD_L;//3

	return 0xFF;
}

void CMD_incrementRotation8(int8_t* x, int8_t* y, CMD_COMMAND* cmd){

	//TODO add diagonals, it is not as easy as orthogonals

	switch(cmd->path->absoluteDirection){ // to bola hodnota absolutnej rotacie na konci vykonania prikazu
						 // switch(cmd->rotEnd)
	case ROT_NORTH:
		*x =  *x;
		*y =  *y;
		break;
	case ROT_WEST:
		*x = -*y;
		*y =  *x;
		break;
	case ROT_SOUTH:
		*x = -*x;
		*y = -*y;
		break;
	case ROT_EAST:
		*x =  *y;
		*y = -*x;
		break;
	case ROT_NW:
	case ROT_NE:
	case ROT_SW:
	case ROT_SE:
	default:
		return;
	}
}

int8_t CMD_dirToRot8(MAZE_ABSOLUTE_DIRECTION_T dir){

	/*
	 * Input direction
	 * 			 N
	 * 		 12     6
	 * 		   ┌─4─┐
	 * 	  W   8│ 0 │2   E
	 * 		   └─1─┘
	 *		  9     3
	 *			 S
	 */

	/*
	 * Output rotation
	 * 			 N
	 * 		  1    -1
	 * 		   ┌─0─┐
	 * 	  W   2│ 0 │-2   E
	 * 		   └─4─┘
	 *		  3     -3
	 *			 S
	 */

	switch(dir){
	case ROT_NORTH:	return 0;
	case ROT_NW:	return 1;
	case ROT_WEST:	return 2;
	case ROT_SW:	return 3;
	case ROT_SOUTH:	return 4; // -4
	case ROT_SE:	return -3;
	case ROT_EAST:	return -2;
	case ROT_NE:	return -1;
	case ROT_NULL: 	return 0;
	}



}

MAZE_ABSOLUTE_DIRECTION_T CMD_rotToDir8(int8_t rot){
	/*
		 * Input rotation
		 * 			 N
		 * 		  1    -1
		 * 		   ┌─0─┐
		 * 	  W   2│ 0 │-2   E
		 * 		   └─4─┘
		 *		  3     -3
		 *			 S
		 */

		/*
		 * Output direction
		 * 			 N
		 * 		 12     6
		 * 		   ┌─4─┐
		 * 	  W   8│ 0 │2   E
		 * 		   └─1─┘
		 *		  9     3
		 *			 S
		 */
	switch(rot){
		case  0:	return ROT_NORTH;
		case  1:	return ROT_NW;
		case  2:	return ROT_WEST;
		case  3:	return ROT_SW;
		case  4:	return ROT_SOUTH;
		case -4:	return ROT_SOUTH;
		case -3:	return ROT_SE ;
		case -2:	return ROT_EAST;
		case -1:	return ROT_NE;
		}



}

MAZE_ABSOLUTE_DIRECTION_T CMD_directionRotate8(MAZE_ABSOLUTE_DIRECTION_T orgRot, MAZE_ABSOLUTE_DIRECTION_T rotIncr){

	/*
	 * rotIncrement is rotation add to original rotation
	 * rotIncr == North :=  orgRot + 0  deg
	 * rotIncr == West  :=  orgRot + 90  deg
	 * rotIncr == South :=  orgRot + 180 deg
	 * rotIncr == East  :=  orgRot - 90  deg
	 *
	 * SE=(E, NE)
	 */

	int8_t org = CMD_dirToRot8(orgRot);
	int8_t incr = CMD_dirToRot8(rotIncr);

	//12 becous 8 (whole rotation) + 1 half of rotation
	const int8_t finalRot = (12+org+incr)%8 - 4;
	return CMD_rotToDir8(finalRot);
}
/*
 *
 * --- Tato funkcia bola zakomentovane pretoze zapisovanie pozicie do CMD
 *  sa spravilo pomocou linkovania absPath > CMD ; abs path obsahuje cell a abs dir (NWSE)
 *  ak sa ukaze ze hetno linkovanie staci tato funkcia sa moze zmazat.
 *  zakomentovana je lebo cmd by musel ma tinu strukturu
 *
void CMD_WritePossitionIntoCommand(	CMD_COMMAND* cmdToWrite,\
									CMD_COMMAND* cmdPrev,\
									int8_t posIncrementX,\
									int8_t posIncrementY,\
									MAZE_ABSOLUTE_DIRECTION_T rotIncrement,\
									CMD_T cmd ,\
									uint16_t dist \
									){

	// rotIncrement for same rotatione CMD_ROT_NORTH
	// to retate +90deg write CMD_ROT_WEST
	// *
	// *
	// * X,Y pos increments are intremet to cmdPrev position, where is thought that prev rotation is allway north,
	// * this fcn rotate xy with coresponding rotation given by cmdPrev.RotEnd
	// *
	// *

	cmdToWrite->cmd =  cmd;

	cmdToWrite->dist = dist;

	cmdToWrite->rotEnd = CMD_directionRotate8(cmdPrev->rotEnd, rotIncrement);


	// increment rotation

	CMD_incrementRotation8(&posIncrementX, &posIncrementY, cmdPrev)

	const uint8_t posXstart = (cmdPrev->cellEnd->address & 0xF0 ) >> 4;
	const uint8_t posYstart = cmdPrev->cellEnd->address & 0x0F;

	const uint8_t newAddress = (posXstart+posIncrementX)<<4 + (posYstart+posIncrementY);
	cmdToWrite->cellEnd = &MAZE_maze[newAddress];

}
*/
void CMD_PathToCommand(CMD_DIRECTIONS_T* pathList, CMD_COMMAND* cmdList, MAZE_DIRECTIONS* absPath){

	// input directions must be realative
	// state machine

	uint8_t x = 0;
	uint8_t state = CMD_STATE_START;
	uint8_t idx = 1;
	uint8_t initPosOffset = CELL_DIMENSION/2;//mm

	//cmdList[0].cellEnd = 0x00;
	cmdList[0].dist = 0;


	#define ERROR_STATE (printf("Error while converting path to command at %i index'n", i))


	cmdList[0].cmd = CMD_STOP;
	uint8_t prevIdx = 0;
// TODO clear cmd list

	uint16_t iEnd =  MAZE_PATH_SIZE;// from this wale list wil be resete
	for( uint16_t i = 0 ; i < MAZE_PATH_SIZE ; i++ ){


		if(idx > CMD_LIST_SIZE)
			printf("Error while converting path to command: cmdList overflow array\n");

		switch(state){

			case CMD_STATE_START :

				switch(pathList[i]){
					case CMD_F:
						x++;
						state = CMD_STATE_ORTHO;
						break;
					case CMD_S:
						cmdList[idx].dist = CELL_DIMENSION*x ;
						cmdList[idx++].cmd = CMD_FWD0 +x;
						state = CMD_STATE_STOP;
						break;
					case CMD_R:
						//CMD_WritePossitionIntoCommand(&cmdList, cmdPrev, posIncrementX, posIncrementY, rotIncrement)
						cmdList[idx++].cmd = CMD_IP90R;
						x = 1;
						state = CMD_STATE_START;
						break;
					case CMD_L:
						cmdList[idx++].cmd = CMD_IP90L;
						x = 1;
						state = CMD_STATE_START;
						break;
					case CMD_B:
						cmdList[idx++].cmd = CMD_IP180L;
						x = 1;
						state = CMD_STATE_START;
						break;
					default:
						ERROR_STATE;
						break;
				}
				break;
			//---------------------------------------------------
			case CMD_STATE_ORTHO :

				switch(pathList[i]){
					case CMD_F:
						x++;
						state = CMD_STATE_ORTHO;
						break;
					case CMD_L:
						cmdList[idx].dist = CELL_DIMENSION*x - initPosOffset;
						initPosOffset = 0;
						cmdList[idx++].cmd = CMD_FWD0 +x;
						state = CMD_STATE_ORTHO_L;
						break;
					case CMD_R:
						cmdList[idx].dist = CELL_DIMENSION*x - initPosOffset;
						initPosOffset = 0;
						cmdList[idx++].cmd = CMD_FWD0 +x;
						state = CMD_STATE_ORTHO_R;
						break;
					case CMD_S:
						if(initPosOffset>0){
							 //you are in middle and you will stop in the middle
							cmdList[idx].dist = CELL_DIMENSION*x;
						}else{
							//you are at transition and you will stop in the middle
							cmdList[idx].dist = CELL_DIMENSION*x -CELL_DIMENSION/2;
						}
						initPosOffset = 90;
						cmdList[idx++].cmd = CMD_FWD0+x;
						state = CMD_STATE_STOP;
						break;
					default:
						ERROR_STATE;
						break;
					}
					break;
			//---------------------------------------------------
			case CMD_STATE_ORTHO_R :

				switch(pathList[i]){
					case CMD_F:
						cmdList[idx++].cmd = CMD_SS90SR;
						x=2;
						state = CMD_STATE_ORTHO;
						break;
					case CMD_L:
						cmdList[idx++].cmd = CMD_SD45R;
						x=2;
						state = CMD_STATE_DIAG_RL;
						break;
					case CMD_R:
						state = CMD_STATE_ORTHO_RR;
						break;
					case CMD_S:
						cmdList[idx++].cmd = CMD_SS90SR; //explore?
						cmdList[idx].dist = CELL_DIMENSION*x - CELL_DIMENSION/2;
						cmdList[idx++].cmd = CMD_FWD1;
						state = CMD_STATE_STOP;
						break;
					default:
						ERROR_STATE;
						break;
					}
				break;
				//---------------------------------------------------
			case CMD_STATE_ORTHO_RR :

				switch(pathList[i]){
					case CMD_F:
						cmdList[idx++].cmd = CMD_SS180R;
						x=2;
						state = CMD_STATE_ORTHO;
						break;
					case CMD_L:
						cmdList[idx++].cmd = CMD_SD135R;
						x=2;
						state = CMD_STATE_DIAG_RL;
						break;
					case CMD_R:
						ERROR_STATE;
						break;
					case CMD_S:
						cmdList[idx++].cmd = CMD_SS180R;
						cmdList[idx].dist = CELL_DIMENSION*x - CELL_DIMENSION/2;
						cmdList[idx++].cmd = CMD_FWD1;
						state = CMD_STATE_STOP;
						break;
					default:
						ERROR_STATE;
						break;
					}
				break;
			//---------------------------------------------------
			case CMD_STATE_ORTHO_L :

				switch(pathList[i]){
					case CMD_F:
						cmdList[idx++].cmd = CMD_SS90SL;
						x=2;
						state = CMD_STATE_ORTHO;
						break;
					case CMD_L:
						state = CMD_STATE_ORTHO_LL;
						break;
					case CMD_R:
						cmdList[idx++].cmd = CMD_SD45L;
						x=2;
						state = CMD_STATE_DIAG_LR;
						break;
					case CMD_S:
						cmdList[idx++].cmd = CMD_SS90SR; //explore?
						cmdList[idx].dist = CELL_DIMENSION*x - CELL_DIMENSION/2;
						cmdList[idx++].cmd = CMD_FWD1;
						state = CMD_STATE_STOP;
						break;
					default:
						ERROR_STATE;
						break;
					}
				break;
			//---------------------------------------------------
			case CMD_STATE_ORTHO_LL :

				switch(pathList[i]){
					case CMD_F:
						cmdList[idx++].cmd = CMD_SS180L;
						x=2;
						state = CMD_STATE_ORTHO;
						break;
					case CMD_L:
						ERROR_STATE;
						break;
					case CMD_R:
						cmdList[idx++].cmd = CMD_SD135L;
						x=2;
						state = CMD_STATE_DIAG_LR;
						break;
					case CMD_S:
						cmdList[idx++].cmd = CMD_SS180L;
						cmdList[idx].dist = CELL_DIMENSION*x - CELL_DIMENSION/2;
						cmdList[idx++].cmd = CMD_FWD1;
						state = CMD_STATE_STOP;
						break;
					default:
						ERROR_STATE;
						break;
					}
				break;
			//---------------------------------------------------
			case CMD_STATE_DIAG_LR :

				switch(pathList[i]){
					case CMD_F:
						cmdList[idx++].cmd = CMD_DIA0+x;
						cmdList[idx++].cmd = CMD_DS45R;
						x=2;
						state = CMD_STATE_ORTHO;
						break;
					case CMD_L:
						x++;
						state = CMD_STATE_DIAG_RL;
						break;
					case CMD_R:
						state = CMD_STATE_DIAG_RR;
						break;
					case CMD_S:
						//TODO DIAGONAL STOP CORRECTON
						cmdList[idx++].cmd = CMD_DIA0+x;
						cmdList[idx++].cmd = CMD_DS45R;
						cmdList[idx].dist = CELL_DIMENSION*x - CELL_DIMENSION/2;
						cmdList[idx++].cmd = CMD_FWD1;
						state = CMD_STATE_STOP;
						break;
					default:
						ERROR_STATE;
						break;
					}
				break;
			//---------------------------------------------------
			case CMD_STATE_DIAG_LL :

				switch(pathList[i]){
					case CMD_F:
						cmdList[idx++].cmd = CMD_DIA0+x;
						cmdList[idx++].cmd = CMD_DS135L;
						x=2;
						state = CMD_STATE_ORTHO;
						break;
					case CMD_L:
						ERROR_STATE;
						break;
					case CMD_R:
						cmdList[idx++].cmd = CMD_DIA0+x;
						cmdList[idx++].cmd = CMD_DD90L;
						x=2;
						state = CMD_STATE_DIAG_LR;
						break;
					case CMD_S:
						cmdList[idx++].cmd = CMD_DIA0+x;
						cmdList[idx++].cmd = CMD_DS135L;
						cmdList[idx].dist = CELL_DIMENSION*x - CELL_DIMENSION/2;
						cmdList[idx++].cmd = CMD_FWD1;
						state = CMD_STATE_STOP;
						break;
					default:
						ERROR_STATE;
						break;
					}
				break;


			//---------------------------------------------------
			case CMD_STATE_DIAG_RL :
				switch(pathList[i]){
					case CMD_F:
						cmdList[idx++].cmd = CMD_DIA0+x;
						cmdList[idx++].cmd = CMD_DS45L;
						x=2;
						state = CMD_STATE_ORTHO;
						break;
					case CMD_L:
						state = CMD_STATE_DIAG_LL;
						break;
					case CMD_R:
						x++;
						state = CMD_STATE_DIAG_LR;
						break;
					case CMD_S:
						cmdList[idx++].cmd = CMD_DIA0+x;
						cmdList[idx++].cmd = CMD_DS45L;
						cmdList[idx].dist = CELL_DIMENSION*x - CELL_DIMENSION/2;
						cmdList[idx++].cmd = CMD_FWD1;
						state = CMD_STATE_STOP;
						break;
					default:
						ERROR_STATE;
						break;
					}
				break;
			//---------------------------------------------------
			case CMD_STATE_DIAG_RR :

				switch(pathList[i]){
					case CMD_F:
						cmdList[idx++].cmd = CMD_DIA0+x;
						cmdList[idx++].cmd = CMD_DS135R;
						x=2;
						state = CMD_STATE_ORTHO;
						break;
					case CMD_L:
						cmdList[idx++].cmd = CMD_DIA0+x;
						cmdList[idx++].cmd = CMD_DD90R;
						x=2;
						state = CMD_STATE_DIAG_RL;
						break;
					case CMD_R:
						ERROR_STATE;
						break;
					case CMD_S:
						cmdList[idx++].cmd = CMD_DIA0+x;
						cmdList[idx++].cmd = CMD_DS135R;
						cmdList[idx].dist = CELL_DIMENSION*x - CELL_DIMENSION/2;
						cmdList[idx++].cmd = CMD_FWD1;
						state = CMD_STATE_STOP;
						break;
					default:
						ERROR_STATE;
						break;
					}
				break;
			//---------------------------------------------------
			case CMD_STATE_STOP :
				cmdList[idx].dist = 0;
				cmdList[idx++].cmd = CMD_STOP;
				state = CMD_STATE_STOP;
				// brake for loop
				iEnd = i;
				i = MAZE_PATH_SIZE;

				break;


			default :
				// error
				printf("Error while converting path to command at %i index'n", i);

		}//end of switch


		for(uint16_t j = prevIdx; j < idx ; j++){
			//set abs direction to prev comands and cell address

			if(i==MAZE_PATH_SIZE){ // path is over
				cmdList[j].path = &absPath[iEnd];
			}else{
				cmdList[j].path = &absPath[i];
			}
		}
		prevIdx = idx;



	}//end of for


	// reset remaining items;
	iEnd = (iEnd > 0 )? iEnd-1 : 0;
	for(uint16_t i = idx-1; i < MAZE_PATH_SIZE ; i++){
				//set abs direction to prev comands and cell address

				cmdList[i].path = &absPath[iEnd];
				cmdList[i].dist = 0;
				cmdList[i].cmd = CMD_STOP;
			}


}
