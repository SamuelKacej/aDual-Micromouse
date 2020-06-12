/*
 * motionSystem.c
 *
 *  Created on: Feb 25, 2020
 *      Author: xkacejs
 */

#include "motionSystem.h"




static uint32_t prevTime;
// TODO : how to set init Motion _instr ID;

void MOTION_Init(float tP,float tI,float tD,float aP,float aI,float aD){



	 INSTR_VelocityPeak	 	= 1500;	// mm/s , < 1000 ; 3500 > Translational
	 INSTR_AverageVelocity 	= 500; 	// mm/s , <  250 ; 1500 > Translational

	 INSTR_MaxTransAccel 	= 9100;	// mm/s    1 g =~ 9807mm/s

	 INSTR_MaxAngVelocity 	= 2*PI;	// rad/s

	 //in place turn acceleration

	 MOTION_instrID = 2*INSTR_LIST_HALF_SIZE -1;
	 MOTION_ExternalAngCorrection = 0;
	 insList = INSTR_InstrList;

	 MOTION_RequestNewInstruction = 0;


	// set parameters;
	// angularController, translationalController;
	MOTOR_ControllerSet(&MOTION_translationController, tP, tI, tD);
	MOTION_translationController.Tx = 0;
	MOTION_translationController.maxU = 6000;
	MOTION_translationController.minU = -6000;
	MOTION_translationController.b = 1;
	MOTION_translationController.c = 1;

	MOTOR_ControllerSet(&MOTION_angularController, aP, aI, aD);
	MOTION_angularController.Tx = 0;
	MOTION_angularController.maxU = 80;//rad/s
	MOTION_angularController.minU = -80;
	MOTION_angularController.b = 1;
	MOTION_angularController.c = 1;



}

void MOTION_Update(){
	/*
	 * This funciton do some magic, this function should be called periodically ! (~5ms)
	 *  1. chcek if was last instraction finished and new one should be processed
	 *  2. processed selectd instraction (MOTION_instrID)
	 */


	// calc period

	//printIstr(MOTION_instrID);



	const uint32_t time = MAIN_GetMicros();
	MOTION_uTimePeriod = (time-prevTime);// us
	prevTime = time;


	// Chcek if actual instraction was finished
	MOTION_ChcekForNewInstraction();

	if(MOTION_instrID%2)
		ACTUATOR_LED(0, 0, 130);
	else
		ACTUATOR_LED(0, 130, 0);

	// processed actauall instraction
	MOTION_ProcessedInstraction(&insList[MOTION_instrID]);


}

void MOTION_UpdateList(){
	/*
	 * CALL IT FROM MAIN THREAD
	 * DO NOT CALL IT PERIODICALLY, BY TIMMER !!!
	 *
	 * This function wil chose if half of instruction list can be precaltulated
	 * precalculation of half can be done only if is processed  {<1; ListSize/2)}
	 *
	 *
	 */

	if(INSTR_ListAlreadyUpdated == 0){

		if(MOTION_instrID < INSTR_LIST_HALF_SIZE){
			// first half of list is processed, so second wil be updated
			if(MOTION_instrID > 0)
				INSTR_FillHalfList(1);
		}else{
			if(MOTION_instrID > INSTR_LIST_HALF_SIZE)
				INSTR_FillHalfList(0);
		}

		// reset in MOTION_MoveInstrId
		INSTR_ListAlreadyUpdated = 1;
	}
}

static void MOTION_ProcessedInstraction(INSTR_INSTRUCTION* instrActual){ // updated
	/*
	 * Do acceleration by highest non zero derivation
	 * calc velocity, set velocity to motors
	 *
	 */


	// calc Velocity
	float newVelA, newVelT;
	MOTION_StepVelocity(instrActual, &newVelT, &newVelA);

	if(MAIN_GetMicros()/1e6 < 1)
	//printf(" %i,\t %.2f, \t %.2f, \t %.2f, \t %.2f\n\r",\
			MAIN_GetMicros()/1000, newVelT, SENSORS_transVel, newVelA*100, SENSORS_angleVel*100);

	// set volocity
	MOTION_SetVelocity(newVelT, newVelA + MOTION_ExternalAngCorrection);


}

static float MOTION_SinusoidalVel(float phi, float time, float continuance){
	/*
	 * Return angular velocity for turn manever at given continuance [rad/s]
	 * phi - final angle of manever [rad]
	 * time - total time for manever [s]
	 * continuance - define relative time <0:1>
	 *
	 * https://www.desmos.com/calculator/rcfvcpdcxe
	 *
	 */
	//time = 1; // we calculate relative continuance, not velocity in total time;
	return  phi * M_PI * sin(M_PI*continuance) / (2*time);
}

static void MOTION_StepVelocity(INSTR_INSTRUCTION* instr, float* transVel, float* angularVel){ //updated
	/*
	 * transVel and angularVel are return parameter for calculated velocities.
	 * instr is actual instruction
	 */


	// YOU CANT ACCELERATE IN TURN !!
	if(instr->angle != 0){
		// rotation


		// not lieanar measure
		float tmpContinuanceAngle = (SENSORS_anglePos - instr->angleBegin)/ instr->angle;
		if(tmpContinuanceAngle < 0)
			tmpContinuanceAngle = 0;
		if(tmpContinuanceAngle > 1)
			tmpContinuanceAngle = 1;

		// normal values are in interval <0;1>

										// this is inverse function of angle function
		const float angleContinuance = acos(1- 2*tmpContinuanceAngle)/ M_PI;
		const float transContiunacne = (SENSORS_transPos - instr->distBegin)/ instr->dist;



		if(instr->dist == 0){
			// In-place turn
			*transVel = 0;

			*angularVel = MOTION_SinusoidalVel(instr->angle, instr->time, angleContinuance);
			// TODO overshoot protection

		}else{
			// Arc turn
			// angular vector is controled by translational vector, for synchronization
			*angularVel = MOTION_SinusoidalVel(instr->angle, instr->time, transContiunacne);
			// TODO overshoot protection

			// translational vector is controled by angular vector, for synchronization;
			*transVel = instr->speed * pow(2,(-(transContiunacne - angleContinuance)));

			//printf("%.2f,\t %.2f,\t %.2f,\t %.2f,\t %.2f\r\n", angleContinuance, transContiunacne, *angularVel, *transVel, SENSORS_anglePos);
			//printf("%i,\t %.2f,\t %.2f\r\n", MAIN_GetMicros()/1000, SENSORS_transPos, SENSORS_anglePos);



		}
	} else if (instr->accel !=0){
		//speed up

		// time instance is used as counter.

		// todo time sa musi vynulovat?
		instr->time +=  MOTION_uTimePeriod*(1e-6);
		*transVel = instr->accel * instr->time;

		// overshoot protection
		if(instr->accel > 0 && *transVel > instr->speed)
			*transVel = instr->speed;
		if(instr->accel < 0 && *transVel < instr->speed)
			*transVel = instr->speed;

		*angularVel = 0; // rotation is not allowed

	}else{
		// const speed
		*angularVel = 0;
		*transVel = instr->speed;
	}


}

void MOTION_ChcekForNewInstraction(){ // updated
	/*
	 * Chcek if finishing condition was set
	 */

	// if variable is 1 id in list will be moved
	volatile uint8_t NewInstraction  = MOTION_RequestNewInstruction;

	const float distTrans = SENSORS_transPos;
	const float distAng	  = SENSORS_anglePos;

	// main should send REQUSET to new instraction
	// this should be activated only if robot travel to long distance
	//const float distReserv = 50;//mm
	//const float angleReserv = 3; //rad


	// translational movement
	if( insList[MOTION_instrID].speed != 0 && distTrans >= insList[MOTION_instrID].distEnd) //+ distReserv)
		NewInstraction = 1;

	// anti-clockwise
	if( insList[MOTION_instrID].angle > 0 && distAng >= insList[MOTION_instrID].angleEnd)
		NewInstraction = 1;

	// clockwise
	if( insList[MOTION_instrID].angle < 0 && distAng <= insList[MOTION_instrID].angleEnd)
			NewInstraction = 1;

	// empty instruction
	if( insList[MOTION_instrID].dist == 0 && insList[MOTION_instrID].angle == 0)
		NewInstraction = 1;


	if(NewInstraction > 0)	{


		MOTION_NewInstraction();
		// reset external request
		MOTION_RequestNewInstruction = 0;

	}
}

static uint8_t MOTION_NewInstraction(){


	//TODO : confirm that half-list was pre-calculated,
	//		error return

	const float distTrans = SENSORS_transPos;
	const float distAng	  = SENSORS_anglePos;

	// move circularly id
	MOTION_MoveInstrId();

	insList[MOTION_instrID].distBegin 	= distTrans;
	insList[MOTION_instrID].distEnd 	= distTrans + insList[MOTION_instrID].dist;
	insList[MOTION_instrID].angleBegin 	= distAng;
	insList[MOTION_instrID].angleEnd 	= distAng + insList[MOTION_instrID].angle;

	return 0;
}

void MOTION_resetList(int id){
	insList[id].command 	= CMD_STOP;
	insList[id].dist 		= 0;
	insList[id].speed		= 0;
	insList[id].accel		= 0;
	insList[id].distBegin	= 0;
	insList[id].distEnd		= 0;
	insList[id].angle		= 0;
	insList[id].angleBegin	= 0;
	insList[id].angleEnd	= 0;
	insList[id].time		= 0;

}

void MOTION_uTurnTest(){
//Z turne

	const int vel = 900;

	int id = 0;
	MOTION_resetList(id);
	id++;


	MOTION_resetList(id);
	insList[id].command = CMD_FWD0 + 1;
	insList[id].dist   	= CELL_DIMENSION;
	insList[id].speed  	= vel;
	insList[id].accel	= vel*vel/2/CELL_DIMENSION;
	id++;

	MOTION_resetList(id);
	insList[id].command = CMD_FWD0 + 1;
	insList[id].dist   	= CELL_DIMENSION;
	insList[id].speed  	= 2*vel;
	insList[id].accel	= 0;
	id++;

/*
				//    list,  angle, radius, velocity ,
	INSTR_AddArc(&insList[id], - 135, 73, vel, CMD_SD135R);
	id++;


	MOTION_resetList(id);
	insList[id].command = CMD_FWD0 + 1;
	insList[id].dist   	= 1* CELL_DIMENSION * 0.707;
	insList[id].speed  	= vel;
	id++;

	INSTR_AddArc(&insList[id], 135, 73, vel, CMD_DS135L);
	id++;

	MOTION_resetList(id);
	insList[id].command = CMD_FWD0 + 1;
	insList[id].dist   	= 1*CELL_DIMENSION;
	insList[id].speed  	= 0;
	insList[id].accel	= -vel*vel/2/CELL_DIMENSION;
	id++;
*/

	MOTION_resetList(id);
	insList[id].command = CMD_FWD0 + 1;
	insList[id].dist   	= 1*CELL_DIMENSION;
	insList[id].speed  	= 0;
	insList[id].accel	= -vel*vel/CELL_DIMENSION;
	id++;

	MOTION_resetList(id);
	id++;


	MOTION_resetList(id);
	INSTR_InstrListUsedInstr[0] = id-1;
	INSTR_InstrListUsedInstr[1] = 0;

}

static void MOTION_MoveInstrId(){// updated
	/*
	 *  circularly move MOTION_instrID
	 *  it chcek if THE new instruction was pre-calculated
	 *   (there maybe was not enought space in half list
	 *   to translate full command, so list need to be cut)
	 *
	 */



	// circular move
	MOTION_instrID = (MOTION_instrID + 1) % (2*INSTR_LIST_HALF_SIZE);

	// does it use first or second half of list
	const uint8_t halfList = (MOTION_instrID < INSTR_LIST_HALF_SIZE)? 0 : 1 ;

	//id in half of the list
	const uint8_t relativeID = MOTION_instrID - halfList * INSTR_LIST_HALF_SIZE;

	// example: want to go to 3rd instr (relID =  2), but pre-calculated was only 2 instr
	if( relativeID >= INSTR_InstrListUsedInstr[halfList]){
		// new instruction was not pre-calculated
		// so go to begin of other half
		MOTION_instrID = (halfList == 0 )? INSTR_LIST_HALF_SIZE : 0 ;
	}


	// if u are at begin of list:
	// allow precalculation of other half  list instractions
	if(MOTION_instrID == 0 || MOTION_instrID == INSTR_LIST_HALF_SIZE)
		INSTR_ListAlreadyUpdated = 0;// other list is not pre-calculated
}

void MOTION_SetVelocity(float transV, float angularV){
	/*
	 * translational velocity and angular velocity will be processed
	 * though controller and motorVelocity R and L wil be calculated
	 *
	 */


	// PID update

	// TODO: adaptive pid regulator

	/* TODO VYLADIT REGULATOR sorry nestiham do odovzdania vyladit tento regulator
	MOTOR_ControllerUpdate(&MOTION_angularController, angularV, SENSORS_angleVel);
	MOTOR_ControllerUpdate(&MOTION_translationController, transV, SENSORS_transVel);

	float velL_req = MOTION_translationController.U - MOTION_angularController.U*WHEEL_PITCH/2;
	float velR_req = MOTION_translationController.U + MOTION_angularController.U*WHEEL_PITCH/2;

	if(transV == 0 && angularV==0)
		 velL_req = velR_req = 0;
	*/
	float velL_req = transV - angularV*WHEEL_PITCH/2;
	float velR_req = transV + angularV*WHEEL_PITCH/2;

	//printf("%i\t, %i\t, %.2f\t, %.2f \r\n", (int)velL_req, (int)velR_req, transV, angularV);

	MOTOR_SetVelocity(MOTOR_ML, velL_req);
	MOTOR_SetVelocity(MOTOR_MR, velR_req);

}

void MOTION_inPlaceRotation(int angleDeg){

	const int vel = 500;

	int id = 0;
	MOTION_resetList(id);
	id++;

	INSTR_AddArc(&insList[id], angleDeg, 0, vel, CMD_SD135R);
	id++;

	MOTION_resetList(id);
	id++;
	MOTION_resetList(id);

	INSTR_InstrListUsedInstr[0] = id-1;
	INSTR_InstrListUsedInstr[1] = 0;

	MOTION_UpdateList();

	ACTUATOR_LED(-1, -1, 200);
	uint32_t tStart = MAIN_GetMicros();

	// delay
	while(tStart + 3000000 > MAIN_GetMicros())
		;

	ACTUATOR_LED(-1, -1, 0);

}
