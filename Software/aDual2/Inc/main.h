/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
volatile uint32_t MAIN_GetMicros();
volatile uint32_t MAIN_seconds;

volatile uint32_t MAIN_tPeriod, MAIN_tStart;
// one hour to ovreflow;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADC_IR_LD_Pin GPIO_PIN_0
#define ADC_IR_LD_GPIO_Port GPIOC
#define ADC_IR_LS_Pin GPIO_PIN_1
#define ADC_IR_LS_GPIO_Port GPIOC
#define ADC_IR_LF_Pin GPIO_PIN_2
#define ADC_IR_LF_GPIO_Port GPIOC
#define ADC_IR_RS_Pin GPIO_PIN_3
#define ADC_IR_RS_GPIO_Port GPIOC
#define TIM5_CH1_ENC1_A_Pin GPIO_PIN_0
#define TIM5_CH1_ENC1_A_GPIO_Port GPIOA
#define TIM5_CH2_ENC1_B_Pin GPIO_PIN_1
#define TIM5_CH2_ENC1_B_GPIO_Port GPIOA
#define M1_EN_Pin GPIO_PIN_2
#define M1_EN_GPIO_Port GPIOA
#define M1_DIR_Pin GPIO_PIN_3
#define M1_DIR_GPIO_Port GPIOA
#define M1_ISENSE_Pin GPIO_PIN_4
#define M1_ISENSE_GPIO_Port GPIOA
#define ADC_BAT1_Pin GPIO_PIN_5
#define ADC_BAT1_GPIO_Port GPIOA
#define ADC_BAT2_Pin GPIO_PIN_6
#define ADC_BAT2_GPIO_Port GPIOA
#define M2_EN_Pin GPIO_PIN_7
#define M2_EN_GPIO_Port GPIOA
#define M2_DIR_Pin GPIO_PIN_4
#define M2_DIR_GPIO_Port GPIOC
#define M2_ISENSE_Pin GPIO_PIN_5
#define M2_ISENSE_GPIO_Port GPIOC
#define ADC_IR_RF_Pin GPIO_PIN_0
#define ADC_IR_RF_GPIO_Port GPIOB
#define ADC_IR_RD_Pin GPIO_PIN_1
#define ADC_IR_RD_GPIO_Port GPIOB
#define SDIO_SW_Pin GPIO_PIN_15
#define SDIO_SW_GPIO_Port GPIOB
#define TIM3_CH1_ENC2_A_Pin GPIO_PIN_6
#define TIM3_CH1_ENC2_A_GPIO_Port GPIOC
#define TIM3_CH2_ENC2_B_Pin GPIO_PIN_7
#define TIM3_CH2_ENC2_B_GPIO_Port GPIOC
#define TIM1_RGB_R_Pin GPIO_PIN_8
#define TIM1_RGB_R_GPIO_Port GPIOA
#define TIM1_RGB_G_Pin GPIO_PIN_9
#define TIM1_RGB_G_GPIO_Port GPIOA
#define TIM1_RGB_B_Pin GPIO_PIN_10
#define TIM1_RGB_B_GPIO_Port GPIOA
#define GPIO_IR_LED2_Pin GPIO_PIN_11
#define GPIO_IR_LED2_GPIO_Port GPIOA
#define GPIO_IR_LED1_Pin GPIO_PIN_12
#define GPIO_IR_LED1_GPIO_Port GPIOA
#define TIM4_BUZZER_Pin GPIO_PIN_7
#define TIM4_BUZZER_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

#define WHEEL_PITCH 72 //mm


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
