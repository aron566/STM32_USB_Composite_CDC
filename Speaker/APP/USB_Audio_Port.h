/**
 *  @file USB_Audio_Port.h
 *
 *  @date 2021-06-01
 *
 *  @author aron566
 *
 *  @brief USB UAC类实现，Microphone 端点0x81 + Speaker 端点 0x02
 *
 *  @version V1.0
 */
#ifndef USB_AUDIO_PORT_H
#define USB_AUDIO_PORT_H
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< need definition of uint8_t */
#include <stddef.h> /**< need definition of NULL    */
#include <stdbool.h>/**< need definition of BOOL    */
#include <stdio.h>  /**< if need printf             */
#include <stdlib.h>
#include <string.h>
#include <limits.h> /**< need variable max value    */
/** Exported macros-----------------------------------------------------------*/
#define AUDIO_PORT_CHANNEL_NUMS               2U      /**< MIC音频通道数 */
#define MONO_CHANNEL_SEL                      2U      /**< 单通道时，0使用L声道 1使用R声道 2配置MONO */
#define AUDIO_PORT_USBD_AUDIO_FREQ            16000  /**< 设置音频采样率 */

#define AUDIO_PORT_IF_NUM                    3       /**< 接口数目 2 or 3 */

/* @ref:https://www.usbzh.com/article/detail-253.html */

/* Microphone */
#define AUDIO_PORT_USE_MICROPHONE             1       /**< 是否配置Microphone */

#define AUDIO_PORT_INPUT_TERMINAL_ID_1        0x01
#define AUDIO_PORT_OUTPUT_TERMINAL_ID_3       0x03
#define AUDIO_PORT_INPUT_FEATURE_ID_2         0x02

/* 音频类终端类型定义 */
#define AUDIO_PORT_MICRO_PHONE_TERMINAL_L     0x01U
#define AUDIO_PORT_MICRO_PHONE_TERMINAL_H     0x02U


/* Speaker */
#define AUDIO_PORT_USE_SPEAKER                1       /**< 是否配置Speaker */

#define AUDIO_PORT_INPUT_TERMINAL_ID_4        0x04
#define AUDIO_PORT_INPUT_FEATURE_ID_5         0x05
#define AUDIO_PORT_OUTPUT_TERMINAL_ID_6       0x06

/* 音频类终端类型定义 */
#define AUDIO_PORT_SPEAKE_TERMINAL_TYPE_L     0x01U
#define AUDIO_PORT_SPEAKE_TERMINAL_TYPE_H     0x03U

#define AUDIO_PORT_STREAM_TERMINAL_TYPE_L     0x01U
#define AUDIO_PORT_STREAM_TERMINAL_TYPE_H     0x01U


/* 立体声配置 */
#if AUDIO_PORT_CHANNEL_NUMS == 2
  /* 使用L+R声道 */
  #define AUDIO_PORT_CHANNEL_CONFIG_L  0x03U
  #define AUDIO_PORT_CHANNEL_CONFIG_H  0x00U
/* 单声道配置 */
#elif AUDIO_PORT_CHANNEL_NUMS == 1
  #if MONO_CHANNEL_SEL == 0
    /* 使用L声道 */
    #define AUDIO_PORT_CHANNEL_CONFIG_L  0x01U
    #define AUDIO_PORT_CHANNEL_CONFIG_H  0x00U
  #elif MONO_CHANNEL_SEL == 1
    /* 使用R声道 */
    #define AUDIO_PORT_CHANNEL_CONFIG_L  0x02U
    #define AUDIO_PORT_CHANNEL_CONFIG_H  0x00U
  #elif MONO_CHANNEL_SEL == 2
    /* 使用默认MONO声道 */
    #define AUDIO_PORT_CHANNEL_CONFIG_L  0x00U
    #define AUDIO_PORT_CHANNEL_CONFIG_H  0x00U
  #endif
#endif

/* 轮询时间间隔 */
#define AUDIO_PORT_FS_BINTERVAL           1U     /**< 1ms一次轮询 */
/* 音频传输大小设置 */
#define AUDIO_PORT_PACKET_SZE(frq)       (uint8_t)(((frq * 2U * 2U)/(1000U/AUDIO_PORT_FS_BINTERVAL)) & 0xFFU), \
                                         (uint8_t)((((frq * 2U * 2U)/(1000U/AUDIO_PORT_FS_BINTERVAL)) >> 8) & 0xFFU)

#define AUDIO_PORT_OUT_SIZE               ((AUDIO_PORT_USBD_AUDIO_FREQ * 2U * 2U)/(1000U/AUDIO_PORT_FS_BINTERVAL))   /**< 音频发送大小Byte */
#define AUDIO_PORT_BUF_SIZE               AUDIO_PORT_OUT_SIZE*4   /**< 音频缓冲区大小 大于3的倍数 */
/** Private includes ---------------------------------------------------------*/

/** Use C compiler -----------------------------------------------------------*/
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Private defines ----------------------------------------------------------*/

/** Exported typedefines -----------------------------------------------------*/

/** Exported constants -------------------------------------------------------*/


/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

/**
 * @brief 向USB缓冲区数据加入数据
 *
 * @param Left_Audio 左通道音频
 * @param Right_Audio 右通道音频
 */
void USB_Audio_Port_Put_Data(const int16_t *Left_Audio, const int16_t *Right_Audio);

/**
 * @brief 初始化音频输出端点
 *
 * @param xpdev
 * @param cfgidx
 * @return uint8_t
 */
uint8_t USB_Audio_Port_EP_IN_Init(void *xpdev, uint8_t cfgidx);

/**
 * @brief 初始化音频输入端点
 *
 * @param xpdev USB句柄
 * @param cfgidx
 * @return uint8_t
 */
uint8_t USB_Audio_Port_EP_OUT_Init(void *xpdev, uint8_t cfgidx);
/*反初始化音频输出端点*/

/**
 * @brief
 *
 * @param xpdev USB句柄
 * @param cfgidx
 * @return uint8_t
 */
uint8_t USB_Audio_Port_EP_IN_DeInit(void *xpdev, uint8_t cfgidx);

/**
 * @brief 反初始化音频输入端点
 *
 * @param xpdev USB句柄
 * @param cfgidx
 * @return uint8_t
 */
uint8_t USB_Audio_Port_EP_OUT_DeInit(void *xpdev, uint8_t cfgidx);

/**
 * @brief USB缓冲区数据发送至HOST
 *
 * @param xpdev USB句柄
 * @param epnum 端点号
 * @return uint8_t 返回状态
 */
uint8_t USB_Audio_Port_DataIn(void *xpdev, uint8_t epnum);

/**
 * @brief USB缓冲区数据接收来自HOST
 *
 * @param xpdev USB句柄
 * @param epnum 端点号
 * @return uint8_t 返回状态
 */
uint8_t USB_Audio_Port_DataOut(void *xpdev, uint8_t epnum);

/**
 * @brief 处理PC音频数据
 *
 * @param pbuf 数据
 * @param size 数据长度字节
 * @param cmd 命令
 * @return int8_t 返回状态
 */
int8_t USB_Audio_Port_Process_PC_Audio(uint8_t *pbuf, uint32_t size, uint8_t cmd);

/**
 * @brief 获取PC音频数据
 *
 * @param Left_Buf 左通道音频存储区
 * @param Right_Buf 右通道音频存储区
 * @return uint32_t 获取到的数据样点数
 */
uint32_t USB_Audio_Port_Get_PC_Audio_Data(int16_t *Left_Buf, int16_t *Right_Buf);

/**
 * @brief 音频传输完成同步，硬件DMA 发送（1/2*AUDIO_PORT_OUT_SIZE）中断调用
 *
 */
void USB_Audio_Port_TransferComplete_CallBack_FS(void);

/**
 * @brief 音频半传输完成同步，硬件DMA 发送 AUDIO_PORT_OUT_SIZE 中断调用
 *
 */
void USB_Audio_Port_HalfTransfer_CallBack_FS(void);

/**
  * @brief  USBD_AUDIO_SOF
  *         handle SOF event
  * @param  xpdev: device instance
  * @retval status
  */
void USB_Audio_Port_AUDIO_Sync(void *xpdev, int offset);

#ifdef __cplusplus ///<end extern c
}
#endif
#endif
/******************************** End of file *********************************/
