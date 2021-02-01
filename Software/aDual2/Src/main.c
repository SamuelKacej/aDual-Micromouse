/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "sdio.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "MY_FLASH.h"
#include "sensors.h"
#include "maze.h"
#include "actuator.h"
#include "motor.h"
#include "motionSystem.h"
#include "BNO055.h"
#include "mouse.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint8_t vTest;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
void MAIN_SetPIDs(){

// current
  MOTOR_ControllerSet( &MOTOR_currentController[0], 9, 0.4, 0); // I_org = 0.15
  MOTOR_currentController[0].b = 0.8;	// source of P error variable
  MOTOR_currentController[0].c = 1;

  MOTOR_ControllerSet( &MOTOR_currentController[1], 9, 0.4, 0); // I_org = 0.15
  MOTOR_currentController[1].b = 0.8;	// source of P error variable
  MOTOR_currentController[1].c = 1;

  // velocity
  MOTOR_ControllerSet( &MOTOR_velocityController[1], 3e-5, 3e-7, 0);
  MOTOR_velocityController[1].b = 1;
  MOTOR_velocityController[1].c = 1;

  MOTOR_ControllerSet( &MOTOR_velocityController[0], 3e-5, 3e-7, 0);
   MOTOR_velocityController[0].b = 1;
   MOTOR_velocityController[0].c = 1;

// vector velocity
  MOTION_Init(0.8, 0.05, 0, 0.8, 0.1, 0);

}
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

	FATFS myFatFS;
	FIL myFile;
	UINT myBytes;

	uint16_t val;
	volatile uint32_t MAIN_tmp;
	float transV = 0; // mm/s
	float angularV = 0 ; 	//rad/s

   // volatile uint32_t MAIN_tPeriod;


#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
int _write(int file, char* data, int len){
	 // arbitrary timeout 1000
	   HAL_StatusTypeDef status =
		  HAL_UART_Transmit(&huart3, (uint8_t*)data, len, 1000);

	   // return # of bytes written - as best we can tell
	   return (status == HAL_OK ? len : 0);
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */



  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  MX_USART3_UART_Init();
  MX_TIM9_Init();
  MX_TIM14_Init();
  MX_TIM2_Init();
  MX_TIM12_Init();
  /* USER CODE BEGIN 2 */


  HAL_TIM_Base_Start_IT(&htim2);// microSeconds timmer

  // INIT LBRS
  MAIN_tStart = 0;
  MAIN_seconds= 0;
  SENSORS_Init();
  ACTUATOR_Init();
  MOTOR_INIT();

  MAIN_SetPIDs();
  MAZE_ClearMaze(MAZE_maze); //odstranil som &

  // INIT CONTROLLERS

  // INIT PERIODIC TIMMERS


  HAL_TIM_Base_Start_IT(&htim12);// 5ms periodic timmer for controller update

 HAL_Delay(30);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */





	MOTOR_SetVoltage(1, 0);
	MOTOR_SetVoltage(0, 0);



  uint8_t LED_Pulse = 0;

  volatile uint32_t tStart = MAIN_GetMicros();

  while (1)
  {



	  //SENSORS_xAngularVelocity = bno055_getGyroZ(&SENSORS_HI2C);



	  if(SENSORS_batteryV[0] < 6800){
	  	  		  //3.4V per cell

	  	  while(1){

	  		HAL_TIM_Base_Stop_IT(&htim12);

			  MOTOR_SetVoltage(1, 0);
			  MOTOR_SetVoltage(0, 0);

			  // start blinking red LED
			  ACTUATOR_LED(0, 0, 0);
			  HAL_Delay(100);
			  ACTUATOR_LED(50, 0, 0);
			  HAL_Delay(20);

	  	  }
	    	  }

	  vTest = MOUSE_LookForWalls();


	  MOUSE_SearchRun(500.0);

	  HAL_TIM_Base_Stop_IT(&htim12);
	  MOTOR_SetVoltage(1, 0);
	  MOTOR_SetVoltage(0, 0);

	  tStart = MAIN_GetMicros() ;
  	  while(tStart+1e7 > MAIN_GetMicros()){
		  ACTUATOR_LED(-1, -1, 100);
		  HAL_Delay(250);
		  ACTUATOR_LED(-1, -1, 0);
		  HAL_Delay(250);
		  }
  	HAL_TIM_Base_Start_IT(&htim12);

	 // MOUSE_ReturnToStart((float)500);
	 // MOUSE_SpeedRun((float)1000);

	  //printing sensors distances;
	 // printf("%i,\t %i,\t %i,\t %i,\t %i,\t %i\t \r\n",\
			  SENSORS_irVal[0],\
			  SENSORS_irVal[1],\
			  SENSORS_irVal[2],\
			  SENSORS_irVal[3],\
			  SENSORS_irVal[4],\
			  SENSORS_irVal[5]);



	 // MAIN_tmp =0;
	  /*
	   * 1) write cells -> maze
	   * 2) update path
	   * 3) cmd absolut -> realative
	   * 4) cmd -> instr
	   * 5) motionUpdate( takes from instr)
	   *
	   *
	   *
	   */
	  //TODO when to calc cell





    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
volatile uint32_t MAIN_GetMicros(){
	return (uint32_t)htim2.Instance->CNT + (uint32_t)MAIN_seconds*htim2.Init.Period;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(htim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_TIM_PeriodElapsedCallback could be implemented in the user file
   */

  if(htim->Instance == htim12.Instance){

	  //TODO  battery chcek
	  //	  volatile int sTime = MAIN_GetMicros();


	  SENSORS_Update(); // 140 us
	  MOTION_Update();
/*
	  char x[64];
	  uint8_t len = sprintf(x,"%i, %.3f, %.3f, %.3f, %.3f \n\r",(int)(MAIN_GetMicros()/1000),
			  MOTOR_currentController[1].U,
			  MOTOR_velocityController[1].U*1000,
			 MOTOR_velocityController[1].FB,
			 MOTOR_velocityController[1].W);

	  HAL_UART_Transmit(&huart3, (uint8_t*)x, len, 1000);

*/

  } else if(htim->Instance == htim2.Instance)
	  MAIN_seconds++;
  else  if(htim->Instance == SENSORS_HTIM_ENC_L.Instance || \
		   htim->Instance == SENSORS_HTIM_ENC_R.Instance)
	  SENSORS_EncoderTimCallback(htim);

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_ADC_ConvCpltCallback could be implemented in the user file
   */


  SENSORS_AdcCallback(hadc);


}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
