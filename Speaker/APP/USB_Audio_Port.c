/**
 *  @file USB_Audio_Port.c
 *
 *  @date 2021-06-01
 *
 *  @author aron566
 *
 *  @copyright 爱谛科技研究院.
 *
 *  @brief USB音频类接口，对外提供音频更新及输出接口
 *
 *  @details 1、开启对应FS中断，中断处理和初始化位于usbd_conf.c
 *           2、16k采样，双声道，10ms出320点数据
 *           3、1ms间隔发送，10ms发送10次，每次发送320/10 = 32点数据 数据大小64字节（16bit*32）
 *           4、接收来自MIC数据，10ms来一次每次双通道160点*2
 *  @version V1.0
 */
/** Includes -----------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
#include "USB_Audio_Port.h"
#include "main.h"
#include "usbd_audio.h"
#include "usbd_audio_if.h"
#include "CircularQueue.h"
#include "usbd_headphone.h"
/** Use C compiler -----------------------------------------------------------*/
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Private typedef ----------------------------------------------------------*/
/** Private macros -----------------------------------------------------------*/
#define USB_PORT_AUDIO_OUT_PACKET AUDIO_PORT_OUT_SIZE    /**< 一次发送大小字节数*/
#define STEREO_FRAME_SIZE         256U
#define USB_PORT_AUDIO_IN_EP      MICROPHONE_IN_EP
#define USB_PORT_AUDIO_OUT_EP     SPEAKER_OUT_EP
/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
extern SAI_HandleTypeDef hsai_BlockA1;
/** Private variables --------------------------------------------------------*/
/* Microphone缓冲区 */
static CQ_handleTypeDef USB_Audio_Data_Handle;
static uint16_t USB_Audio_Send_Buf_CQ[STEREO_FRAME_SIZE * 2] = {0};

/* Speaker缓冲区 */
static CQ_handleTypeDef USB_Audio_PC_Data_Handle;
static uint16_t USB_Audio_From_PC_Buf_CQ[STEREO_FRAME_SIZE * 2] = {0};
/** Private function prototypes ----------------------------------------------*/

/** Private user code --------------------------------------------------------*/

/** Private application code -------------------------------------------------*/
/*******************************************************************************
*
*       Static code
*
********************************************************************************
*/

/**
 * @brief 输入端口初始化
 *
 */
static inline void USB_Audio_Port_EPIN_Init(void)
{
  /* 初始化接收音频缓冲区 */
  CQ_16_init(&USB_Audio_Data_Handle, USB_Audio_Send_Buf_CQ, STEREO_FRAME_SIZE * 2);
}

/**
 * @brief 输出端口初始化
 *
 */
static inline void USB_Audio_Port_EPOUT_Init(void)
{
  /* 初始化接收音频缓冲区 */
  CQ_16_init(&USB_Audio_PC_Data_Handle, USB_Audio_From_PC_Buf_CQ, STEREO_FRAME_SIZE * 2);
}
/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

/**
 * @brief 音频传输完成同步
 *
 */
void USB_Audio_Port_TransferComplete_CallBack_FS(void)
{
  TransferComplete_CallBack_FS();
}

/**
 * @brief 音频半传输完成同步
 *
 */
void USB_Audio_Port_HalfTransfer_CallBack_FS(void)
{
  HalfTransfer_CallBack_FS();
}

/**
 * @brief 处理PC音频数据
 *
 * @param pbuf 数据
 * @param size 数据长度字节
 * @param cmd 命令
 * @return int8_t 返回状态
 */
int8_t USB_Audio_Port_Process_PC_Audio(uint8_t *pbuf, uint32_t size, uint8_t cmd)
{
  /* 采样点数 = 字节数 / 2  = 16位数据点 */
  uint32_t Sample_Num = size / 2;
  if(CQ_isFull(&USB_Audio_PC_Data_Handle) == true)
  {
    CQ_ManualOffsetInc(&USB_Audio_PC_Data_Handle, Sample_Num);
  }
  CQ_16putData(&USB_Audio_PC_Data_Handle, (const uint16_t *)pbuf, Sample_Num);
  return (int8_t)USBD_OK;
}

/**
 * @brief 获取PC音频数据
 *
 * @param Left_Buf 左通道音频存储区
 * @param Right_Buf 右通道音频存储区
 * @return uint32_t 获取到的数据样点数
 */
uint32_t USB_Audio_Port_Get_PC_Audio_Data(int16_t *Left_Buf, int16_t *Right_Buf)
{
#if AUDIO_PORT_USE_SPEAKER || (AUDIO_PORT_IF_NUM == 3)
  int16_t USB_Audio_Receive_Buf[STEREO_FRAME_SIZE] = {0};
  uint32_t Len = CQ_getLength(&USB_Audio_PC_Data_Handle);
  Len = Len > STEREO_FRAME_SIZE ? STEREO_FRAME_SIZE : Len;
  if((Len % 2) != 0 || Len < STEREO_FRAME_SIZE)
  {
    return 0;
  }
  CQ_16getData(&USB_Audio_PC_Data_Handle, (uint16_t *)USB_Audio_Receive_Buf, Len);
  int index = 0;
  for(int i = 0; i < Len; index++)
  {
    Left_Buf[index] = USB_Audio_Receive_Buf[i++];/**< TO DAC LEFT*/
    Right_Buf[index] = USB_Audio_Receive_Buf[i++];/**< TO DAC RIGHT*/
  }
  return Len;
#else
  return 0;
#endif
}

/**
  * @brief  USBD_AUDIO_SOF
  *         handle SOF event
  * @param  xpdev: device instance
  * @retval status
  */
void USB_Audio_Port_AUDIO_Sync(void *xpdev, int offset)
{
  USBD_AUDIO_HandleTypeDef *haudio;
  USBD_HandleTypeDef *pdev = (USBD_HandleTypeDef *)xpdev;
  uint32_t BufferSize = AUDIO_TOTAL_BUF_SIZE / 2U;

  if(pdev->pClassDataCmsit[pdev->classId] == NULL)
  {
    return;
  }

  haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  haudio->offset = (AUDIO_OffsetTypeDef)offset;

  if(haudio->rd_enable == 1U)
  {
    haudio->rd_ptr += (uint16_t)BufferSize;

    if(haudio->rd_ptr == AUDIO_TOTAL_BUF_SIZE)
    {
      /* roll back */
      haudio->rd_ptr = 0U;
    }
  }

  if(haudio->rd_ptr > haudio->wr_ptr)
  {
    if((haudio->rd_ptr - haudio->wr_ptr) < AUDIO_OUT_PACKET)
    {
      BufferSize += 4U;
    }
    else
    {
      if((haudio->rd_ptr - haudio->wr_ptr) > (AUDIO_TOTAL_BUF_SIZE - AUDIO_OUT_PACKET))
      {
        BufferSize -= 4U;
      }
    }
  }
  else
  {
    if((haudio->wr_ptr - haudio->rd_ptr) < AUDIO_OUT_PACKET)
    {
      BufferSize -= 4U;
    }
    else
    {
      if((haudio->wr_ptr - haudio->rd_ptr) > (AUDIO_TOTAL_BUF_SIZE - AUDIO_OUT_PACKET))
      {
        BufferSize += 4U;
      }
    }
  }

  if(haudio->offset == AUDIO_OFFSET_FULL)
  {
    ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData[pdev->classId])->AudioCmd(&haudio->buffer[0],
                                                                        BufferSize, AUDIO_CMD_PLAY);
    haudio->offset = AUDIO_OFFSET_NONE;
  }
}

/**
  ******************************************************************
  * @brief   USB缓冲区数据接收来自HOST
  * @param   [in]pdev device instance
  * @param   [in]epnum 端点号
  * @return  USBD_OK 正常.
  * @author  aron566
  * @version V1.0
  * @date    2021-06-01
  ******************************************************************
  */
uint8_t USB_Audio_Port_DataOut(void *xpdev, uint8_t epnum)
{
  uint16_t PacketSize;
  USBD_AUDIO_HandleTypeDef *haudio;
  USBD_HandleTypeDef *pdev = (USBD_HandleTypeDef *)xpdev;
  uint8_t AUDIOOutEpAdd = USB_PORT_AUDIO_OUT_EP;
#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  AUDIOOutEpAdd  = USBD_CoreGetEPAdd(pdev, USB_PORT_AUDIO_OUT_EP, USBD_EP_TYPE_ISOC);
#endif /* USE_USBD_COMPOSITE */

  haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  if (haudio == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  if (epnum == AUDIOOutEpAdd)
  {
    /* Get received data packet length */
    PacketSize = (uint16_t)USBD_LL_GetRxDataSize(pdev, epnum);

    /* Packet received Callback */
    ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData[pdev->classId])->PeriodicTC(&haudio->buffer[haudio->wr_ptr],
                                                                          PacketSize, AUDIO_OUT_TC);

    /* Increment the Buffer pointer or roll it back when all buffers are full */
    haudio->wr_ptr += PacketSize;

    if (haudio->wr_ptr == AUDIO_TOTAL_BUF_SIZE)
    {
      /* All buffers are full: roll back */
      haudio->wr_ptr = 0U;

      if (haudio->offset == AUDIO_OFFSET_UNKNOWN)
      {
        ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData[pdev->classId])->AudioCmd(&haudio->buffer[0],
                                                                            AUDIO_TOTAL_BUF_SIZE / 2U,
                                                                            AUDIO_CMD_START);
        haudio->offset = AUDIO_OFFSET_NONE;
      }
    }

    if (haudio->rd_enable == 0U)
    {
      if (haudio->wr_ptr == (AUDIO_TOTAL_BUF_SIZE / 2U))
      {
        haudio->rd_enable = 1U;
      }
    }

    /* Prepare Out endpoint to receive next audio packet */
    (void)USBD_LL_PrepareReceive(pdev, AUDIOOutEpAdd,
                                 &haudio->buffer[haudio->wr_ptr],
                                 AUDIO_OUT_PACKET);
  }

  return (uint8_t)USBD_OK;
}

/**
  ******************************************************************
  * @brief   USB缓冲区数据发送至HOST
  * @param   [in]pdev device instance
  * @param   [in]epnum 端点号
  * @return  USBD_OK 正常.
  * @author  aron566
  * @version V1.0
  * @date    2021-06-01
  ******************************************************************
  */
uint8_t USB_Audio_Port_DataIn(void *xpdev, uint8_t epnum)
{
  USBD_HandleTypeDef *pdev = (USBD_HandleTypeDef *)xpdev;
  USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

	USBD_LL_FlushEP(pdev, USB_PORT_AUDIO_IN_EP);
  if(CQ_getLength(&USB_Audio_Data_Handle) < USB_PORT_AUDIO_OUT_PACKET/2)
  {
    return (uint8_t)USBD_BUSY;
  }
  CQ_16getData(&USB_Audio_Data_Handle, (uint16_t *)haudio->buffer, USB_PORT_AUDIO_OUT_PACKET/2);
  return USBD_LL_Transmit(pdev, USB_PORT_AUDIO_IN_EP, haudio->buffer, USB_PORT_AUDIO_OUT_PACKET);
}

/**
  ******************************************************************
  * @brief   反初始化音频输出端点
  * @param   [in]pdev
  * @param   [in]cfgidx
  * @return  USBD_OK 正常.
  * @author  aron566
  * @version V1.0
  * @date    2021-06-01
  ******************************************************************
  */
uint8_t USB_Audio_Port_EP_OUT_DeInit(void *xpdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);
  USBD_HandleTypeDef *pdev = (USBD_HandleTypeDef *)xpdev;
  uint8_t AUDIOOutEpAdd = USB_PORT_AUDIO_OUT_EP;
#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  AUDIOOutEpAdd  = USBD_CoreGetEPAdd(pdev, USB_PORT_AUDIO_OUT_EP, USBD_EP_TYPE_ISOC);
#endif /* USE_USBD_COMPOSITE */

  /* Close EP OUT */
  (void)USBD_LL_CloseEP(pdev, AUDIOOutEpAdd);
  pdev->ep_out[AUDIOOutEpAdd & 0xFU].is_used = 0U;
  pdev->ep_out[AUDIOOutEpAdd & 0xFU].bInterval = 0U;

  /* DeInit  physical Interface components */
  if (pdev->pClassDataCmsit[pdev->classId] != NULL)
  {
    ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData[pdev->classId])->DeInit(0U);
    (void)USBD_free(pdev->pClassDataCmsit[pdev->classId]);
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    pdev->pClassData = NULL;
  }

  return (uint8_t)USBD_OK;
}

/**
  ******************************************************************
  * @brief   反初始化音频输入端点
  * @param   [in]pdev
  * @param   [in]cfgidx
  * @return  USBD_OK 正常.
  * @author  aron566
  * @version V1.0
  * @date    2021-06-01
  ******************************************************************
  */
uint8_t USB_Audio_Port_EP_IN_DeInit(void *xpdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);
  USBD_HandleTypeDef *pdev = (USBD_HandleTypeDef *)xpdev;
  uint8_t AUDIOInEpAdd = USB_PORT_AUDIO_IN_EP;
#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  AUDIOInEpAdd  = USBD_CoreGetEPAdd(pdev, USB_PORT_AUDIO_IN_EP, USBD_EP_TYPE_ISOC);
#endif /* USE_USBD_COMPOSITE */

  /* Close EP IN */
  (void)USBD_LL_CloseEP(pdev, AUDIOInEpAdd);
  pdev->ep_in[AUDIOInEpAdd & 0xFU].is_used = 0U;
  pdev->ep_in[AUDIOInEpAdd & 0xFU].bInterval = 0U;

  /* DeInit  physical Interface components */
  if (pdev->pClassDataCmsit[pdev->classId] != NULL)
  {
    ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData[pdev->classId])->DeInit(0U);
    (void)USBD_free(pdev->pClassDataCmsit[pdev->classId]);
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    pdev->pClassData = NULL;
  }

  return (uint8_t)USBD_OK;
}

/**
  ******************************************************************
  * @brief   初始化为音频输出端点
  * @param   [in]pdev
  * @param   [in]cfgidx
  * @return  USBD_OK 正常.
  * @author  aron566
  * @version V1.0
  * @date    2021-06-01
  ******************************************************************
  */
uint8_t USB_Audio_Port_EP_OUT_Init(void *xpdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);
  USBD_AUDIO_HandleTypeDef *haudio;
  USBD_HandleTypeDef *pdev = (USBD_HandleTypeDef *)xpdev;
  uint8_t AUDIOOutEpAdd = USB_PORT_AUDIO_OUT_EP;

  /* USB接口初始化 */
  USB_Audio_Port_EPOUT_Init();

  static uint32_t mem[(sizeof(USBD_AUDIO_HandleTypeDef)/4)+1];/* On 32-bit boundary */
  /* Allocate Audio structure */

  haudio = (USBD_AUDIO_HandleTypeDef *)mem;

  if(haudio == NULL)
  {
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    return (uint8_t)USBD_EMEM;
  }

  pdev->pClassDataCmsit[pdev->classId] = (void *)haudio;
  pdev->pClassData = pdev->pClassDataCmsit[pdev->classId];

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  AUDIOOutEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_ISOC);
#endif /* USE_USBD_COMPOSITE */

  if(pdev->dev_speed == USBD_SPEED_HIGH)
  {
    pdev->ep_out[AUDIOOutEpAdd & 0xFU].bInterval = AUDIO_HS_BINTERVAL;
  }
  else   /* LOW and FULL-speed endpoints */
  {
    pdev->ep_out[AUDIOOutEpAdd & 0xFU].bInterval = AUDIO_FS_BINTERVAL;
  }

  /* Open EP OUT */
  (void)USBD_LL_OpenEP(pdev, AUDIOOutEpAdd, USBD_EP_TYPE_ISOC, USB_PORT_AUDIO_OUT_PACKET);
  pdev->ep_out[AUDIOOutEpAdd & 0xFU].is_used = 1U;

  haudio->alt_setting = 0U;
  haudio->offset = AUDIO_OFFSET_UNKNOWN;
  haudio->wr_ptr = 0U;
  haudio->rd_ptr = 0U;
  haudio->rd_enable = 0U;

  /* Initialize the Audio output Hardware layer */
  if(((USBD_AUDIO_ItfTypeDef *)pdev->pUserData[pdev->classId])->Init(AUDIO_PORT_USBD_AUDIO_FREQ,
                                                                      AUDIO_DEFAULT_VOLUME,
                                                                      0U) != 0U)
  {
    return (uint8_t)USBD_FAIL;
  }

  /* Prepare Out endpoint to receive 1st packet */
  (void)USBD_LL_PrepareReceive(pdev, AUDIOOutEpAdd, haudio->buffer,
                               USB_PORT_AUDIO_OUT_PACKET);

  return (uint8_t)USBD_OK;
}

/**
  ******************************************************************
  * @brief   初始化为音频输入端点
  * @param   [in]pdev
  * @param   [in]cfgidx
  * @return  USBD_OK 正常.
  * @author  aron566
  * @version V1.0
  * @date    2021-06-01
  ******************************************************************
  */
uint8_t USB_Audio_Port_EP_IN_Init(void *xpdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);
  USBD_AUDIO_HandleTypeDef *haudio;
  USBD_HandleTypeDef *pdev = (USBD_HandleTypeDef *)xpdev;
  uint8_t AUDIOInEpAdd = USB_PORT_AUDIO_IN_EP;

  /* USB接口初始化 */
  USB_Audio_Port_EPIN_Init();

  static uint32_t mem[(sizeof(USBD_AUDIO_HandleTypeDef)/4)+1];/* On 32-bit boundary */
  /* Allocate Audio structure */

  haudio = (USBD_AUDIO_HandleTypeDef *)mem;

  if(haudio == NULL)
  {
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    return (uint8_t)USBD_EMEM;
  }

  pdev->pClassDataCmsit[pdev->classId] = (void *)haudio;
  pdev->pClassData = pdev->pClassDataCmsit[pdev->classId];

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  AUDIOInEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_ISOC);
#endif /* USE_USBD_COMPOSITE */

  if(pdev->dev_speed == USBD_SPEED_HIGH)
  {
    pdev->ep_in[AUDIOInEpAdd & 0xFU].bInterval = AUDIO_HS_BINTERVAL;
  }
  else   /* LOW and FULL-speed endpoints */
  {
    pdev->ep_in[AUDIOInEpAdd & 0xFU].bInterval = AUDIO_FS_BINTERVAL;
  }

  /* Open EP OUT */
  (void)USBD_LL_OpenEP(pdev, AUDIOInEpAdd, USBD_EP_TYPE_ISOC, USB_PORT_AUDIO_OUT_PACKET);
  pdev->ep_in[AUDIOInEpAdd & 0xFU].is_used = 1U;

  haudio->alt_setting = 0U;
  haudio->offset = AUDIO_OFFSET_UNKNOWN;
  haudio->wr_ptr = 0U;
  haudio->rd_ptr = 0U;
  haudio->rd_enable = 0U;

  /* Initialize the Audio output Hardware layer */
  if(((USBD_AUDIO_ItfTypeDef *)pdev->pUserData[pdev->classId])->Init(AUDIO_PORT_USBD_AUDIO_FREQ,
                                                                      AUDIO_DEFAULT_VOLUME,
                                                                      0U) != 0U)
  {
    return (uint8_t)USBD_FAIL;
  }

  /* Prepare Out endpoint to receive 1st packet */
  (void)USBD_LL_Transmit(pdev, AUDIOInEpAdd, haudio->buffer,
                               USB_PORT_AUDIO_OUT_PACKET);

  return (uint8_t)USBD_OK;
}

/**
  ******************************************************************
  * @brief   更新USB音频数据
  * @param   [in]Left_Audio 左通道数据
  * @param   [in]Right_Audio 右通道数据
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-06-01
  ******************************************************************
  */
void USB_Audio_Port_Put_Data(const int16_t *Left_Audio, const int16_t *Right_Audio)
{
  /*更新USB音频数据*/
  int16_t USB_Audio_Receive_Buf[STEREO_FRAME_SIZE] = {0};
  int index = 0, i = 0;
  for(i = 0; i < STEREO_FRAME_SIZE; index++)
  {
    USB_Audio_Receive_Buf[i++] = Left_Audio[index];/**< TO USB LEFT*/
    USB_Audio_Receive_Buf[i++] = Right_Audio[index];/**< TO USB RIGHT*/
  }

  CQ_16putData(&USB_Audio_Data_Handle, (const uint16_t *)USB_Audio_Receive_Buf, STEREO_FRAME_SIZE);
}

#ifdef __cplusplus ///<end extern c
}
#endif
/******************************** End of file *********************************/
