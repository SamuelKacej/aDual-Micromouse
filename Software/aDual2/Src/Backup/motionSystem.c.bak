/*
 * motionSystem.c
 *
 *  Created on: Feb 25, 2020
 *      Author: xkacejs
 */

#include "motionSystem.h"




static uint32_t prevTime;

void MOTION_Init(float tP,float tI,float tD,float aP,float aI,float aD){



	 INSTR_VelocityPeak	 	= 1500;	// mm/s , < 1000 ; 3500 > Translational
	 INSTR_AverageVelocity 	= 500; 	// mm/s , <  250 ; 1500 > Translational

	 INSTR_MaxTransAccel 	= 5000;	// mm/s    1 g =~ 9807mm/s

	 INSTR_MaxAngVelocity 	= 2*PI;	// rad/s

	 //in place turn acceleration

	 MOTION_instrID = INSTR_LIST_SIZE -1;
	 MOTION_ExternalAngCorrection = 0;
	 MOTION_ExternalTransCorrection = 0;
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

	const uint32_t time = MAIN_GetMicros();
	MOTION_uTimePeriod = (time-prevTime);// us
	prevTime = time;


	// Chcek if actual instraction was finished
	MOTION_ChcekForNewInstraction();

	// processed actauall instraction
	MOTION_ProcessedInstraction(&insList[MOTION_instrID]);


}

void MOTION_UpdateList(){
	/*
	 * THIS FUNCTION SHOULD BE CALLED ONCE
	 *
	 *
	 *
	 * return 1 if list was updated
	 * return 0 if list was not updated
	 * CALL IT FROM MAIN THREAD
	 * DO NOT CALL IT PERIODICALLY, BY TIMMER !!!
	 *
	 * This function wil chose if half of instruction list can be precaltulated
	 * precalculation of half can be done only if is processed  {<1; ListSize/2)}
	 *
	 *
	 *
	 */
/*
	if(INSTR_ListAlreadyUpdated == 0){

		if(MOTION_instrID < INSTR_LIST_HALF_SIZE){
			// first half of list is processed, so second wil be updated
			if(MOTION_instrID >= 0) // originalne tu bolo >, mozno tu ma byt >=
				INSTR_FillHalfList(1);
		}else{
			if(MOTION_instrID >= INSTR_LIST_HALF_SIZE)
				INSTR_FillHalfList(0);
		}

		// reset in MOTION_MoveInstrId
		INSTR_ListAlreadyUpdated = 1;
		return 1;
	}

	return 0;*/

	return INSTR_FillList(INSTR_InstrList);
}


void MOTION_ProcessedInstraction(INSTR_INSTRUCTION* instrActual){ // updated
	/*
	 * Do acceleration by highest non zero derivation
	 * calc velocity, set velocity to motors
	 *
	 */


	// calc Velocity

	//float newVelT = SENSORS_transVel;
	//float newVelA = SENSORS_angleVel;
	float newVelT, newVelA;
	MOTION_StepVelocity(instrActual, &newVelT, &newVelA);

	printf("%.1f, %.1f\r\n", SENSORS_transVel, newVelT);


	// set volocity
	MOTION_SetVelocity(newVelT + MOTION_ExternalTransCorrection, newVelA + MOTION_ExternalAngCorrection);
	MOTION_ExternalAngCorrection = 0; // reset correction
	MOTION_ExternalTransCorrection = 0; // reset correction


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

void MOTION_StepVelocity(INSTR_INSTRUCTION* instr, float* transVel, float* angularVel){ //updated
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

			instr->continuance = (angleContinuance)*100;
			//if u stop you will have allways continuance 0 (you can't start)
			// I think only this bridge of IF does need this initial step
			const float tmpContinuanceAngle1 = (tmpContinuanceAngle<0.02)? 0.02: angleContinuance;

			// In-place turn
			*transVel = 0;

			*angularVel = MOTION_SinusoidalVel(instr->angle, instr->time, tmpContinuanceAngle1 );
			// TODO overshoot protection

		}else{

			instr->continuance = (angleContinuance+transContiunacne)*100/2;
			// Arc turn
			// angular vector is controled by translational vector, for synchronization
			*angularVel = MOTION_SinusoidalVel(instr->angle, instr->time, transContiunacne);
			// TODO overshoot protection

			// translational vector is controled by angular vector, for synchronization;
			*transVel = instr->speed * pow(2,( angleContinuance - transContiunacne));

		}
	} else if (instr->accel !=0){


		*angularVel = 0; // rotation is not allowed

		instr->continuance = 100*(SENSORS_transPos - instr->distBegin)/ instr->dist;

		static float slowDownCNT = 0;
		//speed up

		if(instr->continuance <= instr->slowDownCont){
			// time instance is used as counter.
			// TODO time sa musi vynulovat?
			instr->time +=  MOTION_uTimePeriod*(1e-6);
			*transVel = instr->accel * instr->time;// vzdy to pojde od 0-to je trocha zle

			// overshoot protection
			if(instr->accel > 0 && *transVel > instr->speed)
				*transVel = instr->speed;
			if(instr->accel < 0 && *transVel < instr->speed)
				*transVel = instr->speed;
			slowDownCNT = 0;
		}else{// slow down if requested
			slowDownCNT +=  MOTION_uTimePeriod*(1e-6);
			*transVel =  instr->speed - instr->accel * slowDownCNT;

			// overshoot protection
			if(instr->accel > 0 && *transVel < 20) // you are using -accel
				*transVel = 20;
			if(instr->accel < 0 && *transVel > -20)
				*transVel = -20;

			}


	}else{
		// const speed
		*angularVel = 0;

		// speed up if actual velocity and target are different
		// todo proper acceleration with instr->time
		if(SENSORS_transVel < ((instr->speed)-200)){
			*transVel = SENSORS_transVel + 120;
		}else if(SENSORS_transVel > ((instr->speed)+200)){
			*transVel = SENSORS_transVel - 120;
		}else{
			*transVel = instr->speed;
		}

		instr->continuance = 100*(SENSORS_transPos - instr->distBegin)/ instr->dist;
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

	// MAIN (MOUSE.h) should send REQUSET to new instraction
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


	if(NewInstraction > 0 || insList[MOTION_instrID].command->cmd == CMD_STOP)	{


		MOTION_NewInstraction();
		// reset external request
		MOTION_RequestNewInstruction = 0;

	}
}

uint8_t MOTION_NewInstraction(){


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
	insList[id].command 	= CMD_STOP; // TODO create default struct cmd
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

static uint8_t MOTION_GetNextInstrID(uint8_t idOffset){
	// TODO recreate this fcn
	// function return id of next element in MOTION_instrID
	// idOffset -> offset from actual instruction (0 = actual instr, 1 = next instr, ...)

	// circular move
	uint8_t tmpID = (MOTION_instrID + idOffset) % (INSTR_LIST_SIZE);


	/* OLD instr list
	// does it use first or second half of list
	const uint8_t halfList = (tmpID < INSTR_LIST_HALF_SIZE)? 0 : 1 ;

	//id in half of the list
	const uint8_t relativeID = tmpID - halfList * INSTR_LIST_HALF_SIZE;

	// example: want to go to 3rd instr (relID =  2), but pre-calculated was only 2 instr
	if( relativeID >= INSTR_InstrListUsedInstr[halfList]){
		// new instruction was not pre-calculated
		// so go to begin of other half
		tmpID = (halfList == 0 )? INSTR_LIST_HALF_SIZE : 0 ;
	}
	*/

	return tmpID;
}

void MOTION_MoveInstrId(){// updated
	/*
	 *  circularly move MOTION_instrID
	 *  it chcek if THE new instruction was pre-calculated
	 *   (there maybe was not enought space in half list
	 *   to translate full command, so list need to be cut)
	 *
	 */


	// reset finished element
	INSTR_ResetInstrList(&INSTR_InstrList[MOTION_instrID], 1);
	MOTION_instrID = (MOTION_instrID +1 ) % INSTR_LIST_SIZE;




			/*
			 * MOTION_instrID =MOTION_GetNextInstrID(1);

	// if u are at begin of list:
	// allow precalculation of other half  list instractions
	if(MOTION_instrID == 0 || MOTION_instrID == INSTR_LIST_HALF_SIZE)
		INSTR_ListAlreadyUpdated = 0;// other list is not pre-calculated
	*/
}

INSTR_INSTRUCTION* MOTION_GetNextInstruction(uint8_t idOffset){

	return &insList[MOTION_GetNextInstrID(idOffset)];
}

void MOTION_SetVelocity(float transV, float angularV){
	/*
	 * translational velocity and angular velocity will be processed
	 * though controller and motorVelocity R and L wil be calculated
	 *
	 */


	// PID update

	// TODO: adaptive pid regulator

	// TODO feedforward !!!! Medium priority


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

	INSTR_InstrListUsedInstr = id-1;

	MOTION_UpdateList();

	ACTUATOR_LED(-1, -1, 200);
	uint32_t tStart = MAIN_GetMicros();

	// delay
	while(tStart + 3000000 > MAIN_GetMicros())
		;

	ACTUATOR_LED(-1, -1, 0);

}
