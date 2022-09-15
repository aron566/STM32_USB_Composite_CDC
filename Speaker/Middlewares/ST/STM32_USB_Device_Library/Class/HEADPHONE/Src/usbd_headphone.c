/**
 *  @file usbd_headphone.c
 *
 *  @date 2022年09月14日 16:57:55 星期三
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2022 aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @details None.
 *
 *  @version V1.0
 */
/** Includes -----------------------------------------------------------------*/
#include "usbd_ctlreq.h"
/* Private includes ----------------------------------------------------------*/
#include "usbd_headphone.h"
#include "USB_Audio_Port.h"
/** Use C compiler -----------------------------------------------------------*/
#ifdef __cplusplus ///< use C compiler
extern "C" {
#endif
/** Private macros -----------------------------------------------------------*/
#if AUDIO_PORT_IF_NUM == 2
  #define USB_HEADPHONE_CONFIG_DESC_SIZ 109U
#else
  #define USB_HEADPHONE_CONFIG_DESC_SIZ 192U
#endif
/** @defgroup USBD_AUDIO_Private_Macros
  * @{
  */
#define AUDIO_SAMPLE_FREQ(frq) \
  (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))

#define AUDIO_PACKET_SZE(frq) \
  (uint8_t)(((frq * 2U * 2U) / 1000U) & 0xFFU), (uint8_t)((((frq * 2U * 2U) / 1000U) >> 8) & 0xFFU)

#ifdef USE_USBD_COMPOSITE
#define AUDIO_PACKET_SZE_WORD(frq)     (uint32_t)((((frq) * 2U * 2U)/1000U))
#endif /* USE_USBD_COMPOSITE  */
/**
  * @}
  */
/** Private typedef ----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/

#ifndef USE_USBD_COMPOSITE
/* USB AUDIO device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_AUDIO_CfgDesc[USB_HEADPHONE_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /* Configuration 1 */
  0x09,                                 /* bLength */
  USB_DESC_TYPE_CONFIGURATION,          /* bDescriptorType 配置描述符*/
  LOBYTE(USB_HEADPHONE_CONFIG_DESC_SIZ),/* wTotalLength  USB_HEADPHONE_CONFIG_DESC_SIZ bytes*/
  HIBYTE(USB_HEADPHONE_CONFIG_DESC_SIZ),
  AUDIO_PORT_IF_NUM,                   /* bNumInterfaces  2 or 3个接口 AC与AS（1Microphone or 2Speaker） */
  0x01,                                 /* bConfigurationValue 配置需要参数的值 */
  0x00,                                 /* iConfiguration 配置描述符的字符串说明索引号，为0则不使用*/
#if (USBD_SELF_POWERED == 1U)
  0xC0,                                 /* bmAttributes: Bus Powered according to user configuration */
#else
  0x80,                                 /* bmAttributes: Bus Powered according to user configuration */
#endif
  USBD_MAX_POWER,                       /* bMaxPower = 100 mA */
  /* 09 byte*/

  /* USB Microphone Standard interface descriptor */
  /* USB Audio Control 标准控制接口描述符（音频控制类接口描述符）*/
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  USB_DESC_TYPE_INTERFACE,              /* bDescriptorType 0x04接口描述符 */
  0x00,                                 /* bInterfaceNumber 本接口索引 index 0 */
  0x00,                                 /* bAlternateSetting 本备用设置索引*/
  0x00,                                 /* bNumEndpoints Audio Control Interface没有使用专用的端点，而是使用了默认的端点0来操作。并且该接口，没有中断端点 */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass 0x01音频类 */
  AUDIO_SUBCLASS_AUDIOCONTROL,          /* bInterfaceSubClass 0x01音频控制类 */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/

  /* USB Audio Class-specific AC Interface Descriptor */
  /* 音频相关的描述符  控制接口规范，音频流控制接口数量，需要有一个header作为一些统一的描述 */
#if AUDIO_PORT_IF_NUM == 2
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
#elif AUDIO_PORT_IF_NUM == 3
  AUDIO_INTERFACE_DESC_SIZE + 1,        /* bLength */
#endif
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType audio interface descriptor 0x24音频类接口描述符 */
  AUDIO_CONTROL_HEADER,                 /* bDescriptorSubtype 音频控制头子类 0x01 audio control header */
  0x00,          /* 1.00 */             /* bcdADC 音频类 规范1.0 0x0100*/
  0x01,
#if AUDIO_PORT_IF_NUM == 2
  0x27,                                 /* wTotalLength = 39 (9自身+12终端+9特征+9终端) Audio Class相关描述总大小Bytes 0x0027 */
#elif AUDIO_PORT_IF_NUM == 3
  10 + (12 + 9 + 9) * 2,              /* wTotalLength = 40 (10自身+12终端+9特征+9终端) Audio Class相关描述总大小Bytes 0x0028 */
#endif
  0x00,
#if AUDIO_PORT_IF_NUM == 2
  0x01,                                 /* bInCollection 该音频控制接口所拥有的音频流接口总数，也就是2。 */
  0x01,                                 /* baInterfaceNr 所拥有的第一个音频流接口号 */
  /* 9 byte*/
#elif AUDIO_PORT_IF_NUM == 3
  0x02,                                 /* bInCollection 该音频控制接口所拥有的音频流接口总数，也就是2。 */
  0x01,                                 /* baInterfaceNr 所拥有的第一个音频流接口号 */
  0x02,                                 /* baInterfaceNr 所拥有的第二个音频流接口号 */
  /* 10 byte*/
#endif

#if AUDIO_PORT_USE_MICROPHONE
  //=============Microphone 终端1 IN -> 特征2 -> 终端3 OUT 流=============//
  /* USB Microphone Input Terminal Descriptor */
  /* Terminal 1 输入 来自终端类型0x0201（麦克风）数据 通道2个(L+R) */
  AUDIO_INPUT_TERMINAL_DESC_SIZE,       /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType 0x24 音频接口描述符 */
  AUDIO_CONTROL_INPUT_TERMINAL,         /* bDescriptorSubtype 子输入终端描述符子类 0x02 输入终端子类型 */
  0x01,                                 /* bTerminalID 本Termianl ID */
  AUDIO_PORT_MICRO_PHONE_TERMINAL_L,    /* wTerminalType  类型麦克风 */
  AUDIO_PORT_MICRO_PHONE_TERMINAL_H,
  0x00,                                 /* bAssocTerminal 相关的input terminal ID，0代表此值未使用 */
  AUDIO_PORT_CHANNEL_NUMS,              /* bNrChannels 通道数 */
  AUDIO_PORT_CHANNEL_CONFIG_L,          /* wChannelConfig 0x0003  L+R声道 */
  AUDIO_PORT_CHANNEL_CONFIG_H,
  0x00,                                 /* iChannelNames 通道名字符串描述（没有）*/
  0x00,                                 /* iTerminal 端点字符串描述（没有） */
  /* 12 byte*/

  /* USB Microphone Audio Feature Unit Descriptor 特征描述 */
  /* Terminal 2 ，Source Terminal 1 麦克风，ControlSize 1 ，Mute */
  0x09,                                 /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType 0x24 音频接口描述符 */
  AUDIO_CONTROL_FEATURE_UNIT,           /* bDescriptorSubtype 0x06 子类型:特征描述 */
  AUDIO_PORT_INPUT_FEATURE_ID_2,        /* bUnitID 特征ID 2 */
  AUDIO_PORT_INPUT_TERMINAL_ID_1,       /* bSourceID Terminal 1 麦克风 */
  0x01,                                 /* bControlSize */
  AUDIO_CONTROL_MUTE,                   /* bmaControls(0) */
  0,                                    /* bmaControls(1) */
  0x00,                                 /* iTerminal */
  /* 09 byte*/

  /* USB Microphone Output Terminal Descriptor */
  /* Terminal 3，音频流，Source Terminal 2（特征ID 2） */
  0x09,                                 /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType 0x24 音频接口描述符 */
  AUDIO_CONTROL_OUTPUT_TERMINAL,        /* bDescriptorSubtype 0x03 子类型:输出终端 */
  AUDIO_PORT_OUTPUT_TERMINAL_ID_3,      /* bTerminalID */
  AUDIO_PORT_STREAM_TERMINAL_TYPE_L,    /* wTerminalType  音频流0x0101*/
  AUDIO_PORT_STREAM_TERMINAL_TYPE_H,
  0x00,                                 /* bAssocTerminal */
  AUDIO_PORT_INPUT_FEATURE_ID_2,        /* bSourceID */
  0x00,                                 /* iTerminal */
  /* 09 byte*/
#endif

#if AUDIO_PORT_USE_SPEAKER
  //=============音频流 终端4 IN -> 特征5 -> 终端6 OUT Speaker=============//
  /* USB Speaker Input Terminal Descriptor */
  /* Terminal 4 输入 来自终端类型0x0101（音频流）数据 通道2个(L+R) */
  AUDIO_INPUT_TERMINAL_DESC_SIZE,       /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType 0x24 音频接口描述符 */
  AUDIO_CONTROL_INPUT_TERMINAL,         /* bDescriptorSubtype 子类接口描述：0x02输入终端子类型*/
  AUDIO_PORT_INPUT_TERMINAL_ID_4,       /* bTerminalID 本Termianl ID*/
  AUDIO_PORT_STREAM_TERMINAL_TYPE_L,    /* wTerminalType  类型音频流*/
  AUDIO_PORT_STREAM_TERMINAL_TYPE_H,
  0x00,                                 /* bAssocTerminal 相关的input terminal ID，0代表此值未使用 */
  AUDIO_PORT_CHANNEL_NUMS,              /* bNrChannels 通道数 */
  AUDIO_PORT_CHANNEL_CONFIG_L,          /* wChannelConfig 0x0003  L+R声道 */
  AUDIO_PORT_CHANNEL_CONFIG_H,
  0x00,                                 /* iChannelNames 通道名字符串描述（没有）*/
  0x00,                                 /* iTerminal 端点字符串描述（没有） */
  /* 12 byte*/

  /* USB Speaker Audio Feature Unit Descriptor 特征描述 */
  /* Terminal 5 ，Source Terminal 4 音频流，ControlSize 1 ，Mute */
  0x09,                                 /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType 0x24 音频接口描述符 */
  AUDIO_CONTROL_FEATURE_UNIT,           /* bDescriptorSubtype 0x06 子类型:特征描述 */
  AUDIO_PORT_INPUT_FEATURE_ID_5,        /* bUnitID 特征ID 5 */
  AUDIO_PORT_INPUT_TERMINAL_ID_4,       /* bSourceID Terminal 4 音频流 */
  0x01,                                 /* bControlSize */
  AUDIO_CONTROL_MUTE,                   /* bmaControls(0) */
  0,                                    /* bmaControls(1) */
  0x00,                                 /* iTerminal */
  /* 09 byte*/

  /*USB Speaker Output Terminal Descriptor */
  /* Terminal 6，输出Speaker，Source Terminal 5（特征ID 5） */
  0x09,                                 /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType 0x24 音频接口描述符 */
  AUDIO_CONTROL_OUTPUT_TERMINAL,        /* bDescriptorSubtype 0x03 子类型:输出终端 */
  AUDIO_PORT_OUTPUT_TERMINAL_ID_6,      /* bTerminalID */
  AUDIO_PORT_SPEAKE_TERMINAL_TYPE_L,    /* wTerminalType  Speaker0x0301 */
  AUDIO_PORT_SPEAKE_TERMINAL_TYPE_H,
  0x00,                                 /* bAssocTerminal */
  AUDIO_PORT_INPUT_FEATURE_ID_5,        /* bSourceID */
  0x00,                                 /* iTerminal */
  /* 09 byte*/
#endif

#if AUDIO_PORT_USE_MICROPHONE
  //==================Microphone 接口1 - 端点=================//
  /* USB Microphone Standard AS Interface Descriptor - Audio Streaming Zero Bandwidth */
  /* Interface 1, Alternate Setting 0                                             */
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  USB_DESC_TYPE_INTERFACE,              /* bDescriptorType 0x04 接口描述符 */
  0x01,                                 /* bInterfaceNumber 本接口索引 index 1 */
  0x00,                                 /* bAlternateSetting 0时为默认的无音频输出接口即无端点描述符，从bAlternateSetting=1开始有数据传输 */
  0x00,                                 /* bNumEndpoints 端点数0 */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass 0x01 音频接口类 */
  AUDIO_SUBCLASS_AUDIOSTREAMING,        /* bInterfaceSubClass 0x02 子类音频流 */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/

  /* USB Microphone Standard AS Interface Descriptor - Audio Streaming Operational */
  /* Interface 1, Alternate Setting 1                                           */
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  USB_DESC_TYPE_INTERFACE,              /* bDescriptorType 0x04 接口描述符 */
  0x01,                                 /* bInterfaceNumber 本接口索引 index 1 */
  0x01,                                 /* bAlternateSetting 0时为默认的无音频输出接口即无端点描述符，从bAlternateSetting=1开始有数据传输 */
  0x01,                                 /* bNumEndpoints 端点数1 */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass 0x01 音频接口类 */
  AUDIO_SUBCLASS_AUDIOSTREAMING,        /* bInterfaceSubClass 0x02 子类音频流 */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/

  /* USB Microphone Audio Streaming Interface Descriptor */
  AUDIO_STREAMING_INTERFACE_DESC_SIZE,  /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType 0x24 音频接口描述符 */
  AUDIO_STREAMING_GENERAL,              /* bDescriptorSubtype 0x01 一般音频流 */
  AUDIO_PORT_OUTPUT_TERMINAL_ID_3,      /* bTerminalLink 连接 输出终端 3 */
  0x01,                                 /* bDelay 1ms */
  0x01,                                 /* wFormatTag AUDIO_FORMAT_PCM  0x0001 */
  0x00,
  /* 07 byte*/

  /* USB Microphone Audio Type III Format Interface Descriptor */
  0x0B,                                 /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType 0x24 音频接口描述符 */
  AUDIO_STREAMING_FORMAT_TYPE,          /* bDescriptorSubtype 0x02 音频格式描述符 */
  AUDIO_FORMAT_TYPE_I,                  /* bFormatType */
  AUDIO_PORT_CHANNEL_NUMS,              /* bNrChannels 2个通道 */
  0x02,                                 /* bSubFrameSize :  2 Bytes per frame (16bits) */
  16,                                   /* bBitResolution (16-bits per sample) */
  0x01,                                 /* bSamFreqType only one frequency supported  支持几中采样率，在下面每3Bytes描述支持的采样率 */
  AUDIO_SAMPLE_FREQ(AUDIO_PORT_USBD_AUDIO_FREQ),   /* Audio sampling frequency coded on 3 bytes */
  /* 11 byte*/

  /* Endpoint 1 - Standard Descriptor */
  AUDIO_STANDARD_ENDPOINT_DESC_SIZE,    /* bLength */
  USB_DESC_TYPE_ENDPOINT,               /* bDescriptorType 0x05 端点描述符 */
  MICROPHONE_IN_EP,              /* bEndpointAddress 0x81 输入端点 D7：方向位，1表示这是一个数据输入端点，0表示这是一个数据输出端点																																			D3~D0：端点号 */
  USBD_EP_TYPE_ISOC,                    /* bmAttributes */
  AUDIO_PORT_PACKET_SZE(AUDIO_PORT_USBD_AUDIO_FREQ),/* wMaxPacketSize in Bytes (Freq(Samples)*2(Stereo)*2(HalfWord)) */
  AUDIO_PORT_FS_BINTERVAL,              /* bInterval */
  0x00,                                 /* bRefresh */
  0x00,                                 /* bSynchAddress */
  /* 09 byte*/

  /* Endpoint - Audio Streaming Descriptor*/
  AUDIO_STREAMING_ENDPOINT_DESC_SIZE,   /* bLength */
  AUDIO_ENDPOINT_DESCRIPTOR_TYPE,       /* bDescriptorType 0x25 音频端点描述符 */
  AUDIO_ENDPOINT_GENERAL,               /* bDescriptor 0x01 一般端点描述 */
  0x80,                                 /* bmAttributes */
  0x01,                                 /* bLockDelayUnits */
  0x00,                                 /* wLockDelay */
  0x00,
  /* 07 byte*/
#endif

#if AUDIO_PORT_USE_SPEAKER
  //==================Speaker 接口2 - 端点=================//
  /* USB Speaker Standard AS Interface Descriptor - Audio Streaming Zero Bandwidth */
  /* Interface 2, Alternate Setting 0                                             */
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  USB_DESC_TYPE_INTERFACE,              /* bDescriptorType 0x04 接口描述符 */
#if AUDIO_PORT_IF_NUM == 2
  0x01,                                 /* bInterfaceNumber 本接口索引 index 1 */
#elif AUDIO_PORT_IF_NUM == 3
  0x02,                                 /* bInterfaceNumber 本接口索引 index 2 */
#endif
  0x00,                                 /* bAlternateSetting 0时为默认的无音频输出接口即无端点描述符，从bAlternateSetting=1开始有数据传输 */
  0x00,                                 /* bNumEndpoints 端点数0 */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass 0x01 音频接口类 */
  AUDIO_SUBCLASS_AUDIOSTREAMING,        /* bInterfaceSubClass 0x02 子类音频流 */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/

  /* USB Speaker Standard AS Interface Descriptor - Audio Streaming Operational */
  /* Interface 2, Alternate Setting 1                                           */
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  USB_DESC_TYPE_INTERFACE,              /* bDescriptorType 0x04 接口描述符 */
#if AUDIO_PORT_IF_NUM == 2
  0x01,                                 /* bInterfaceNumber 本接口索引 index 1 */
#elif AUDIO_PORT_IF_NUM == 3
  0x02,                                 /* bInterfaceNumber 本接口索引 index 2 */
#endif
  0x01,                                 /* bAlternateSetting 0时为默认的无音频输出接口即无端点描述符，从bAlternateSetting=1开始有数据传输 */
  0x01,                                 /* bNumEndpoints 端点数1 */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass 0x01 音频接口类 */
  AUDIO_SUBCLASS_AUDIOSTREAMING,        /* bInterfaceSubClass 0x02 子类音频流 */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/

  /* USB Speaker Audio Streaming Interface Descriptor */
  AUDIO_STREAMING_INTERFACE_DESC_SIZE,  /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType 0x24 音频接口描述符 */
  AUDIO_STREAMING_GENERAL,              /* bDescriptorSubtype 0x01 一般音频流 */
  AUDIO_PORT_INPUT_TERMINAL_ID_4,      /* bTerminalLink 连接 音频流输入终端 1 */
  0x01,                                 /* bDelay 1ms */
  0x01,                                 /* wFormatTag AUDIO_FORMAT_PCM  0x0001 */
  0x00,
  /* 07 byte*/

  /* USB Speaker Audio Type III Format Interface Descriptor */
  0x0B,                                 /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType 0x24 音频接口描述符 */
  AUDIO_STREAMING_FORMAT_TYPE,          /* bDescriptorSubtype 0x02 音频格式描述符 */
  AUDIO_FORMAT_TYPE_I,                  /* bFormatType */
  AUDIO_PORT_CHANNEL_NUMS,              /* bNrChannels 2个通道 */
  0x02,                                 /* bSubFrameSize :  2 Bytes per frame (16bits) */
  16,                                   /* bBitResolution (16-bits per sample) */
  0x01,                                 /* bSamFreqType only one frequency supported 支持几中采样率，在下面每3Bytes描述支持的采样率 */
  AUDIO_SAMPLE_FREQ(AUDIO_PORT_USBD_AUDIO_FREQ),   /* Audio sampling frequency coded on 3 bytes */
  /* 11 byte*/

  /* Endpoint 2 - Standard Descriptor */
  AUDIO_STANDARD_ENDPOINT_DESC_SIZE,    /* bLength */
  USB_DESC_TYPE_ENDPOINT,               /* bDescriptorType 0x05 端点描述符 */
  SPEAKER_OUT_EP,            /* bEndpointAddress 0x02 输出端点 */
  USBD_EP_TYPE_ISOC,                    /* bmAttributes */
  AUDIO_PORT_PACKET_SZE(AUDIO_PORT_USBD_AUDIO_FREQ),/* wMaxPacketSize in Bytes (Freq(Samples)*2(Stereo)*2(HalfWord)) */
  AUDIO_PORT_FS_BINTERVAL,              /* bInterval */
  0x00,                                 /* bRefresh */
  0x00,                                 /* bSynchAddress */
  /* 09 byte*/

  /* Endpoint - Audio Streaming Descriptor*/
  AUDIO_STREAMING_ENDPOINT_DESC_SIZE,   /* bLength */
  AUDIO_ENDPOINT_DESCRIPTOR_TYPE,       /* bDescriptorType 0x25 音频端点描述符 */
  AUDIO_ENDPOINT_GENERAL,               /* bDescriptor 0x01 一般端点描述 */
  0x80,                                 /* bmAttributes */
  0x01,                                 /* bLockDelayUnits */
  0x00,                                 /* wLockDelay */
  0x00,
  /* 07 byte*/
#endif
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_AUDIO_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};
#endif /* USE_USBD_COMPOSITE  */

/**
  * @}
  */
/** Private variables --------------------------------------------------------*/

/** Private function prototypes ----------------------------------------------*/
/** @defgroup USBD_AUDIO_Private_FunctionPrototypes
  * @{
  */
static uint8_t USBD_AUDIO_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_AUDIO_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);

static uint8_t USBD_AUDIO_Setup(USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req);
#ifndef USE_USBD_COMPOSITE
static uint8_t *USBD_AUDIO_GetCfgDesc(uint16_t *length);
static uint8_t *USBD_AUDIO_GetDeviceQualifierDesc(uint16_t *length);
#endif /* USE_USBD_COMPOSITE  */
static uint8_t USBD_AUDIO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_AUDIO_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_AUDIO_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_AUDIO_EP0_TxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_AUDIO_SOF(USBD_HandleTypeDef *pdev);

static uint8_t USBD_AUDIO_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_AUDIO_IsoOutIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);
static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void *USBD_AUDIO_GetAudioHeaderDesc(uint8_t *pConfDesc);

/**
  * @}
  */
/** Public variables ---------------------------------------------------------*/

/** @defgroup USBD_AUDIO_Private_Variables
  * @{
  */

USBD_ClassTypeDef USBD_HEADPHONE =
{
  USBD_AUDIO_Init,
  USBD_AUDIO_DeInit,
  USBD_AUDIO_Setup,
  USBD_AUDIO_EP0_TxReady,
  USBD_AUDIO_EP0_RxReady,
  USBD_AUDIO_DataIn,
  USBD_AUDIO_DataOut,
  USBD_AUDIO_SOF,
  USBD_AUDIO_IsoINIncomplete,
  USBD_AUDIO_IsoOutIncomplete,
#ifdef USE_USBD_COMPOSITE
  NULL,
  NULL,
  NULL,
  NULL,
#else
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetDeviceQualifierDesc,
#endif /* USE_USBD_COMPOSITE  */
};

/** Private user code --------------------------------------------------------*/


/** Private application code -------------------------------------------------*/
/*******************************************************************************
*
*       Static code
*
********************************************************************************
*/


/** @defgroup USBD_AUDIO_Private_Functions
  * @{
  */

/**
  * @brief  USBD_AUDIO_Init
  *         Initialize the AUDIO interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_AUDIO_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  uint8_t ret = 0;
  /* 打开输出端点 */
#if AUDIO_PORT_USE_SPEAKER || (AUDIO_PORT_IF_NUM == 3)
  ret = USB_Audio_Port_EP_OUT_Init(pdev, cfgidx);
  if(ret != (uint8_t)USBD_OK)
  {
    return ret;
  }
#endif

  /* 打开输入端点 */
#if AUDIO_PORT_USE_MICROPHONE || (AUDIO_PORT_IF_NUM == 3)
  ret = USB_Audio_Port_EP_IN_Init(pdev, cfgidx);
  if(ret != (uint8_t)USBD_OK)
  {
    return ret;
  }
#endif
  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_AUDIO_Init
  *         DeInitialize the AUDIO layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_AUDIO_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  uint8_t ret = 0;
  /* 关闭输出端点 */
#if AUDIO_PORT_USE_SPEAKER || (AUDIO_PORT_IF_NUM == 3)
  ret = USB_Audio_Port_EP_OUT_DeInit(pdev, cfgidx);
  if(ret != (uint8_t)USBD_OK)
  {
    return ret;
  }
#endif

  /* 关闭输入端点 */
#if AUDIO_PORT_USE_MICROPHONE || (AUDIO_PORT_IF_NUM == 3)
  ret = USB_Audio_Port_EP_IN_DeInit(pdev, cfgidx);
  if(ret != (uint8_t)USBD_OK)
  {
    return ret;
  }
#endif
  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_AUDIO_Setup
  *         Handle the AUDIO specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_AUDIO_Setup(USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef *haudio;
  uint16_t len;
  uint8_t *pbuf;
  uint16_t status_info = 0U;
  USBD_StatusTypeDef ret = USBD_OK;

  haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  if (haudio == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS:
      switch (req->bRequest)
      {
        case AUDIO_REQ_GET_CUR:
          AUDIO_REQ_GetCurrent(pdev, req);
          break;

        case AUDIO_REQ_SET_CUR:
          AUDIO_REQ_SetCurrent(pdev, req);
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_DESCRIPTOR:
          if ((req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE)
          {
            pbuf = (uint8_t *)USBD_AUDIO_GetAudioHeaderDesc(pdev->pConfDesc);
            if (pbuf != NULL)
            {
              len = MIN(USB_AUDIO_DESC_SIZ, req->wLength);
              (void)USBD_CtlSendData(pdev, pbuf, len);
            }
            else
            {
              USBD_CtlError(pdev, req);
              ret = USBD_FAIL;
            }
          }
          break;

        case USB_REQ_GET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, (uint8_t *)&haudio->alt_setting, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            if ((uint8_t)(req->wValue) <= USBD_MAX_NUM_INTERFACES)
            {
              haudio->alt_setting = (uint8_t)(req->wValue);
            }
            else
            {
              /* Call the error management function (command will be NAKed */
              USBD_CtlError(pdev, req);
              ret = USBD_FAIL;
            }
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_CLEAR_FEATURE:
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;
    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }

  return (uint8_t)ret;
}

#ifndef USE_USBD_COMPOSITE
/**
  * @brief  USBD_AUDIO_GetCfgDesc
  *         return configuration descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_AUDIO_GetCfgDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_AUDIO_CfgDesc);

  return USBD_AUDIO_CfgDesc;
}
#endif /* USE_USBD_COMPOSITE  */

/**
  * @brief  USBD_AUDIO_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_AUDIO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
#if AUDIO_PORT_USE_MICROPHONE || (AUDIO_PORT_IF_NUM == 3)
  return USB_Audio_Port_DataIn(pdev, epnum);
#else
  return (uint8_t)USBD_OK;
#endif
}

/**
  * @brief  USBD_AUDIO_EP0_RxReady
  *         handle EP0 Rx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_AUDIO_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  USBD_AUDIO_HandleTypeDef *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  if (haudio == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  if (haudio->control.cmd == AUDIO_REQ_SET_CUR)
  {
    /* In this driver, to simplify code, only SET_CUR request is managed */

    if (haudio->control.unit == AUDIO_OUT_STREAMING_CTRL)
    {
      ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData[pdev->classId])->MuteCtl(haudio->control.data[0]);
      haudio->control.cmd = 0U;
      haudio->control.len = 0U;
    }
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_AUDIO_EP0_TxReady
  *         handle EP0 TRx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_AUDIO_EP0_TxReady(USBD_HandleTypeDef *pdev)
{
  UNUSED(pdev);

  /* Only OUT control data are processed */
  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_AUDIO_SOF
  *         handle SOF event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_AUDIO_SOF(USBD_HandleTypeDef *pdev)
{
  UNUSED(pdev);

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_AUDIO_IsoINIncomplete
  *         handle data ISO IN Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_AUDIO_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  UNUSED(pdev);
  UNUSED(epnum);

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_AUDIO_IsoOutIncomplete
  *         handle data ISO OUT Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_AUDIO_IsoOutIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  UNUSED(pdev);
  UNUSED(epnum);

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_AUDIO_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_AUDIO_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
#if AUDIO_PORT_USE_SPEAKER || (AUDIO_PORT_IF_NUM == 3)
  return USB_Audio_Port_DataOut(pdev, epnum);
#else
  return (uint8_t)USBD_OK;
#endif
}

/**
  * @brief  AUDIO_Req_GetCurrent
  *         Handles the GET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  if (haudio == NULL)
  {
    return;
  }

  (void)USBD_memset(haudio->control.data, 0, USB_MAX_EP0_SIZE);

  /* Send the current mute state */
  (void)USBD_CtlSendData(pdev, haudio->control.data,
                         MIN(req->wLength, USB_MAX_EP0_SIZE));
}

/**
  * @brief  AUDIO_Req_SetCurrent
  *         Handles the SET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  if (haudio == NULL)
  {
    return;
  }

  if (req->wLength != 0U)
  {
    haudio->control.cmd = AUDIO_REQ_SET_CUR;     /* Set the request value */
    haudio->control.len = (uint8_t)MIN(req->wLength, USB_MAX_EP0_SIZE);  /* Set the request data length */
    haudio->control.unit = HIBYTE(req->wIndex);  /* Set the request target unit */

    /* Prepare the reception of the buffer over EP0 */
    (void)USBD_CtlPrepareRx(pdev, haudio->control.data, haudio->control.len);
  }
}
#ifndef USE_USBD_COMPOSITE
/**
  * @brief  DeviceQualifierDescriptor
  *         return Device Qualifier descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_AUDIO_GetDeviceQualifierDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_AUDIO_DeviceQualifierDesc);

  return USBD_AUDIO_DeviceQualifierDesc;
}
#endif

/**
  * @brief  USBD_AUDIO_GetAudioHeaderDesc
  *         This function return the Audio descriptor
  * @param  pdev: device instance
  * @param  pConfDesc:  pointer to Bos descriptor
  * @retval pointer to the Audio AC Header descriptor
  */
static void *USBD_AUDIO_GetAudioHeaderDesc(uint8_t *pConfDesc)
{
  USBD_ConfigDescTypeDef *desc = (USBD_ConfigDescTypeDef *)(void *)pConfDesc;
  USBD_DescHeaderTypeDef *pdesc = (USBD_DescHeaderTypeDef *)(void *)pConfDesc;
  uint8_t *pAudioDesc =  NULL;
  uint16_t ptr;

  if (desc->wTotalLength > desc->bLength)
  {
    ptr = desc->bLength;

    while (ptr < desc->wTotalLength)
    {
      pdesc = USBD_GetNextDesc((uint8_t *)pdesc, &ptr);
      if ((pdesc->bDescriptorType == AUDIO_INTERFACE_DESCRIPTOR_TYPE) &&
          (pdesc->bDescriptorSubType == AUDIO_CONTROL_HEADER))
      {
        pAudioDesc = (uint8_t *)pdesc;
        break;
      }
    }
  }
  return pAudioDesc;
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

/**
  * @brief  USBD_AUDIO_SOF
  *         handle SOF event
  * @param  pdev: device instance
  * @retval status
  */
void USBD_HEADPHONE_Sync(USBD_HandleTypeDef *pdev, AUDIO_OffsetTypeDef offset)
{
  USB_Audio_Port_AUDIO_Sync(pdev, (int)offset);
}

/**
  * @brief  USBD_AUDIO_RegisterInterface
  * @param  fops: Audio interface callback
  * @retval status
  */
uint8_t USBD_HEADPHONE_RegisterInterface(USBD_HandleTypeDef *pdev,
                                     USBD_AUDIO_ItfTypeDef *fops)
{
  if (fops == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  pdev->pUserData[pdev->classId] = fops;

  return (uint8_t)USBD_OK;
}

#ifdef USE_USBD_COMPOSITE
/**
  * @brief  USBD_HEADPHONE_GetEpPcktSze
  * @param  pdev: device instance (reserved for future use)
  * @param  If: Interface number (reserved for future use)
  * @param  Ep: Endpoint number (reserved for future use)
  * @retval status
  */
uint32_t USBD_HEADPHONE_GetEpPcktSze(USBD_HandleTypeDef *pdev, uint8_t If, uint8_t Ep)
{
  uint32_t mps;

  UNUSED(pdev);
  UNUSED(If);
  UNUSED(Ep);

  mps = AUDIO_PACKET_SZE_WORD(USBD_AUDIO_FREQ);

  /* Return the wMaxPacketSize value in Bytes (Freq(Samples)*2(Stereo)*2(HalfWord)) */
  return mps;
}
#endif /* USE_USBD_COMPOSITE */

#ifdef __cplusplus ///<end extern c
}
#endif
/******************************** End of file *********************************/
