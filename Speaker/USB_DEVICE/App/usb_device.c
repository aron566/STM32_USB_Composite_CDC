/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usb_device.c
  * @version        : v1.0_Cube
  * @brief          : This file implements the USB Device
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

/* Includes ------------------------------------------------------------------*/

#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_audio.h"
#include "usbd_audio_if.h"

/* USER CODE BEGIN Includes */
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "usbd_microphone.h"
#include "usbd_headphone.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceFS;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
void MX_USB_DEVICE_Init(void)
{
  /* USER CODE BEGIN USB_DEVICE_Init_PreTreatment */
#ifdef USE_USBD_COMPOSITE
  /* 初始化USB */
  if(USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
  {
    Error_Handler();
  }

  /* 注册Headphone接口 */
  if(USBD_HEADPHONE_RegisterInterface(&hUsbDeviceFS, &USBD_AUDIO_fops_FS) != USBD_OK)
  {
    Error_Handler();
  }
  /* 注册组合设备Headphone类 */
  static uint8_t Headphone_EP_Addr[] = {MICROPHONE_IN_EP, SPEAKER_OUT_EP};
  if(USBD_RegisterClassComposite(&hUsbDeviceFS, &USBD_HEADPHONE,
                                                CLASS_TYPE_HEADPHONE, Headphone_EP_Addr) != USBD_OK)
  {
    Error_Handler();
  }

  /* 注册Microphone接口 */
  // if(USBD_MICROPHONE_RegisterInterface(&hUsbDeviceFS, &USBD_AUDIO_fops_FS) != USBD_OK)
  // {
  //   Error_Handler();
  // }
  // /* 注册组合设备Microphone类 */
  // static uint8_t Microphone_EP_Addr[] = {MICROPHONE_IN_EP};
  // if(USBD_RegisterClassComposite(&hUsbDeviceFS, &USBD_MICROPHONE,
  //                                               CLASS_TYPE_MICROPHONE, Microphone_EP_Addr) != USBD_OK)
  // {
  //   Error_Handler();
  // }

  /* 注册Speaker接口 */
  // if(USBD_AUDIO_RegisterInterface(&hUsbDeviceFS, &USBD_AUDIO_fops_FS) != USBD_OK)
  // {
  //   Error_Handler();
  // }
  // /* 注册组合设备Speaker类 */
  // static uint8_t Speaker_EP_Addr[] = {SPEAKER_OUT_EP};
  // if(USBD_RegisterClassComposite(&hUsbDeviceFS, &USBD_AUDIO,
  //                                               CLASS_TYPE_AUDIO, Speaker_EP_Addr) != USBD_OK)
  // {
  //   Error_Handler();
  // }

  /* 注册CDC接口 */
  // if(USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) != USBD_OK)
  // {
  //   Error_Handler();
  // }
  // /* 注册组合设备CDC类 */
  // static uint8_t CDC_EP_Addr[] = {CDC_IN_EP, CDC_OUT_EP, CDC_CMD_EP};
  // if(USBD_RegisterClassComposite(&hUsbDeviceFS, &USBD_CDC,
  //                                               CLASS_TYPE_CDC, CDC_EP_Addr) != USBD_OK)
  // {
  //   Error_Handler();
  // }

  /* 注册组合设备多路CDC */
  if(USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) != USBD_OK)
  {
    Error_Handler();
  }
  /* 注册组合设备多路CDC类 */
  static uint8_t MULTI_CDC_EP_Addr[] = {CDC_IN_EP, CDC_OUT_EP, CDC_CMD_EP, CDC2_IN_EP, CDC2_OUT_EP, CDC2_CMD_EP, CDC3_IN_EP, CDC3_OUT_EP, CDC3_CMD_EP};
  if(USBD_RegisterClassComposite(&hUsbDeviceFS, &USBD_CDC,
                                                CLASS_TYPE_MULTI_CDC, MULTI_CDC_EP_Addr) != USBD_OK)
  {
    Error_Handler();
  }

  if(USBD_Start(&hUsbDeviceFS) != USBD_OK)
  {
    Error_Handler();
  }

  HAL_PWREx_EnableUSBVoltageDetector();
#else
  /* USER CODE END USB_DEVICE_Init_PreTreatment */

  /* Init Device Library, add supported class and start the library. */
  if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
  {
    Error_Handler();
  }
  // if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK)
  // {
  //   Error_Handler();
  // }
  // if (USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) != USBD_OK)
  // {
  //   Error_Handler();
  // }
  if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_HEADPHONE) != USBD_OK)
  {
    Error_Handler();
  }
  /* 注册Headphone接口 */
  if(USBD_HEADPHONE_RegisterInterface(&hUsbDeviceFS, &USBD_AUDIO_fops_FS) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_Start(&hUsbDeviceFS) != USBD_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN USB_DEVICE_Init_PostTreatment */
  HAL_PWREx_EnableUSBVoltageDetector();
#endif
  /* USER CODE END USB_DEVICE_Init_PostTreatment */
}

/**
  * @}
  */

/**
  * @}
  */
