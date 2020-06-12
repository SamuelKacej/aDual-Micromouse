/*
 * instructions.c
 *
 *  Created on: Feb 20, 2020
 *      Author: xkacejs
 */


#include "instructions.h"



void INSTR_FillHalfList( uint8_t calcHalf ){

	// calcHalf - set which half (first or second) will be filled with instraction
	// always fill just one half of list, because other half is processed by  motor controler

	INSTR_INSTRUCTION* insList = &INSTR_InstrList[(calcHalf)? INSTR_LIST_HALF_SIZE :0];
	// pointer to first or second half of list


	const uint16_t usedInstruction = INSTR_CmdToInstr( CMD_commandList, &INSTR_CommandListIndex, insList );
	INSTR_InstrListUsedInstr[calcHalf] = usedInstruction;

}

float INSTR_CalcAccel(uint16_t vStart, uint16_t vTarget, uint16_t distance){
	/* mm/s/s =( mm/s, mm/s, mm)
	 * Calculate acceleration for speedup from vStart to vTarget, on  given distance
	 */
	return ((int16_t)(vTarget - vStart)) / (2* distance/(vStart+vTarget));

}
float INSTR_CalcDist(uint16_t vStart, uint16_t vTarget, uint16_t accel){
	// return distance traveled until you accelerate from vStart to vTarg
	const float time = (vTarget-vStart)/accel;
	return time*(vTarget+vStart)/2;
}
float INSTR_CalcVel(uint16_t vStart, uint16_t dist, uint16_t accel){
	return sqrt(vStart*vStart + accel*dist*2);
}

void INSTR_AddArc(INSTR_INSTRUCTION* insList, int16_t angleDeg, uint16_t radius, uint16_t transVelocity, CMD_T command ){


	/*
	 * insList	- pointer from which will be instructions add
	 * angleDeg - ± angle in degrees at the end of arc, anti-clock wise orientation
	 * radius 	- radius of circle in mm
	 *
	 * Angle is sinusoidal
	 *
	 */

	radius *= 1.3;// nie je to KRUH, ale OVAL

	const float angleRad = angleDeg*PI/180;
	// TODO: presny vypocet kryvky zakruty, nie je to kruh !!!!!

	// arcLength must be always positive
	const float arcLength =(angleDeg>0)? radius*angleRad : -radius*angleRad;  // mm



	float timeArc; // seconds

	if (radius != 0)
		timeArc = arcLength/transVelocity; 		// Arc
	else{
		timeArc = angleRad * 180/ transVelocity;// in-place turn
												// it will take same time as SS90deg turn
		transVelocity = 0;						// no radius => no arc :)
	}


	insList->command 		= command;
	insList->speed 			= transVelocity;
	insList->dist 			= arcLength;
	insList->accel			= 0;
	insList->angle			= angleRad;
	insList->time			= timeArc;


}



uint16_t INSTR_CmdToInstr( CMD_T* cmdList, uint16_t* idc, INSTR_INSTRUCTION* insList){

	/*
	 * cmdList 	- is pointer to WHOLE command list!
	 * idc 		- is index of element in cmdList from which translational will begin
	 * insList	- is linked list of instruction which will be filled
	 *
	 * return	- corresponds to quantity of written instruction in to list
	 */


	uint16_t id = 0; //id in insList < 0 ; INSTR_LIST_SIZE>

	INSTR_ResetInstrList(insList, INSTR_LIST_HALF_SIZE);

	while ( *idc < CMD_LIST_SIZE){

		if( id > (INSTR_LIST_HALF_SIZE - MAX_INSTRACTION_PER_COMMAND)){
			return id;

		}

		switch(cmdList[*idc]){

			case CMD_STOP:
					// reset set instruction to stop instruction
					id++;
				break;

			//=============== TURNS ================
			// expectted that robot is not moving, just standing at the place
			case CMD_IP45R:
				// first and last instructions are empty
				id++;
				INSTR_AddArc(&insList[id], -45, 0, INSTR_AverageVelocity, cmdList[*idc]);
				id +=2;// 4(arc) +1(empty)

				break;
			//--------------------------------------
			case CMD_IP45L:
				// first and last instructions are empty
				id++;
				INSTR_AddArc(&insList[id], +45, 0, INSTR_AverageVelocity, cmdList[*idc]);
				id +=2;// 1 +1(empty)

				break;
			//--------------------------------------
			case CMD_IP90R:
				// first and last instructions are empty
				id++;
				INSTR_AddArc(&insList[id], -90, 0, INSTR_AverageVelocity, cmdList[*idc]);
				id +=2;// 1(arc) +1(empty)

				break;
			//--------------------------------------
			case CMD_IP90L:
				// first and last instructions are empty
				id++;
				INSTR_AddArc(&insList[id], +90, 0, INSTR_AverageVelocity, cmdList[*idc]);
				id +=2;// 1(arc) +1(empty)

				break;
			//--------------------------------------
			case CMD_IP135R:
				// first and last instructions are empty
				id++;
				INSTR_AddArc(&insList[id], -135, 0, INSTR_AverageVelocity, cmdList[*idc]);
				id +=2;// 1(arc) +1(empty)

				break;
			//--------------------------------------
			case CMD_IP135L:
				// first and last instructions are empty
				id++;
				INSTR_AddArc(&insList[id], +135, 0, INSTR_AverageVelocity, cmdList[*idc]);
				id +=2;// 1(arc) +1(empty)

				break;
			//--------------------------------------
			case CMD_IP180R:
				// first and last instructions are empty
				id++;
				INSTR_AddArc(&insList[id], -180, 0, INSTR_AverageVelocity, cmdList[*idc]);
				id +=2;// 1(arc) +1(empty)

				break;
			//--------------------------------------
			case CMD_IP180L:
				// first and last instructions are empty
				id++;
				INSTR_AddArc(&insList[id], +180, 0, INSTR_AverageVelocity, cmdList[*idc]);
				id +=2;// 1(arc) +1(empty)

				break;


			//=========== TRANSITIONS ==============
			case CMD_SS90SR:
				INSTR_AddArc(&insList[id], -90, 90, INSTR_AverageVelocity, cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_SS90SL:
				INSTR_AddArc(&insList[id], 90, 90, INSTR_AverageVelocity, cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_SS180R:
				INSTR_AddArc(&insList[id], -180, 90, INSTR_AverageVelocity, cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_SS180L:
				INSTR_AddArc(&insList[id], 180, 90, INSTR_AverageVelocity, cmdList[*idc]);
				id++;
				break;
			//--------------------------------------

			case CMD_SD45R:
				INSTR_AddArc(&insList[id], -45, 83, INSTR_AverageVelocity, cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_SD45L:
				INSTR_AddArc(&insList[id], 45, 83, INSTR_AverageVelocity, cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_SD135R:
				INSTR_AddArc(&insList[id], -135, 76, INSTR_AverageVelocity, cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_SD135L:
				INSTR_AddArc(&insList[id], 135, 76, INSTR_AverageVelocity, cmdList[*idc]);
				id++;
				break;

			//--------------------------------------
			case CMD_DS45R:
				// diagonalna zatacka
				INSTR_AddArc(&insList[id], -45, 83, INSTR_AverageVelocity, cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_DS45L:
				INSTR_AddArc(&insList[id], 45, 83, INSTR_AverageVelocity, cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_DS135R:
				//	https://www.desmos.com/calculator/frivebujta
				INSTR_AddArc(&insList[id], -135, 76, INSTR_AverageVelocity, cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_DS135L:
				//	https://www.desmos.com/calculator/frivebujta
				INSTR_AddArc(&insList[id], 135, 76, INSTR_AverageVelocity, cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_DD90R:
				// r = 45mm * sqrt(2)
				INSTR_AddArc(&insList[id], -90, 64, INSTR_AverageVelocity, cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_DD90L:
				// r = 45mm * sqrt(2)
				INSTR_AddArc(&insList[id], 90, 64, INSTR_AverageVelocity, cmdList[*idc]);
				id++;
				break;


			default:
				//============== FORWARD ==============
				if( cmdList[*idc] >=  CMD_FWD0 && cmdList[*idc] <= CMD_FWD15){

					// forward N borders of cell
					uint8_t nCells = cmdList[*idc] - CMD_FWD0;

					if(nCells> 3){
						// peak acceleration


						// speed up
						insList[id].command = CMD_FWD1;
						insList[id].dist   	= CELL_DIMENSION;
						insList[id].speed  	= INSTR_CalcVel(INSTR_AverageVelocity,\
													CELL_DIMENSION, INSTR_MaxTransAccel);
						insList[id].accel  	= INSTR_MaxTransAccel;
						id++;

						// constV
						const uint8_t cellsConstV = nCells-3;
						// speed-up + speed-down + safeCell;

						insList[id].command	= CMD_FWD0 + cellsConstV;
						insList[id].dist   	= CELL_DIMENSION*cellsConstV;
						insList[id].speed  	= insList[id-1].speed;
						insList[id].accel  	= 0;
						id++;

						// slow-down
						/*
						 * FWD2 sa rata cez kolko hran bunky musi prejst
						 * */
						insList[id].command = CMD_FWD2;
						insList[id].dist    = 2*CELL_DIMENSION;
						insList[id].speed   = INSTR_AverageVelocity;
						insList[id].accel   = -INSTR_MaxTransAccel;
						id++;


					}else{
						// const velocity

						insList[id].command = CMD_FWD0 + nCells;
						insList[id].dist    = nCells* CELL_DIMENSION;
						insList[id].speed   = INSTR_AverageVelocity;
						insList[id].accel  	= 0;
						id++;

					}


				//============== DIAGONAL ==============
				}else if(cmdList[*idc] >=  CMD_DIA0 && cmdList[*idc] <= CMD_DIA31){

					// diagonal N centers of cell
					const uint8_t nCells = cmdList[*idc] - CMD_DIA0;


					// forward N borders of cell
					if(nCells> 3){
						//  1/2  peak acceleration


						// speed up
						insList[id].command	= CMD_DIA1;
						insList[id].dist   	= CELL_DIMENSION*SQRT2/2;
						insList[id].speed  	= INSTR_CalcVel(INSTR_AverageVelocity,\
													CELL_DIMENSION*SQRT2/2, INSTR_MaxTransAccel/2);
						insList[id].accel  	= INSTR_MaxTransAccel/2;
						id++;

						// constV
						const uint8_t cellsConstV = nCells-3;
						// speed-up + speed-down + safeCell;

						insList[id].command	= CMD_DIA0 + cellsConstV;
						insList[id].dist   	= CELL_DIMENSION*cellsConstV*SQRT2/2;
						insList[id].speed  	= insList[id-1].speed;
						insList[id].accel  	= 0;
						id++;

						// slow-down on 2 cells
						insList[id].command = CMD_DIA2;
						insList[id].dist   	= 2*CELL_DIMENSION*SQRT2/2;
						insList[id].speed  	= INSTR_AverageVelocity;
						insList[id].accel  	= -INSTR_MaxTransAccel/2;
						id++;


					}else{
						// const velocity

						insList[id].command= CMD_FWD0 + nCells;
						insList[id].dist   = nCells*CELL_DIMENSION*SQRT2/2;
						insList[id].speed  = INSTR_AverageVelocity;
						insList[id].accel  = 0;
						id++;

					}


				}else{
					// ERROR, unknown command
					printf("UNNKOWN COMMAND %i with id %i \r\n", (int)cmdList[*idc], (int) *idc);
				}


		}// end of switch

		*idc += 1;

	}// end of while

	return id;

}

void INSTR_ResetInstrList(INSTR_INSTRUCTION* list, uint16_t length){
	//function set every instruction in list to 0
	for( uint16_t i=0  ; i< length ; i++){

		list[i].command 	= CMD_STOP;
		list[i].dist 		= 0;
		list[i].speed		= 0;
		list[i].accel		= 0;
		list[i].distBegin	= 0;
		list[i].distEnd		= 0;
		list[i].angle		= 0;
		list[i].angleBegin	= 0;
		list[i].angleEnd	= 0;
		list[i].time		= 0;

	}
}
