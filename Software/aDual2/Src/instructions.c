/*
 * instructions.c
 *
 *  Created on: Feb 20, 2020
 *      Author: xkacejs
 */


#include "instructions.h"


/*
 * Previously ussed instruction processing, back in time there ware two arrays,
 *  where one was processed and another was filled with data then they switch...
 *  now it uses just one big array
void INSTR_FillHalfList( uint8_t calcHalf ){

	// calcHalf - set which half (first or second) will be filled with instraction
	// always fill just one half of list, because other half is processed by  motor controler

	INSTR_INSTRUCTION* insList = &INSTR_InstrList[(calcHalf)? INSTR_LIST_HALF_SIZE :0];
	// pointer to first or second half of list


	const uint16_t usedInstruction = INSTR_CmdToInstr( CMD_commandList, &INSTR_CommandListIndex, insList );
	INSTR_InstrListUsedInstr[calcHalf] = usedInstruction;

}
*/
void INSTR_FillList(INSTR_INSTRUCTION* insStartElement){
	//Function takes CMD_commandList, and translate it into Instructions from

	const uint16_t usedInstruction = INSTR_CmdToInstr( CMD_commandList,&INSTR_CommandListIndex, insStartElement );
	INSTR_InstrListUsedInstr = usedInstruction;

}

float INSTR_CalcAccel(uint16_t vStart, uint16_t vTarget, uint16_t distance){
	/* mm/s/s =( mm/s, mm/s, mm)
	 * Calculate acceleration for speedup from vStart to vTarget, on  given distance
	 */
	return ((int16_t)(vTarget - vStart)) / (2* distance/(vStart+vTarget));

}
float INSTR_CalcDist(uint16_t vStart, uint16_t vTarget, int16_t accel){
	//mm
	// return distance traveled until you accelerate from vStart to vTarg
	const float time = ((float)vTarget-vStart)/accel;
	return time*(vTarget+vStart)/2;
}
float INSTR_CalcVel(uint16_t vStart, uint16_t dist, uint16_t accel){
	return sqrt(vStart*vStart + accel*dist*2);
}

void INSTR_AddArc(INSTR_INSTRUCTION* insList, int16_t angleDeg, uint16_t radius, uint16_t transVelocity, CMD_COMMAND* command ){


	/*
	 * insList	- pointer from which will be instructions add
	 * angleDeg - ± angle in degrees at the end of arc, anti-clock wise orientation
	 * radius 	- radius of circle in mm
	 *
	 * Angle is sinusoidal
	 *
	 */

	// nie je to KRUH, ale OVAL 1.3
	//radius *= (radius == 0)? 1 : 1.1;


	float angleRad = angleDeg*PI/180;
	const float absAngleRad = (angleRad>=0)?angleRad:-angleRad;
	// TODO: presny vypocet kryvky zakruty, nie je to kruh !!!!!

	// arcLength must be always positive
	const float arcLength = radius*absAngleRad;  // mm



	float timeArc; // seconds

	if (radius != 0)
		timeArc = arcLength/transVelocity; 		// Arc
	else{
		timeArc = absAngleRad*180/ transVelocity;// in-place turn
												 // it will take same time as SS90deg turn
		transVelocity = 0;						 // no radius => no arc :)


	}


	insList->command 		= command;
	insList->speed 			= transVelocity;
	insList->dist 			= arcLength;
	insList->accel			= 0;
	insList->angle			= angleRad;
	insList->time			= timeArc;


}

uint16_t INSTR_CmdToInstr( CMD_COMMAND* cmdList, uint16_t* idc, INSTR_INSTRUCTION* insList){

	/*
	 * cmdList 	- is pointer to WHOLE command list!
	 * idc 		- is index of element in cmdList from which translational will begin
	 * insList	- is linked list of instruction which will be filled
	 *
	 * return	- corresponds to quantity of written instruction in to list
	 */


	uint16_t id = 0; //id in insList < 0 ; INSTR_LIST_SIZE>
	uint16_t prevID = 0;
	uint8_t cntStop = 0;

	INSTR_ResetInstrList(insList, INSTR_LIST_SIZE);

	while ( *idc < CMD_LIST_SIZE){

		/* this was used when list have max 16 element
		if( id > ( - MAX_INSTRACTION_PER_COMMAND)){
			return id;

		}*/

		switch(cmdList[*idc].cmd){

			case CMD_STOP:
					// reset set instruction to stop instruction
					id++;
				break;

			//=============== TURNS ================
			// expectted that robot is not moving, just standing at the place
			case CMD_IP45R:
				// first and last instructions are empty

				id++;
				INSTR_AddArc(&insList[id], -45, 0, INSTR_AverageVelocity*2.5, &cmdList[*idc]);
				id +=2;// 1 +1(empty)

				break;
			//--------------------------------------
			case CMD_IP45L:
				// first and last instructions are empty
				id++;
				INSTR_AddArc(&insList[id], +45, 0, INSTR_AverageVelocity*2.5, &cmdList[*idc]);
				id +=2;// 1 +1(empty)

				break;
			//--------------------------------------
			case CMD_IP90R:
				// first and last instructions are empty
				id++;
				INSTR_AddArc(&insList[id], -90, 0, INSTR_AverageVelocity*2.5, &cmdList[*idc]);
				id +=2;// 1(arc) +1(empty)

				break;
			//--------------------------------------
			case CMD_IP90L:
				// first and last instructions are empty
				id++;
				INSTR_AddArc(&insList[id], +90, 0, INSTR_AverageVelocity*2.5, &cmdList[*idc]);
				id +=2;// 1(arc) +1(empty)

				break;
			//--------------------------------------
			case CMD_IP135R:
				// first and last instructions are empty
				id++;
				INSTR_AddArc(&insList[id], -135, 0, INSTR_AverageVelocity*2.5, &cmdList[*idc]);
				id +=2;// 1(arc) +1(empty)

				break;
			//--------------------------------------
			case CMD_IP135L:
				// first and last instructions are empty
				id++;
				INSTR_AddArc(&insList[id], +135, 0, INSTR_AverageVelocity*2.5, &cmdList[*idc]);
				id +=2;// 1(arc) +1(empty)

				break;
			//--------------------------------------
			case CMD_IP180R:
				// first and last instructions are empty
				id++;
				INSTR_AddArc(&insList[id], -180, 0, INSTR_AverageVelocity*2.5, &cmdList[*idc]);
				id +=2;// 1(arc) +1(empty)

				break;
			//--------------------------------------
			case CMD_IP180L:
				// first and last instructions are empty
				id++;
				INSTR_AddArc(&insList[id], +180, 0, INSTR_AverageVelocity*2.5, &cmdList[*idc]);
				id +=2;// 1(arc) +1(empty)

				break;


			//=========== TRANSITIONS ==============
			case CMD_SS90SR:
				INSTR_AddArc(&insList[id], -90, 90*1.15, INSTR_AverageVelocity, &cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_SS90SL:
				INSTR_AddArc(&insList[id], 90, 90*1.15, INSTR_AverageVelocity, &cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_SS180R:
				INSTR_AddArc(&insList[id], -180, 90*1.1, INSTR_AverageVelocity, &cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_SS180L:
				INSTR_AddArc(&insList[id], 180, 90*1.1, INSTR_AverageVelocity, &cmdList[*idc]);
				id++;
				break;
			//--------------------------------------

			case CMD_SD45R:
				INSTR_AddArc(&insList[id], -45, 83*1.2, INSTR_AverageVelocity, &cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_SD45L:
				INSTR_AddArc(&insList[id], 45, 83*1.2, INSTR_AverageVelocity, &cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_SD135R:
				INSTR_AddArc(&insList[id], -135, 76*1.2, INSTR_AverageVelocity, &cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_SD135L:
				INSTR_AddArc(&insList[id], 135, 76*1.2, INSTR_AverageVelocity, &cmdList[*idc]);
				id++;
				break;

			//--------------------------------------
			case CMD_DS45R:
				// diagonalna zatacka
				INSTR_AddArc(&insList[id], -45, 83*1.2, INSTR_AverageVelocity, &cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_DS45L:
				INSTR_AddArc(&insList[id], 45, 83*1.2, INSTR_AverageVelocity, &cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_DS135R:
				//	https://www.desmos.com/calculator/frivebujta
				INSTR_AddArc(&insList[id], -135, 76*1.2, INSTR_AverageVelocity, &cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_DS135L:
				//	https://www.desmos.com/calculator/frivebujta
				INSTR_AddArc(&insList[id], 135, 76*1.2, INSTR_AverageVelocity, &cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_DD90R:
				// r = 45mm * sqrt(2)
				INSTR_AddArc(&insList[id], -90, 64*1.2, INSTR_AverageVelocity, &cmdList[*idc]);
				id++;
				break;
			//--------------------------------------
			case CMD_DD90L:
				// r = 45mm * sqrt(2)
				INSTR_AddArc(&insList[id], 90, 64*1.2, INSTR_AverageVelocity, &cmdList[*idc]);
				id++;
				break;


			default:
				//============== FORWARD ==============
				if( cmdList[*idc].cmd >=  CMD_FWD0 && cmdList[*idc].cmd <= CMD_FWD15){

					// forward N borders of cell
					uint8_t nCells = cmdList[*idc].cmd - CMD_FWD0;

					if(nCells > 3){
						// peak acceleration


						// speed up
						insList[id].command = &cmdList[*idc];
						insList[id].dist   	= CELL_DIMENSION;
						insList[id].speed  	= INSTR_CalcVel(INSTR_AverageVelocity,\
													CELL_DIMENSION, INSTR_MaxTransAccel);
						insList[id].accel  	= INSTR_MaxTransAccel;
						id++;

						// constV
						//const uint8_t cellsConstV = nCells-3;
						// speed-up + speed-down + safeCell;

						insList[id].command	= &cmdList[*idc];
						insList[id].dist   	= cmdList[*idc].dist-CELL_DIMENSION*3;
						insList[id].speed  	= insList[id-1].speed;
						insList[id].accel  	= 0;
						id++;

						// slow-down
						/*
						 * FWD2 sa rata cez kolko hran bunky musi prejst
						 * */
						insList[id].command = &cmdList[*idc];
						insList[id].dist    = 2*CELL_DIMENSION;
						insList[id].speed   = INSTR_AverageVelocity;
						insList[id].accel   = -INSTR_MaxTransAccel;
						id++;


					}else if(cmdList[(*idc)+1].cmd == CMD_STOP){
						// next cmd wil be stop -> so slow down to 80mm/s

						const float slowDownDist = INSTR_CalcDist(INSTR_AverageVelocity, 0, -INSTR_MaxTransAccel);

						insList[id].command = &cmdList[*idc];
						insList[id].dist    = cmdList[*idc].dist;
						insList[id].speed   = INSTR_AverageVelocity;
						insList[id].accel   = INSTR_MaxTransAccel;
						insList[id].slowDownCont = 997-(uint8_t)(100*slowDownDist/cmdList[*idc].dist);
						id++;
					// maybe replace 100 with 102 to have little over shoot to make sure instr will be proceesed to finish


					}else{
						// const velocity

						insList[id].command = &cmdList[*idc];
						insList[id].dist    = cmdList[*idc].dist;
						insList[id].speed   = INSTR_AverageVelocity;
						insList[id].accel  	= 0;
						id++;

					}


				//============== DIAGONAL ==============
				}else if(cmdList[*idc].cmd >=  CMD_DIA0 && cmdList[*idc].cmd <= CMD_DIA31){

					// diagonal N centers of cell
					// Robot starts at one diagonal transition and ends at another so thats why -1
					const uint8_t nCells = cmdList[*idc].cmd - CMD_DIA0 - 1;


					// forward N borders of cell
					if(nCells> 3){
						//  1/2  peak acceleration


						// speed up
						insList[id].command	= &cmdList[*idc];
						insList[id].dist   	= CELL_DIMENSION*SQRT2/2;
						insList[id].speed  	= INSTR_CalcVel(INSTR_AverageVelocity,\
													CELL_DIMENSION*SQRT2/2, INSTR_MaxTransAccel/2);
						insList[id].accel  	= INSTR_MaxTransAccel/2;
						id++;

						// constV
						const uint8_t cellsConstV = nCells-3;
						// speed-up + speed-down + safeCell;

						insList[id].command	= &cmdList[*idc];
						insList[id].dist   	= CELL_DIMENSION*cellsConstV*SQRT2/2;
						insList[id].speed  	= insList[id-1].speed;
						insList[id].accel  	= 0;
						id++;

						// slow-down on 2 cells
						insList[id].command = &cmdList[*idc];
						insList[id].dist   	= 2*CELL_DIMENSION*SQRT2/2;
						insList[id].speed  	= INSTR_AverageVelocity;
						insList[id].accel  	= -INSTR_MaxTransAccel/2;
						id++;


					}else{
						// const velocity

						insList[id].command= &cmdList[*idc];
						insList[id].dist   = nCells*CELL_DIMENSION*SQRT2/2;
						insList[id].speed  = INSTR_AverageVelocity;
						insList[id].accel  = 0;
						id++;

					}


				}else{
					// ERROR, unknown command
					printf("UNNKOWN COMMAND %i with id %i \r\n", (int)&cmdList[*idc], (int) *idc);
				}


		}// end of switch

		for(uint16_t i = prevID ; i<id ;i++ ){
			insList[i].command = &cmdList[*idc];
		}
		prevID = id;


		if(cmdList[*idc].cmd == CMD_STOP){
			cntStop++;
			if(cntStop>5)
				break; // break of while -> return
		}else{
			cntStop = 0;
		}
		*idc += 1;


	}// end of while

	return id;

}

void INSTR_ResetInstrList(INSTR_INSTRUCTION* list, uint16_t length){


	//function set every instruction in list to 0
	for( uint16_t i=0  ; i< length ; i++){

		list[i].command 	= &CMD_commandList[0];
		list[i].dist 		= 0;
		list[i].speed		= 0;
		list[i].accel		= 0;
		list[i].distBegin	= 0;
		list[i].distEnd		= 0;
		list[i].angle		= 0;
		list[i].angleBegin	= 0;
		list[i].angleEnd	= 0;
		list[i].time		= 0;
		list[i].continuance = 0;
		list[i].slowDownCont= 200; // default is never
		list[i].next 		= &list[i+1];

	}

	list[length-1].next = &list[0];
}
