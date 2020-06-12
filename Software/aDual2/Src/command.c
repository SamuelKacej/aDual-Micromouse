/*
 * command.c
 *
 *  Created on: Feb 19, 2020
 *      Author: xkacejs
 */

#include "command.h"


void CMD_clearList(){

	for(uint16_t i = 0 ; i < CMD_LIST_SIZE ; i++)
		CMD_commandList[i] = CMD_STOP;

}

void CMD_AbsolutePathToRelative(MAZE_DIRECTIONS* abosoluteList, CMD_DIRECTIONS_T* relativeList ){
	// translate absolute direction made by floodfill to realative list
	// reference direction will be direction from  previous cell

	int8_t lastRotation = 0;


	for( uint16_t i = 0 ; i < MAZE_PATH_SIZE ; i++ ){

		const int8_t dir =  CMD_DirectionMazeToCmd( abosoluteList[i].absoluteDirection);
		relativeList[i] = CMD_DirectionRotate(dir, &lastRotation );

	}

}

uint8_t CMD_DirectionRotate(int8_t dir,	int8_t* rotation){

	/*	Rotate absolut comand coordiance by given rotation,
	 * return relative direction
	 * return *rotation new rotation.
	 * 			_2_
	 * 		 1 | 0 |3
	 * 			-4-
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
uint8_t CMD_DirectionMazeToCmd(uint8_t dir){
// translate ABSOLUET maze coordinace to ABSOLUTE command cordinance
	if(dir == 0x00) return CMD_S;//0

	if(dir == 0x01) return CMD_B;//4

	if(dir == 0x02) return CMD_R;//1

	if(dir == 0x04) return CMD_F;//2

	if(dir == 0x08) return CMD_L;//3

	return 0xFF;
}

void CMD_PathToCommand(CMD_DIRECTIONS_T* pathList, CMD_T* cmdList){

	// input directions must be realtive
	// state machine

	uint8_t x = 0;
	uint8_t state = CMD_STATE_START;
	uint8_t idx = 0;

#define ERROR_STATE // vypis idx i a path[i]

	for( uint16_t i = 0 ; i < MAZE_PATH_SIZE ; i++ ){

		if(idx > CMD_LIST_SIZE)
			printf("Error while converting path to command: cmdList overflow array\n");

		switch(state){

			case CMD_STATE_START :

				switch(pathList[i]){
					case CMD_F:
						x = 1;
						state = CMD_STATE_ORTHO;
						break;
					case CMD_S:
						state = CMD_STATE_STOP;
						break;
					default:
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
						cmdList[idx++] = CMD_FWD0 +x;
						state = CMD_STATE_ORTHO_L;
						break;
					case CMD_R:
						cmdList[idx++] = CMD_FWD0 +x;
						state = CMD_STATE_ORTHO_R;
						break;
					case CMD_S:
						cmdList[idx++] = CMD_FWD0+x;
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
						cmdList[idx++] = CMD_SS90SR;
						x=2;
						state = CMD_STATE_ORTHO;
						break;
					case CMD_L:
						cmdList[idx++] = CMD_SD45R;
						x=2;
						state = CMD_STATE_DIAG_RL;
						break;
					case CMD_R:
						state = CMD_STATE_ORTHO_RR;
						break;
					case CMD_S:
						cmdList[idx++] = CMD_SS90SR; //explore?
						cmdList[idx++] = CMD_FWD1;
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
						cmdList[idx++] = CMD_SS180R;
						x=2;
						state = CMD_STATE_ORTHO;
						break;
					case CMD_L:
						cmdList[idx++] = CMD_SD135R;
						x=2;
						state = CMD_STATE_DIAG_RL;
						break;
					case CMD_R:
						ERROR_STATE;
						break;
					case CMD_S:
						cmdList[idx++] = CMD_SS180R;
						cmdList[idx++] = CMD_FWD1;
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
						cmdList[idx++] = CMD_SS90SL;
						x=2;
						state = CMD_STATE_ORTHO;
						break;
					case CMD_L:
						state = CMD_STATE_ORTHO_LL;
						break;
					case CMD_R:
						cmdList[idx++] = CMD_SD45L;
						x=2;
						state = CMD_STATE_DIAG_LR;
						break;
					case CMD_S:
						cmdList[idx++] = CMD_SS90SR; //explore?
						cmdList[idx++] = CMD_FWD1;
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
						cmdList[idx++] = CMD_SS180L;
						x=2;
						state = CMD_STATE_ORTHO;
						break;
					case CMD_L:
						ERROR_STATE;
						break;
					case CMD_R:
						cmdList[idx++] = CMD_SD135L;
						x=2;
						state = CMD_STATE_DIAG_LR;
						break;
					case CMD_S:
						cmdList[idx++] = CMD_SS180L;
						cmdList[idx++] = CMD_FWD1;
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
						cmdList[idx++] = CMD_DIA0+x;
						cmdList[idx++] = CMD_DS45R;
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
						cmdList[idx++] = CMD_DIA0+x;
						cmdList[idx++] = CMD_DS45R;
						cmdList[idx++] = CMD_FWD1;
						state = CMD_STATE_STOP;
						break;
					default:
						ERROR_STATE
						break;
					}
				break;
			//---------------------------------------------------
			case CMD_STATE_DIAG_LL :

				switch(pathList[i]){
					case CMD_F:
						cmdList[idx++] = CMD_DIA0+x;
						cmdList[idx++] = CMD_DS135L;
						x=2;
						state = CMD_STATE_ORTHO;
						break;
					case CMD_L:
						ERROR_STATE;
						break;
					case CMD_R:
						cmdList[idx++] = CMD_DIA0+x;
						cmdList[idx++] = CMD_DD90L;
						x=2;
						state = CMD_STATE_DIAG_LR;
						break;
					case CMD_S:
						cmdList[idx++] = CMD_DIA0+x;
						cmdList[idx++] = CMD_DS135L;
						cmdList[idx++] = CMD_FWD1;
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
						cmdList[idx++] = CMD_DIA0+x;
						cmdList[idx++] = CMD_DS45L;
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
						cmdList[idx++] = CMD_DIA0+x;
						cmdList[idx++] = CMD_DS45L;
						cmdList[idx++] = CMD_FWD1;
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
						cmdList[idx++] = CMD_DIA0+x;
						cmdList[idx++] = CMD_DS135R;
						x=2;
						state = CMD_STATE_ORTHO;
						break;
					case CMD_L:
						cmdList[idx++] = CMD_DIA0+x;
						cmdList[idx++] = CMD_DD90R;
						x=2;
						state = CMD_STATE_DIAG_RL;
						break;
					case CMD_R:
						ERROR_STATE;
						break;
					case CMD_S:
						cmdList[idx++] = CMD_DIA0+x;
						cmdList[idx++] = CMD_DS135R;
						cmdList[idx++] = CMD_FWD1;
						state = CMD_STATE_STOP;
						break;
					default:
						ERROR_STATE;
						break;
					}
				break;
			//---------------------------------------------------
			case CMD_STATE_STOP :
				cmdList[idx++] = CMD_STOP;
				state = CMD_STATE_STOP;
				break;

			default :
				// error
				printf("Error while converting path to command at %i index'n", i);

		}//end of switch


	}//end of for

	cmdList[idx++] = CMD_STOP;



}