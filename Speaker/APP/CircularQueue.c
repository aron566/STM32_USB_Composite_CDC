/**
 *  @file CircularQueue.c
 *
 *  @date 2022/03/30
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2021 aron566 <aron566@163.com>.
 *
 *  @brief 环形缓冲区接口
 *
 *  @details None
 *
 *  @version v1.4
 */
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Includes -----------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
#include "CircularQueue.h"
#if USE_LINUX_SYSTEM
#include <sys/types.h>
#endif
// #include "Algorithm_Math_Port.h"

/** Private typedef ----------------------------------------------------------*/
/** Private macros -----------------------------------------------------------*/
/**
 * @name 返回值定义
 * @{
 */
#define TRUE true
#define FALSE false
/** @}*/

/* 区域指定 */
#define AT_CQ_CODE_SPACE  //MATH_PORT_SECTION("USE_ITCM_SPACE")//(".text.aidis.cq")
#define AT_CQ_RAM_SPACE   //MATH_PORT_SECTION("USE_DTCM_SPACE")//(".aids.cq.cache")
#define AT_CQ_RO_SPACE    //MATH_PORT_SECTION("USE_ITCM_SPACE")//(".text.aidis.cq.ro")

#define USE_ARM_DSP_FUNC_EN   0/**< 使能DSP加速 */
/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
/** Private function prototypes ----------------------------------------------*/
/** Private user code --------------------------------------------------------*/

/** Private application code -------------------------------------------------*/
/*******************************************************************************
*
*       Static code
*
********************************************************************************
*/
/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

/**
 * [CQ_init 环形缓冲区初始化]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  memAdd        [数据存储区]
 * @param  len           [缓冲区大小]
 * @return               [初始化成功状态]
 */
AT_CQ_CODE_SPACE
bool CQ_init(CQ_handleTypeDef *CircularQueue, uint8_t *memAdd, uint32_t len)
{
  CircularQueue->size = len;

  if(!IS_POWER_OF_2(CircularQueue->size))
  {
    return FALSE;
  }

  if(memAdd == NULL)
  {
    return FALSE;
  }

  CircularQueue->Buffer.data8Buffer = memAdd;

  memset(CircularQueue->Buffer.data8Buffer, 0, len);
  CircularQueue->entrance = CircularQueue->exit = 0;

  return TRUE;
}

/**
 * [CQ_isEmpty 环形缓冲区判断是否为空]
 * @param  CircularQueue [环形缓冲区句柄]
 * @return               [TRUE 为空]
 */
AT_CQ_CODE_SPACE
bool CQ_isEmpty(CQ_handleTypeDef *CircularQueue)
{
  if(CircularQueue->entrance == CircularQueue->exit)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/**
 * [CQ_isFull 环形缓冲区判断是否为满]
 * @param  CircularQueue [环形缓冲区句柄]
 * @return               [TRUE 为满]
 */
AT_CQ_CODE_SPACE
bool CQ_isFull(CQ_handleTypeDef *CircularQueue)
{
  if((CircularQueue->entrance - CircularQueue->exit) == CircularQueue->size)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/**
 * [CQ_getLength 环形缓冲区获取剩余空间长度]
 * @param  CircularQueue [环形缓冲区句柄]
 * @return               [剩余长度]
 */
AT_CQ_CODE_SPACE
uint32_t CQ_getLength(CQ_handleTypeDef*CircularQueue)
{
  return (CircularQueue->entrance - CircularQueue->exit) > CircularQueue->size?CircularQueue->size:(CircularQueue->entrance - CircularQueue->exit);
}

/**
 * [CQ_emptyData 环形缓冲区清空操作]
 * @param  CircularQueue [环形缓冲区句柄]
 * @return               [None]
 */
AT_CQ_CODE_SPACE
void CQ_emptyData(CQ_handleTypeDef*CircularQueue)
{
  CircularQueue->entrance = CircularQueue->exit = 0;
  memset(CircularQueue->Buffer.data8Buffer, 0, CircularQueue->size);
}

/**
 * [CQ_getData 环形缓冲区读走数据]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  targetBuf     [目标缓冲区]
 * @return               [读取的长度]
 */
AT_CQ_CODE_SPACE
uint32_t CQ_getData(CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf, uint32_t len)
{
  uint32_t size = 0;

  /* 此次读取的实际大小，取 可读 和 目标读取数量  两个值小的那个 */
  len = GET_MIN(len, CircularQueue->entrance - CircularQueue->exit);
  /* 原理雷同存入 */
  size = GET_MIN(len, CircularQueue->size - (CircularQueue->exit & (CircularQueue->size - 1)));

  /* 使用DSP加速 */
#if USE_ARM_DSP_FUNC_EN
  arm_copy_q7((q7_t *)(CircularQueue->Buffer.data8Buffer + (CircularQueue->exit & (CircularQueue->size - 1))), (q7_t *)targetBuf, size);
  arm_copy_q7((q7_t *)CircularQueue->Buffer.data8Buffer, (q7_t *)(targetBuf + size), len - size);
#else
  memcpy(targetBuf, CircularQueue->Buffer.data8Buffer + (CircularQueue->exit & (CircularQueue->size - 1)), size);
  memcpy(targetBuf + size, CircularQueue->Buffer.data8Buffer, len - size);
#endif

  /* 利用无符号数据的溢出特性 */
  CircularQueue->exit += len;

  return len;
}


/**
 * [CQ_putData 环形缓冲区加入新数据]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  sourceBuf     [为实际存储区地址]
 * @param  len           [数据存入长度]
 * @return               [存入的长度]
 */
AT_CQ_CODE_SPACE
uint32_t CQ_putData(CQ_handleTypeDef *CircularQueue, const uint8_t * sourceBuf, uint32_t len)
{
  uint32_t size = 0;
  /* 此次存入的实际大小，取 剩余空间 和 目标存入数量  两个值小的那个 */
  len = GET_MIN(len, CircularQueue->size - CircularQueue->entrance + CircularQueue->exit);

  /* &(size-1)代替取模运算，同上原理，得到此次存入队列入口到末尾的大小 */
  size = GET_MIN(len, CircularQueue->size - (CircularQueue->entrance & (CircularQueue->size - 1)));

  /* 使用DSP加速 */
#if USE_ARM_DSP_FUNC_EN
  arm_copy_q7((q7_t *)sourceBuf, (q7_t *)(CircularQueue->Buffer.data8Buffer + (CircularQueue->entrance & (CircularQueue->size - 1))), size);
  arm_copy_q7((q7_t *)(sourceBuf + size), (q7_t *)(CircularQueue->Buffer.data8Buffer), len - size);
#else
  memcpy(CircularQueue->Buffer.data8Buffer + (CircularQueue->entrance & (CircularQueue->size - 1)), sourceBuf, size);
  memcpy(CircularQueue->Buffer.data8Buffer, sourceBuf + size, len - size);
#endif

  /* 利用无符号数据的溢出特性 */
  CircularQueue->entrance += len;

  return len;
}

/**
 * [DQ_putData 环形缓冲区加入新数据：每次数据帧开头先存入本帧的数据长度，所以每次先取一个字节得到包长度，再按长度取包]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  sourceBuf     [为实际存储区地址]
 * @param  len           [数据存入长度]
 * @return               [存入的长度]
 */
AT_CQ_CODE_SPACE
uint32_t DQ_putData(CQ_handleTypeDef *CircularQueue, const uint8_t * sourceBuf, uint32_t len)
{
  uint32_t size = 0;
  uint32_t lenth = 1;
  uint32_t pack_len = len;
  /* 取可存储大小 和 需存大小 小的值 */
  len = GET_MIN(len+lenth, CircularQueue->size - CircularQueue->entrance + CircularQueue->exit);//长度上头部加上数据长度记录

  /* 对kfifo->size取模运算可以转化为与运算，如：kfifo->in % kfifo->size 可以转化为 kfifo->in & (kfifo->size – 1) */
  /* &(size-1)代替取模运算，同上原理，得到此次存入队列入口到末尾的大小 */
  size = GET_MIN(len, CircularQueue->size - (CircularQueue->entrance & (CircularQueue->size - 1)));

  /* 使用DSP加速 */
#if USE_ARM_DSP_FUNC_EN
  arm_copy_q7((q7_t *)&pack_len, (q7_t *)(CircularQueue->Buffer.data8Buffer + (CircularQueue->entrance & (CircularQueue->size - 1))), lenth);
  arm_copy_q7((q7_t *)sourceBuf, (q7_t *)(CircularQueue->Buffer.data8Buffer + (CircularQueue->entrance & (CircularQueue->size - 1)) + lenth), size - lenth);
  arm_copy_q7((q7_t *)(sourceBuf + size - lenth), (q7_t *)(CircularQueue->Buffer.data8Buffer), len - size);
#else
  memcpy(CircularQueue->Buffer.data8Buffer + (CircularQueue->entrance & (CircularQueue->size - 1)), &pack_len, lenth);
  memcpy(CircularQueue->Buffer.data8Buffer + (CircularQueue->entrance & (CircularQueue->size - 1)) + lenth, sourceBuf, size - lenth);
  memcpy(CircularQueue->Buffer.data8Buffer, sourceBuf + size - lenth, len - size);
#endif

  /* 利用无符号数据的溢出特性 */
  CircularQueue->entrance += len;

  return len;
}

/**
 * [DQ_getData 环形缓冲区读走数据：DQ会调用CQ取走一字节数据用来判断本次数据包长度]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  targetBuf     [目标缓冲区]
 * @return               [读取的长度]
 */
AT_CQ_CODE_SPACE
uint32_t DQ_getData(CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf)
{
  uint32_t size = 0;
  uint32_t len = 0;
  /* 存储帧头 长度信息 */
  uint8_t package_len[1];
  /* 获取长度信息 */
  CQ_getData(CircularQueue, (uint8_t *)package_len, 1);
  len = package_len[0];
  /* 此次读取的实际大小，取 剩余可读 和 目标读取数量  两个值小的那个 */
  len = GET_MIN(len, CircularQueue->entrance - CircularQueue->exit);
  /* 原理雷同存入 */
  size = GET_MIN(len, CircularQueue->size - (CircularQueue->exit & (CircularQueue->size - 1)));

  /* 使用DSP加速 */
#if USE_ARM_DSP_FUNC_EN
  arm_copy_q7((q7_t *)(CircularQueue->Buffer.data8Buffer + (CircularQueue->exit & (CircularQueue->size - 1))), (q7_t *)targetBuf, size);
  arm_copy_q7((q7_t *)(CircularQueue->Buffer.data8Buffer), (q7_t *)(targetBuf + size), len - size);
#else
  memcpy(targetBuf, CircularQueue->Buffer.data8Buffer + (CircularQueue->exit & (CircularQueue->size - 1)), size);
  memcpy(targetBuf + size, CircularQueue->Buffer.data8Buffer, len - size);
#endif

  /* 利用无符号数据的溢出特性 */
  CircularQueue->exit += len;

  return len;
}

/**
  ******************************************************************
  * @brief   跳过8位无效帧头数据
  * @param   [in]cb 缓冲区
  * @param   [in]header_data 无效数据
  * @return  缓冲区可读长度
  * @author  aron566
  * @version V1.0
  * @date    2020-09-20
  ******************************************************************
  */
AT_CQ_CODE_SPACE
uint32_t CQ_skipInvaildU8Header(CQ_handleTypeDef *cb, uint8_t header_data)
{
  uint8_t header = 0;
  while(CQ_getLength(cb) > 0)
  {
    header = CQ_ManualGet_Offset_Data(cb, 0);
    /* 判断帧头 */
    if(header != header_data)
    {
      CQ_ManualOffsetInc(cb, 1);
    }
    else
    {
      return CQ_getLength(cb);
    }
  }
  return 0;
}

/**
  ******************************************************************
  * @brief   跳过16位无效帧头数据
  * @param   [in]cb 缓冲区
  * @param   [in]header_data 无效数据
  * @return  缓冲区可读长度
  * @author  aron566
  * @version V1.0
  * @date    2020-09-20
  ******************************************************************
  */
AT_CQ_CODE_SPACE
uint32_t CQ_skipInvaildU16Header(CQ_handleTypeDef *cb, uint16_t header_data)
{
  uint16_t header = 0;
  while(CQ_getLength(cb) >= 2)
  {
    header = CQ_ManualGet_Offset_Data(cb, 0);
    header |= (((uint16_t)CQ_ManualGet_Offset_Data(cb, 1))<<8);
    /* 判断帧头 */
    if(header != header_data)
    {
      CQ_ManualOffsetInc(cb, 1);
    }
    else
    {
      return CQ_getLength(cb);
    }
  }
  return 0;
}

/**
  ******************************************************************
  * @brief   跳过32位无效帧头数据
  * @param   [in]cb 缓冲区
  * @param   [in]header_data 无效数据
  * @return  缓冲区可读长度
  * @author  aron566
  * @version V1.0
  * @date    2020-09-20
  ******************************************************************
  */
AT_CQ_CODE_SPACE
uint32_t CQ_skipInvaildU32Header(CQ_handleTypeDef *cb, uint32_t header_data)
{
  uint32_t header = 0;
  while(CQ_getLength(cb) >= 4)
  {
    header = CQ_ManualGet_Offset_Data(cb, 0);
    header |= (((uint16_t)CQ_ManualGet_Offset_Data(cb, 1))<<8);
    header |= (((uint16_t)CQ_ManualGet_Offset_Data(cb, 2))<<16);
    header |= (((uint16_t)CQ_ManualGet_Offset_Data(cb, 3))<<24);
    /* 判断帧头 */
    if(header != header_data)
    {
      CQ_ManualOffsetInc(cb, 1);
    }
    else
    {
      return CQ_getLength(cb);
    }
  }
  return 0;
}

/**
  ******************************************************************
  * @brief   跳过Modbus16位无效帧头数据
  * @param   [in]cb 缓冲区
  * @param   [in]header_data 无效数据
  * @return  缓冲区可读长度
  * @author  aron566
  * @version V1.0
  * @date    2020-09-20
  ******************************************************************
  */
AT_CQ_CODE_SPACE
uint32_t CQ_skipInvaildModbusU16Header(CQ_handleTypeDef *cb, uint16_t header_data)
{
  uint16_t header = 0;
  while(CQ_getLength(cb) >= 2)
  {
    header = (((uint16_t)CQ_ManualGet_Offset_Data(cb, 0))<<8);
    header |= CQ_ManualGet_Offset_Data(cb, 1);
    /* 判断帧头 */
    if(header != header_data)
    {
      CQ_ManualOffsetInc(cb, 1);
    }
    else
    {
      return CQ_getLength(cb);
    }
  }
  return 0;
}

/**
  ******************************************************************
  * @brief   跳过Modbus32位无效帧头数据
  * @param   [in]cb 缓冲区
  * @param   [in]header_data 无效数据
  * @return  缓冲区可读长度
  * @author  aron566
  * @version V1.0
  * @date    2020-09-20
  ******************************************************************
  */
AT_CQ_CODE_SPACE
uint32_t CQ_skipInvaildModbusU32Header(CQ_handleTypeDef *cb, uint32_t header_data)
{
  uint32_t header = 0;
  while(CQ_getLength(cb) >= 4)
  {
    header = (((uint16_t)CQ_ManualGet_Offset_Data(cb, 0))<<24);
    header |= (((uint16_t)CQ_ManualGet_Offset_Data(cb, 1))<<16);
    header |= (((uint16_t)CQ_ManualGet_Offset_Data(cb, 2))<<8);
    header |= CQ_ManualGet_Offset_Data(cb, 3);
    /* 判断帧头 */
    if(header != header_data)
    {
      CQ_ManualOffsetInc(cb, 1);
    }
    else
    {
      return CQ_getLength(cb);
    }
  }
  return 0;
}

/**
 * [CQ_ManualGetData 手动缓冲区长度记录---适用于modbus解析]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  targetBuf     [目标缓冲区]
 * @param  len           [数据读取长度]
 * @return               [读取的长度]
 */
AT_CQ_CODE_SPACE
uint32_t CQ_ManualGetData(CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf, uint32_t len)
{
  uint32_t size = 0;

  /* 此次读取的实际大小，取 可读 和 目标读取数量  两个值小的那个 */
  len = GET_MIN(len, CircularQueue->entrance - CircularQueue->exit);
  /* 原理雷同存入 */
  size = GET_MIN(len, CircularQueue->size - (CircularQueue->exit & (CircularQueue->size - 1)));

  /* 使用DSP加速 */
#if USE_ARM_DSP_FUNC_EN
  arm_copy_q7((q7_t *)(CircularQueue->Buffer.data8Buffer + (CircularQueue->exit & (CircularQueue->size - 1))), (q7_t *)targetBuf, size);
  arm_copy_q7((q7_t *)(CircularQueue->Buffer.data8Buffer), (q7_t *)(targetBuf + size), len - size);
#else
  memcpy(targetBuf, CircularQueue->Buffer.data8Buffer + (CircularQueue->exit & (CircularQueue->size - 1)), size);
  memcpy(targetBuf + size, CircularQueue->Buffer.data8Buffer, len - size);
#endif

  return len;
}

/**
 * [CQ_ManualGet_Offset_Data 读取指定索引号的数据]
 * @param CircularQueue [环形缓冲区句柄]
 * @param index         [索引号]
 */
AT_CQ_CODE_SPACE
uint8_t CQ_ManualGet_Offset_Data(CQ_handleTypeDef *CircularQueue, uint32_t index)
{
  /* 计算偏移 */
  uint32_t read_offset = ((CircularQueue->exit + index) & (CircularQueue->size - 1));
  /* 取出数据 */
  uint8_t data = *((uint8_t*)CircularQueue->Buffer.data8Buffer + read_offset);

  return data;
}

/**
 * [CQ_ManualOffsetInc 手动增加已取出长度]
 * @param CircularQueue [环形缓冲区句柄]
 * @param len           [偏移长度]
 */
AT_CQ_CODE_SPACE
void CQ_ManualOffsetInc(CQ_handleTypeDef *CircularQueue, uint32_t len)
{
  len = GET_MIN(CQ_getLength(CircularQueue), len);
  CircularQueue->exit += len;
}

/**
 * [cb_create 申请并初始化环形缓冲区]
 * @param  buffsize [申请环形缓冲区大小]
 * @return          [环形队列管理句柄]
 */
CQ_handleTypeDef *cb_create(uint32_t buffsize)
{
  if(!IS_POWER_OF_2(buffsize))
  {
    return NULL;
  }

  CQ_handleTypeDef *cb = (CQ_handleTypeDef *)calloc(1, sizeof(CQ_handleTypeDef));
  if(NULL == cb)
  {
    return NULL;
  }
  buffsize = (buffsize <= 2048 ? buffsize : 2048);
  cb->size = buffsize;
  cb->exit = 0;
  cb->entrance = 0;
  //the buff never release!
  cb->Buffer.data8Buffer = (uint8_t *)calloc((size_t)cb->size, sizeof(uint8_t));
  if(NULL == cb->Buffer.data8Buffer)
  {
    return NULL;
  }
  cb->is_malloc = true;
  return cb;
}

/**
 * @brief 删除一个缓冲区
 *
 * @param CircularQueue
 */
void cb_delete(CQ_handleTypeDef *CircularQueue)
{
  if(CircularQueue == NULL)
  {
    return;
  }
  if(CircularQueue->is_malloc == false)
  {
    return;
  }
  free(CircularQueue->Buffer.data8Buffer);
  free(CircularQueue);
}

/**
 * [CQ_16_init 静态初始化16bit环形缓冲区]
 * @param  CircularQueue [缓冲区指针]
 * @param  memAdd        [uint16_t 缓冲区地址]
 * @param  len           [缓冲区长度>1]
 * @return               [初始化状态]
 */
AT_CQ_CODE_SPACE
bool CQ_16_init(CQ_handleTypeDef *CircularQueue, uint16_t *memAdd, uint32_t len)
{
  CircularQueue->size = len;

  if(!IS_POWER_OF_2(CircularQueue->size))
  {
    return FALSE;
  }

  if(memAdd == NULL)
  {
    return FALSE;
  }

  CircularQueue->Buffer.data16Buffer = memAdd;
  CircularQueue->entrance = CircularQueue->exit = 0;
  return TRUE;
}

/**
 * [cb_16create 动态申请并初始化环形缓冲区]
 * @param  buffsize [申请环形缓冲区大小]
 * @return          [环形队列管理句柄]
 */
CQ_handleTypeDef *cb_16create(uint32_t buffsize)
{
  if(!IS_POWER_OF_2(buffsize))
  {
    return NULL;
  }

  CQ_handleTypeDef *cb = (CQ_handleTypeDef *)calloc(1, sizeof(CQ_handleTypeDef));
  if(NULL == cb)
  {
    return NULL;
  }
  buffsize = (buffsize <= 2048 ? buffsize : 2048);
  cb->size = buffsize;
  cb->exit = 0;
  cb->entrance = 0;
  //the buff never release!
  cb->Buffer.data16Buffer = (uint16_t *)calloc((size_t)cb->size, sizeof(uint16_t));
  if(NULL == cb->Buffer.data16Buffer)
  {
    return NULL;
  }
  cb->is_malloc = true;
  return cb;
}

/**
 * [CQ_16getData 取出数据]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  targetBuf     [目标地址]
 * @param  len           [取出长度]
 * @return               [取出长度]
 */
AT_CQ_CODE_SPACE
uint32_t CQ_16getData(CQ_handleTypeDef *CircularQueue, uint16_t *targetBuf, uint32_t len)
{
  uint32_t size = 0;

  /* 此次读取的实际大小，取 可读 和 目标读取数量  两个值小的那个 */
  len = GET_MIN(len, CircularQueue->entrance - CircularQueue->exit);
  /* 原理雷同存入 */
  size = GET_MIN(len, CircularQueue->size - (CircularQueue->exit & (CircularQueue->size - 1)));

  /* 使用DSP加速 */
#if USE_ARM_DSP_FUNC_EN
  arm_copy_q15((q15_t *)(CircularQueue->Buffer.data16Buffer + (CircularQueue->exit & (CircularQueue->size - 1))), (q15_t *)targetBuf, size);
  arm_copy_q15((q15_t *)(CircularQueue->Buffer.data16Buffer), (q15_t *)(targetBuf + size), len - size);
#else
  uint32_t len_temp  = 2 * len;
  uint32_t size_temp = 2 * size;
  memcpy(targetBuf, CircularQueue->Buffer.data16Buffer + (CircularQueue->exit & (CircularQueue->size - 1)), size_temp);
  memcpy(targetBuf + size, CircularQueue->Buffer.data16Buffer, len_temp - size_temp);
#endif

  /* 利用无符号数据的溢出特性 */
  CircularQueue->exit += len;

  return len;
}


/**
 * [CQ_16putData 加入数据]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  sourceBuf     [源地址]
 * @param  len           [长度]
 * @return               [加入数据长度]
 */
AT_CQ_CODE_SPACE
uint32_t CQ_16putData(CQ_handleTypeDef *CircularQueue, const uint16_t *sourceBuf, uint32_t len)
{
  uint32_t size = 0;

  /* 此次存入的实际大小，取 剩余空间 和 目标存入数量  两个值小的那个 */
  len = GET_MIN(len, CircularQueue->size - CircularQueue->entrance + CircularQueue->exit);

  /* &(size-1)代替取模运算，同上原理，得到此次存入队列入口到末尾的大小 */
  size = GET_MIN(len, CircularQueue->size - (CircularQueue->entrance & (CircularQueue->size - 1)));

  /* 使用DSP加速 */
#if USE_ARM_DSP_FUNC_EN
  arm_copy_q15((q15_t *)sourceBuf, (q15_t *)(CircularQueue->Buffer.data16Buffer + (CircularQueue->entrance & (CircularQueue->size - 1))), size);
  arm_copy_q15((q15_t *)(sourceBuf + size), (q15_t *)CircularQueue->Buffer.data16Buffer, len - size);
#else
  uint32_t len_temp  = 2 * len;
  uint32_t size_temp = 2 * size;
  memcpy(CircularQueue->Buffer.data16Buffer + (CircularQueue->entrance & (CircularQueue->size - 1)), sourceBuf, size_temp);
  memcpy(CircularQueue->Buffer.data16Buffer, sourceBuf + size, len_temp - size_temp);
#endif

  /* 利用无符号数据的溢出特性 */
  CircularQueue->entrance += len; //写入数据记录

  return len;
}

/**
 * [CQ_32_init 静态初始化32bit环形缓冲区]
 * @param  CircularQueue [缓冲区指针]
 * @param  memAdd        [uint32_t 缓冲区地址]
 * @param  len           [缓冲区长度>1]
 * @return               [初始化状态]
 */
AT_CQ_CODE_SPACE
bool CQ_32_init(CQ_handleTypeDef *CircularQueue, uint32_t *memAdd, uint32_t len)
{
  CircularQueue->size = len;

  if(!IS_POWER_OF_2(CircularQueue->size))
  {
    return FALSE;
  }

  if(memAdd == NULL)
  {
    return FALSE;
  }

  CircularQueue->Buffer.data32Buffer = memAdd;

  memset(CircularQueue->Buffer.data32Buffer, 0, len*4);
  CircularQueue->entrance = CircularQueue->exit = 0;

  return TRUE;
}

/**
 * [cb_32create 动态申请并初始化环形缓冲区]
 * @param  buffsize [申请环形缓冲区大小]
 * @return          [环形队列管理句柄]
 */
CQ_handleTypeDef *cb_32create(uint32_t buffsize)
{
  if(!IS_POWER_OF_2(buffsize))
  {
    return NULL;
  }

  CQ_handleTypeDef *cb = (CQ_handleTypeDef *)calloc(1, sizeof(CQ_handleTypeDef));
  if(NULL == cb)
  {
    return NULL;
  }
  buffsize = (buffsize <= 2048 ? buffsize : 2048);
  cb->size = buffsize;
  cb->exit = 0;
  cb->entrance = 0;

  cb->Buffer.data32Buffer = (uint32_t *)calloc((size_t)cb->size, sizeof(uint32_t));
  if(NULL == cb->Buffer.data32Buffer)
  {
    return NULL;
  }
  cb->is_malloc = true;
  return cb;
}

/**
 * [CQ_32putData 加入数据]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  sourceBuf     [源地址]
 * @param  len           [长度]
 * @return               [加入数据长度]
 */
AT_CQ_CODE_SPACE
uint32_t CQ_32putData(CQ_handleTypeDef *CircularQueue, const uint32_t * sourceBuf, uint32_t len)
{
  uint32_t size = 0;

  /* 此次存入的实际大小，取 剩余空间 和 目标存入数量  两个值小的那个 */
  len = GET_MIN(len, CircularQueue->size - CircularQueue->entrance + CircularQueue->exit);

  /* &(size-1)代替取模运算，同上原理，得到此次存入队列入口到末尾的大小 */
  size = GET_MIN(len, CircularQueue->size - (CircularQueue->entrance & (CircularQueue->size - 1)));

  /* 使用DSP加速 */
#if USE_ARM_DSP_FUNC_EN
  arm_copy_q31((q31_t *)sourceBuf, (q31_t *)(CircularQueue->Buffer.data32Buffer + (CircularQueue->entrance & (CircularQueue->size - 1))), size);
  arm_copy_q31((q31_t *)(sourceBuf + size), (q31_t *)(CircularQueue->Buffer.data32Buffer), len - size);
#else
  uint32_t len_temp  = 4 * len;
  uint32_t size_temp = 4 * size;
  memcpy(CircularQueue->Buffer.data32Buffer + (CircularQueue->entrance & (CircularQueue->size - 1)), sourceBuf, size_temp);
  memcpy(CircularQueue->Buffer.data32Buffer, sourceBuf + size, len_temp - size_temp);
#endif

  /* 利用无符号数据的溢出特性 */
  CircularQueue->entrance += len;

  return len;
}

/**
 * [CQ_32getData 取出数据]
 * @param  CircularQueue [环形缓冲区句柄]
 * @param  targetBuf     [目标地址]
 * @param  len           [取出长度]
 * @return               [取出长度]
 */
AT_CQ_CODE_SPACE
uint32_t CQ_32getData(CQ_handleTypeDef *CircularQueue, uint32_t *targetBuf, uint32_t len)
{
  uint32_t size = 0;

  /* 此次读取的实际大小，取 可读 和 目标读取数量  两个值小的那个 */
  len = GET_MIN(len, CircularQueue->entrance - CircularQueue->exit);
  /* 原理雷同存入 */
  size = GET_MIN(len, CircularQueue->size - (CircularQueue->exit & (CircularQueue->size - 1)));

  /* 使用DSP加速 */
#if USE_ARM_DSP_FUNC_EN
  arm_copy_q31((q31_t *)(CircularQueue->Buffer.data32Buffer + (CircularQueue->exit & (CircularQueue->size - 1))), (q31_t *)targetBuf, size);
  arm_copy_q31((q31_t *)(CircularQueue->Buffer.data32Buffer), (q31_t *)(targetBuf + size), len - size);
#else
  uint32_t len_temp  = 4 * len;
  uint32_t size_temp = 4 * size;
  memcpy(targetBuf, CircularQueue->Buffer.data32Buffer + (CircularQueue->exit & (CircularQueue->size - 1)), size_temp);
  memcpy(targetBuf + size, CircularQueue->Buffer.data32Buffer, len_temp - size_temp);
#endif

  /* 利用无符号数据的溢出特性 */
  CircularQueue->exit += len;

  return len;
}

#if ENABLE_CQ_DEBUG_PRINTF
  /**
   * @brief 调试打印输出
   *
   * @param CircularQueue
   */
  AT_CQ_CODE_SPACE
  void CQ_DEBUG_Printf(const CQ_handleTypeDef *CircularQueue)
  {
    if(CircularQueue == NULL)
    {
      return;
    }
    printf("CircularQueue->size = %u\n", CircularQueue->size);
    printf("CircularQueue->entrance = %u\n", CircularQueue->entrance);
    printf("CircularQueue->exit = %u\n", CircularQueue->exit);
    printf("CircularQueue->Buffer.data32Buffer = %p\n", CircularQueue->Buffer.data32Buffer);
  }
#endif

#ifdef __cplusplus ///<end extern c
}
#endif
/******************************** End of file *********************************/
