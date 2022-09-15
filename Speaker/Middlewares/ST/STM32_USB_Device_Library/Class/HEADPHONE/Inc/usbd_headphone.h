/**
 *  @file usbd_headphone.h
 *
 *  @date 2022年09月14日 16:57:25 星期三
 *
 *  @author Copyright (c) 2022 aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @version V1.0
 */
#ifndef USBD_HEADPHONE_H
#define USBD_HEADPHONE_H
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< need definition of uint8_t */
#include <stddef.h> /**< need definition of NULL    */
#include <stdbool.h>/**< need definition of BOOL    */
#include <stdio.h>  /**< if need printf             */
#include <stdlib.h>
#include <string.h>
#include <limits.h> /**< need variable max value    */
#include <stdalign.h> /**< need alignof    */
#include <stdarg.h> /**< need va_start    */
/** Private includes ---------------------------------------------------------*/
#include  "usbd_ioreq.h"
#include "usbd_audio.h"
#include "usbd_microphone.h"
/** Use C compiler -----------------------------------------------------------*/
#ifdef __cplusplus ///< use C compiler
extern "C" {
#endif
/** Private defines ----------------------------------------------------------*/

/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */

extern USBD_ClassTypeDef USBD_HEADPHONE;
#define USBD_HEADPHONE_CLASS &USBD_HEADPHONE
/**
  * @}
  */

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
uint8_t USBD_HEADPHONE_RegisterInterface(USBD_HandleTypeDef *pdev,
                                     USBD_AUDIO_ItfTypeDef *fops);

void USBD_HEADPHONE_Sync(USBD_HandleTypeDef *pdev, AUDIO_OffsetTypeDef offset);

#ifdef USE_USBD_COMPOSITE
uint32_t USBD_HEADPHONE_GetEpPcktSze(USBD_HandleTypeDef *pdev, uint8_t If, uint8_t Ep);
#endif /* USE_USBD_COMPOSITE */

/**
  * @}
  */

#ifdef __cplusplus ///<end extern c
}
#endif
#endif
/******************************** End of file *********************************/
