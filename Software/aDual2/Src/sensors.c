/*
 * sensors.c
 *
 *  Created on: Feb 12, 2020
 *      Author: xkacejs
 */





#include "sensors.h"


#define buffLengthADC1 4 //irFL irFR iM1 iM2
#define buffLengthADC2 4 //irDL irDR bat1 bat2
#define buffLengthADC3 2 //irSL irSR

// ADC buffers for DMA
static uint16_t valAdc1[buffLengthADC1];
static uint16_t valAdc2[buffLengthADC2];
static uint16_t valAdc3[buffLengthADC3];
static uint8_t adcReadComplete = 0;

//encoder cnt, val and timmers
static int16_t encoderCarryCnt[2] = {0,0};
static TIM_HandleTypeDef* htimEnc[2];
static int32_t prevDistEnc[2];		//  used for velocity calc
static uint32_t prevTime[2];		//	used for velocity calc
static uint32_t prevVectorTime;

static SENSOR_FILTER currentFilter[2];

static SENSOR_FILTER_Float filterDistanceLeft;
static SENSOR_FILTER_Float filterDistanceRight;

uint8_t SENSORS_irVal2dist( uint16_t, uint8_t);

void SENSORS_Init(){
//in this function define whole configuration


	// ADC buff init
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) valAdc1, buffLengthADC1);
	HAL_ADC_Start_DMA(&hadc2, (uint32_t*) valAdc2, buffLengthADC2);
	HAL_ADC_Start_DMA(&hadc3, (uint32_t*) valAdc3, buffLengthADC3);


	// encoder timmers values
	HAL_TIM_Encoder_Start(&SENSORS_HTIM_ENC_L, TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(&SENSORS_HTIM_ENC_R, TIM_CHANNEL_ALL);
	HAL_TIM_Base_Start_IT(&SENSORS_HTIM_ENC_L);
	HAL_TIM_Base_Start_IT(&SENSORS_HTIM_ENC_R);
	htimEnc[SENSORS_MOTOR_L] = &SENSORS_HTIM_ENC_L;
	htimEnc[SENSORS_MOTOR_R] = &SENSORS_HTIM_ENC_R;
	SENSOR_FilterResetFloat(&filterDistanceLeft);
	SENSOR_FilterResetFloat(&filterDistanceRight);


	encoderCarryCnt[0] = 0;
	encoderCarryCnt[1] = 0;

	// gyro init
	bno055_init(&SENSORS_HI2C, OPERATION_MODE_NDOF);



}

uint8_t SENSORS_test_i2c(){

	volatile float a = bno055_getEulerYaw(&SENSORS_HI2C);
	 a = bno055_getGyroZ(&SENSORS_HI2C);
	 a = bno055_getLinAccelZ(&SENSORS_HI2C);
	return a;// bno_test(&SENSORS_HI2C);
}

void SENSORS_Update(){
// this function read values from sensors and do some calculation then it store in struct (motor, irSensors, position)
// tato funkcia by nemala zabrat viac ako 200us

	//TODO
	//tieto funkcie mozno bude fajn rozbalit a popreplietat aby sa procak mohol pocitat pokial ADC convertuje




	SENSORS_EncoderUpdateValues(); // 5us
	SENSORS_AdcUpdateValues(); // this function takes around 110us
	SENSORS_GyroUpdateValues(); //  call every 10ms

	SENSORS_vectorsCalc(); // 5 us


}

void SENSORS_EncoderUpdateValues(){


	for(uint8_t i=0; i<2 ; i++){

		// get value from counter
		uint32_t periodOfTimmer = htimEnc[i]->Init.Period;
		SENSORS_valEnc[i] =  (encoderCarryCnt[i] * (int64_t)periodOfTimmer) + (uint32_t)htimEnc[i]->Instance->CNT;

		// increments to mm
		SENSORS_dist[i] = SENSORS_valEnc[i] * SENSORS_CONST_INC_MM;

		// calc velocity
		const uint32_t time = MAIN_GetMicros();
		const volatile uint32_t deltaTime = time-prevTime[i];

		SENSORS_velocity[i] = SENSORS_CONST_INC_MM*(1000000 *((float)SENSORS_valEnc[i]-prevDistEnc[i]))/(deltaTime);

		prevTime[i] = time;
		prevDistEnc[i] = SENSORS_valEnc[i];

		// for debug


	}
}

void SENSORS_vectorsCalc(){

	// get time
	const uint32_t time = MAIN_GetMicros();
	const float dTime = (time - prevVectorTime) ;// uS
	prevVectorTime = time;

	// get motor distances
	const float distL0 = (float)SENSORS_valEnc[SENSORS_MOTOR_L] * SENSORS_CONST_INC_MM; //(float)SENSORS_dist[SENSORS_MOTOR_L];
	const float distR0 = (float)SENSORS_valEnc[SENSORS_MOTOR_R] * SENSORS_CONST_INC_MM; //(float)SENSORS_dist[SENSORS_MOTOR_R];

	SENSOR_FilterAddFloat(distL0, &filterDistanceLeft);
	SENSOR_FilterAddFloat(distR0, &filterDistanceRight);

	const float distL = SENSOR_FilterGetFloat(&filterDistanceLeft);
	const float distR = SENSOR_FilterGetFloat(&filterDistanceRight);



	// precalculation
	const float newTransPos 	= (distR + distL)/2;
	const float newTransVel	 	= (1000000.0/dTime)*(newTransPos-SENSORS_transPos);
	const float newTransAccel 	= (1000000.0*(newTransVel-SENSORS_transVel))/dTime;
	const float newTransJerk 	= (1000000.0*(newTransAccel-SENSORS_transAccel))/dTime;


	const float newAnglePos 	= (distR - distL)/ WHEEL_PITCH;
	const float newAngleVel	 	= (1000000*(newAnglePos-SENSORS_anglePos))/dTime;
	const float newAngleAccel 	= (1000000*(newAngleVel-SENSORS_angleVel))/dTime;
	const float newAngleJerk 	= (1000000*(newAngleAccel-SENSORS_angleAccel))/dTime;

	// simple kalman filter
	static float filtTransVel  = 0;
	static float filtAngleVel  = 0; // it is static...
	filtTransVel = filtTransVel + (newTransVel - filtTransVel)/VELOCITY_FILTER_COEFICIENT;
	filtAngleVel = filtAngleVel + (newAngleVel - filtAngleVel)/VELOCITY_FILTER_COEFICIENT;

	// update
	SENSORS_transPos 	= newTransPos;
	SENSORS_transVel 	= filtTransVel;
	SENSORS_transAccel 	= newTransAccel;
	SENSORS_transJerk 	= newTransJerk;

	// TODO ak chces cez gyro filtrovat aj uhol musis sa usitit ze na zaciatku bol IRC a GYRO na rovnako...
	//SENSORS_anglePos 	= newAnglePos;
	SENSORS_angleVel 	= (2*filtAngleVel + 8*SENSORS_xAngularVelocity)/10 ;
	SENSORS_anglePos 	+= SENSORS_angleVel *dTime/1e6;
	SENSORS_angleAccel 	= newAngleAccel;
	SENSORS_angleJerk 	= newAngleJerk;


	/*
	uint8_t s[30];//7*3+3
	const uint8_t len =  sprintf((char*)s,"%.1f\t %.2f\t %.2f\t \r\n", distL0, SENSORS_xAngularVelocity, dTime/1000 );
	HAL_UART_Transmit(&huart3, s, len, 10);
	*/

}

void SENSORS_GyroUpdateValues(){
	// This function takes 0.8ms - 1.2ms to processed


	static volatile uint32_t lastFcnCall = 0;
	const uint32_t time = MAIN_GetMicros();

	if(time - lastFcnCall > SENSORS_IMU_uTIMEOUT){
		//TODO: sync IRC and xAngle
		//SENSORS_xAngle = 100 * bno055_getEulerYaw(&SENSORS_HI2C);
		SENSORS_xAngularVelocity = bno055_getGyroZ(&SENSORS_HI2C);
		lastFcnCall = time;
	}
}

void SENSORS_AdcUpdateValues(){

	uint8_t uHalfPeriodOfLight = 60;
	//========================== ADC READING ==========================
	SENSORS_batVal[0] = valAdc2[2];
	SENSORS_batVal[1] = valAdc2[3];


	SENSORS_iSenseVal[0] = valAdc1[2];
	SENSORS_iSenseVal[1] = valAdc1[3];
	int16_t irAdcToogleVal[3][6];

	// blinking
	HAL_GPIO_WritePin(GPIO_IR_LED1_GPIO_Port, GPIO_IR_LED1_Pin,  GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIO_IR_LED2_GPIO_Port, GPIO_IR_LED2_Pin,  GPIO_PIN_RESET);

	// read adc ir sens
	SENSORS_irRead((uint16_t*) &irAdcToogleVal[0][0]);													// # reading 00 - toto by slo vynechat

	// get system clock
	uint32_t clk_start = MAIN_GetMicros();
	// turn on light
	HAL_GPIO_WritePin(GPIO_IR_LED1_GPIO_Port, GPIO_IR_LED1_Pin,  GPIO_PIN_SET);

	while ((MAIN_GetMicros() - clk_start) < uHalfPeriodOfLight);

	//--------------------------------------

	adcReadComplete = 0;
	// wait until reads ends
	while(adcReadComplete == 0b111);
	// read adc ir sens
	SENSORS_irRead((uint16_t*) &irAdcToogleVal[1][0]);													// # reading 10

	// current
	SENSORS_iSenseVal[0] += valAdc1[2];
	SENSORS_iSenseVal[1] += valAdc1[3];

	// get system clock
	clk_start = MAIN_GetMicros();
	// turn on light
	HAL_GPIO_WritePin(GPIO_IR_LED1_GPIO_Port, GPIO_IR_LED1_Pin,  GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIO_IR_LED2_GPIO_Port, GPIO_IR_LED2_Pin,  GPIO_PIN_SET);		//wait 20us to rise
	// wait 20us
	while ((MAIN_GetMicros() - clk_start) < uHalfPeriodOfLight);
	//--------------------------------------

	adcReadComplete = 0;
	// wait until reads ends
	while(adcReadComplete == 0b111);
	// read adc ir sens
	SENSORS_irRead((uint16_t*) &(irAdcToogleVal[2][0]));													// # reading 01

	HAL_GPIO_WritePin(GPIO_IR_LED2_GPIO_Port, GPIO_IR_LED2_Pin,  GPIO_PIN_RESET);

	// current
	SENSORS_iSenseVal[0] += valAdc1[2];
	SENSORS_iSenseVal[1] += valAdc1[3];

	//END of ADC reading

	//======================== DATA PROCESSING ========================

	// ir difference
	for( uint8_t i = 0 ; i < 6; i++){
		SENSORS_irVal[i] = irAdcToogleVal[1+ ((i+1)%2)][i] - irAdcToogleVal[0][i];
		SENSORS_irDistance[i] = SENSORS_irVal2dist(SENSORS_irVal[i], i);
	}



	// baterry voltage and motor curent  calculation
	for(uint8_t i = 0 ; i < 2 ; i++){
		SENSOR_FilterAdd(SENSORS_iSenseVal[i]/3, &currentFilter[i]);
		const uint16_t avgCurrent =SENSOR_FilterGet(&currentFilter[i]);

		SENSORS_motorI[i]   = avgCurrent * SENSORS_CONST_B_TO_U * 1000 * SENSORS_CONST_MOTOR_I;
		SENSORS_batteryV[i] = SENSORS_batVal[i]    * SENSORS_CONST_B_TO_U * 1000 * SENSORS_CONST_BAT_DIV;

	}





}


uint8_t SENSORS_irVal2dist( uint16_t val, uint8_t id){
	// return value is in mm

	/*
	const float A[6] = {  80, 133, 119, 105, 162, 142};
	const float B[6] = { 341, 188, 168, 149, 230, 201};
	const float C[6] = {1158, 144, 774, 541, 608, 783};

	const float tmp = (val-C[id])/B[id];
	const float  distance = A[id] / (tmp*tmp);
	 */

	const float th[6] = { 900, 0, 0, 0, 0, 1140};
	float  distance;

	if(val < th[id]){
		// distance > ~20mm

		const float A[6] = { 2910, 2168, 2650	, 1710, 2927, 3560};
		const float C[6] = { 240, 188, 807		, 776, 230, 170};
		const float D[6] = { 16.5, 0, -8.9		, -7.1, 0, 15};

		const float tmp = (val-C[id])/A[id];
		distance = 1/(tmp*tmp) + D[id];

	}else{
		// distance < ~20mm

		const float A[6] = { 12300 , 2168, 2650	, 1710, 2927, 10000};
		const float C[6] = {-835, 188, 807		, 776, 230, -500};
		const float D[6] = {-12.7, 0, -8.9		, -7.1, 0, -8.7};

		const float tmp = (val-C[id])/A[id];
		distance = 1/(tmp*tmp) + D[id];

	}


	if(distance < 0 )
		return 0;
	if(distance > 200)
		return 200;

	return (uint8_t)distance;


}


void SENSORS_irRead(uint16_t* sensorArray){

	sensorArray[0] = valAdc1[1]; // FR
	sensorArray[1] = valAdc2[1]; // DR
	sensorArray[2] = valAdc3[1]; // SR
	sensorArray[3] = valAdc3[0]; // SL
	sensorArray[4] = valAdc2[0]; // DL
	sensorArray[5] = valAdc1[0]; // FL

}

void SENSORS_AdcCallback(ADC_HandleTypeDef* hadc){


	if (hadc->Instance == hadc1.Instance)
			adcReadComplete |= 0b001;
	if (hadc->Instance == hadc2.Instance)
			adcReadComplete |= 0b010;
	if (hadc->Instance == hadc3.Instance)
			adcReadComplete |= 0b100;

}

void SENSORS_EncoderTimCallback(TIM_HandleTypeDef *htim){
	/*
	 *  This function should be called from HAL_TIM_PeriodElapsedCallback, if htim is encoder timmer
	 */

		 uint32_t counterValue = htim->Instance->CNT;
		 uint32_t periodOfTimmer = htim->Init.Period;

		 if( counterValue > periodOfTimmer/2){
			 // underflow of timmer
			 // if argument is tim1 ==> [0]--
			 // else [1]--
			 encoderCarryCnt[ (htim->Instance == htimEnc[0]->Instance)? 0 : 1 ]--;
		 }else{
			 // overflow of timmer
			 // if argument is tim1 ==> [0]++
			 // else [1]++
			 encoderCarryCnt[ (htim->Instance == htimEnc[0]->Instance)? 0 : 1 ]++;
		 }

}




void SENSOR_FilterAdd(uint16_t data, SENSOR_FILTER* filter){
	filter->data[filter->id] = data;
	filter->id++;
	filter->id%=SENSOR_FILTER_LENGTH;
}
uint16_t SENSOR_FilterGet(SENSOR_FILTER* filter){

	uint32_t cnt=0;
	for(int i=0; i< SENSOR_FILTER_LENGTH ; i++)
		cnt += filter->data[i];

	return cnt/SENSOR_FILTER_LENGTH;
}

void SENSOR_FilterAddFloat(float data, SENSOR_FILTER_Float* filter){
	filter->data[filter->id] = data;
	filter->id++;
	filter->id%=SENSOR_FILTER_LENGTH;
}
float SENSOR_FilterGetFloat(SENSOR_FILTER_Float* filter){

	float cnt=0;
	for(int i=0; i< SENSOR_FILTER_LENGTH ; i++)
		cnt += filter->data[i];

	return cnt/SENSOR_FILTER_LENGTH;
}

void SENSOR_FilterResetFloat(SENSOR_FILTER_Float* filter){
		for(int i=0; i< SENSOR_FILTER_LENGTH ; i++)
			filter->data[i]=0;

}
