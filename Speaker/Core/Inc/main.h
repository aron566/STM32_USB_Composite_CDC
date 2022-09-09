/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32h7xx_hal.h"

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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define HS_DET_Pin GPIO_PIN_0
#define HS_DET_GPIO_Port GPIOC
#define HS_DET_EXTI_IRQn EXTI0_IRQn
#define USB_5V_DET_Pin GPIO_PIN_2
#define USB_5V_DET_GPIO_Port GPIOC
#define CHARGE_STATE_Pin GPIO_PIN_1
#define CHARGE_STATE_GPIO_Port GPIOA
#define DEBUG_USART2_TX_Pin GPIO_PIN_2
#define DEBUG_USART2_TX_GPIO_Port GPIOA
#define DEBUG_USART2_RX_Pin GPIO_PIN_3
#define DEBUG_USART2_RX_GPIO_Port GPIOA
#define LCD_SPI_CS_Pin GPIO_PIN_4
#define LCD_SPI_CS_GPIO_Port GPIOA
#define LCD_SPI1_SCK_Pin GPIO_PIN_5
#define LCD_SPI1_SCK_GPIO_Port GPIOA
#define LCD_SPI1_MISO_Pin GPIO_PIN_6
#define LCD_SPI1_MISO_GPIO_Port GPIOA
#define LCD_SPI1_MOSI_Pin GPIO_PIN_7
#define LCD_SPI1_MOSI_GPIO_Port GPIOA
#define BAT_ADC_Pin GPIO_PIN_0
#define BAT_ADC_GPIO_Port GPIOB
#define LCD_RESET_Pin GPIO_PIN_1
#define LCD_RESET_GPIO_Port GPIOB
#define TP_RESET_Pin GPIO_PIN_2
#define TP_RESET_GPIO_Port GPIOB
#define LCD_ID_Pin GPIO_PIN_7
#define LCD_ID_GPIO_Port GPIOE
#define MAX17048_ALARM_Pin GPIO_PIN_8
#define MAX17048_ALARM_GPIO_Port GPIOE
#define LCD_DCX_Pin GPIO_PIN_10
#define LCD_DCX_GPIO_Port GPIOE
#define TP_Pin GPIO_PIN_11
#define TP_GPIO_Port GPIOE
#define I2C2_MAX17048_SCL_Pin GPIO_PIN_10
#define I2C2_MAX17048_SCL_GPIO_Port GPIOB
#define I2C2_MAX17048_SDA_Pin GPIO_PIN_11
#define I2C2_MAX17048_SDA_GPIO_Port GPIOB
#define I2C4_MPU6050_SCL_Pin GPIO_PIN_12
#define I2C4_MPU6050_SCL_GPIO_Port GPIOD
#define I2C4_MPU6050_SDA_Pin GPIO_PIN_13
#define I2C4_MPU6050_SDA_GPIO_Port GPIOD
#define MPU6050_FSYNC_Pin GPIO_PIN_14
#define MPU6050_FSYNC_GPIO_Port GPIOD
#define MPU6050_INT_Pin GPIO_PIN_15
#define MPU6050_INT_GPIO_Port GPIOD
#define I2C3_TP_SDA_Pin GPIO_PIN_9
#define I2C3_TP_SDA_GPIO_Port GPIOC
#define I2C3_TP_SCL_Pin GPIO_PIN_8
#define I2C3_TP_SCL_GPIO_Port GPIOA
#define USB_PULL_IO_Pin GPIO_PIN_10
#define USB_PULL_IO_GPIO_Port GPIOA
#define SPI6_CS_FLASH0_Pin GPIO_PIN_15
#define SPI6_CS_FLASH0_GPIO_Port GPIOA
#define VCC_LCD_3_3V_EN_Pin GPIO_PIN_10
#define VCC_LCD_3_3V_EN_GPIO_Port GPIOC
#define VCC_BACK_LIGHT_EN_Pin GPIO_PIN_11
#define VCC_BACK_LIGHT_EN_GPIO_Port GPIOC
#define VCC_DAC_3_3V_EN_Pin GPIO_PIN_12
#define VCC_DAC_3_3V_EN_GPIO_Port GPIOC
#define POWER_KEY_Pin GPIO_PIN_0
#define POWER_KEY_GPIO_Port GPIOD
#define BT_CONNECT_STAT_Pin GPIO_PIN_1
#define BT_CONNECT_STAT_GPIO_Port GPIOD
#define VCC_BT8829C_EN_Pin GPIO_PIN_2
#define VCC_BT8829C_EN_GPIO_Port GPIOD
#define VCC_LED_2_8V_EN_Pin GPIO_PIN_4
#define VCC_LED_2_8V_EN_GPIO_Port GPIOD
#define SPI6_CS_FLASH1_Pin GPIO_PIN_7
#define SPI6_CS_FLASH1_GPIO_Port GPIOD
#define SPI6_SCK_FLASH_Pin GPIO_PIN_3
#define SPI6_SCK_FLASH_GPIO_Port GPIOB
#define SPI6_MISO_FLASH_Pin GPIO_PIN_4
#define SPI6_MISO_FLASH_GPIO_Port GPIOB
#define SPI6_MOSI_FLASH_Pin GPIO_PIN_5
#define SPI6_MOSI_FLASH_GPIO_Port GPIOB
#define I2C1_WM8978_SCL_Pin GPIO_PIN_6
#define I2C1_WM8978_SCL_GPIO_Port GPIOB
#define I2C1_WM8978_SDA_Pin GPIO_PIN_7
#define I2C1_WM8978_SDA_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
