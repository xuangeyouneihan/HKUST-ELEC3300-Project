/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v2.0_Cube
  * @brief          : Usb device for Virtual Com Port.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */
#include <stdbool.h>
#include "main.h"
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */

// 放到了 usbd_cdc_if.h 里

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */

// ANCHOR
// uint8_t jsonBuffer[MAX_JSON_SIZE];
// int32_t jsonBufferIndex = 0; // current position where to write the next data
// static bool jsonReceiving = false;

uint8_t infoBuffer[MAX_INFO_SIZE];
int32_t infoBufferIndex = 0;
uint8_t charBuffer[MAX_CHAR_SIZE];
int32_t charBufferIndex = 0;
bool infoReceived = false; // 当收到 infoBuffer 完整数据后置 true
bool charReceived = false; // 当收到 charBuffer 完整数据后置 true

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

    case CDC_SET_COMM_FEATURE:

    break;

    case CDC_GET_COMM_FEATURE:

    break;

    case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING:

    break;

    case CDC_GET_LINE_CODING:

    break;

    case CDC_SET_CONTROL_LINE_STATE:

    break;

    case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */

  // return (USBD_OK);
  // CDC_Transmit_FS(Buf, *Len);
  // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
  // HAL_Delay(1);
  // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);

  // 如果正在进行写操作，则不处理新数据
  if (infoReceived && charReceived)
  {
    return USBD_BUSY;
  }

  // 正常检查缓冲区空间（此处使用 MAX_JSON_SIZE 作为上限）
  // 注意：这里只做简化示例，实际情况请考虑分开 infoBuffer 与 charBuffer 的错误处理
  if (!infoReceived)
  {
    // 处于 infoBuffer 收集阶段
    if (infoBufferIndex + *Len < MAX_INFO_SIZE)
    {
      memcpy(infoBuffer + infoBufferIndex, Buf, *Len);
      infoBufferIndex += *Len;
    }
    else // Anchor
    {
      // infoBuffer 溢出，重置状态
      infoBufferIndex = 0;
      infoReceived = false;
      charBufferIndex = 0;  // Fix?
      charReceived = false; // Fix? : cahrReceived was never reset in overflow
      const char *error = "{Buffer overflow}";
      CDC_Transmit_FS((uint8_t *)error, strlen(error));
      goto prepare_next;
    }
    // 判断是否为终止段（长度不足 SEGMENT_SIZE）
    if ((*Len) < SEGMENT_SIZE)
    {
      // 如果终止段的首个字节为 '\0'
      // 则认为该段无有效数据，所以不必重复追加（取消这部分数据）
      if (Buf[0] == 0)
      {
        // 撤销这段追加的数据：将 infoBufferIndex 回退
        infoBufferIndex -= *Len;
      }
      // 对方接下来会发送 characters 数据
      // 添加结束符（如果空间允许）
      if (infoBufferIndex < MAX_INFO_SIZE)
        infoBuffer[infoBufferIndex] = '\0';
      infoReceived = true;
    }
  }
  else
  {
    // 处于接收 characters 数据阶段
    if (charBufferIndex + *Len < MAX_CHAR_SIZE)
    {
      memcpy(charBuffer + charBufferIndex, Buf, *Len);
      charBufferIndex += *Len;
    }
    else // Anchor
    {
      // charBuffer 溢出，重置状态
      charBufferIndex = 0;
      charReceived = false;
      infoBufferIndex = 0;  // Fix?
      infoReceived = false; // Fix? : infoReceived was never reset in overflow
      const char *error = "{Buffer overflow}";
      CDC_Transmit_FS((uint8_t *)error, strlen(error));
      goto prepare_next;
    }
    // 判断是否为终止段（长度不足 SEGMENT_SIZE）
    if ((*Len) < SEGMENT_SIZE)
    {
      // 如果终止段的首个字节为 '\0'
      // 则认为该段无有效数据，所以不必重复追加（取消这部分数据）
      if (Buf[0] == 0)
      {
        // 撤销这段追加的数据：将 charBufferIndex 回退
        charBufferIndex -= *Len;
      }
      char signal[(*Len) + 1];
      memcpy(signal, Buf, *Len);
      signal[*Len] = '\0';
      if (strcmp(signal, FINISH) == 0) // Anchor
      {
        infoReceived = false;
        charReceived = false; // Fix? : cahrReceived was never reset after FINISH flag
        charBufferIndex = 0;  // Fix?
        infoBufferIndex = 0;  // Fix?
      }
      // 对方接下来会发送下一个 character 数据
      // 添加结束符（如果空间允许）w
      if (charBufferIndex < MAX_CHAR_SIZE)
        charBuffer[charBufferIndex] = '\0';
      charReceived = true;
    }
  }

prepare_next:
  // 准备接收下一数据块
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, Buf);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  // 此处可选择发送当前状态或缓冲区内容用于调试
  // CDC_Transmit_FS(jsonBuffer, infoBufferIndex);
  return USBD_OK;

  // if (writing)
  // {
  //   return USBD_OK;
  // }
  // if (!jsonReceiving)
  // {
  //   jsonReceiving = true;
  // }
  // // 更新最后一次接收时间（超时判断可在后台任务中使用）
  // // lastPacketTick = currentTick;  // 如果需要超时机制，可用全局变量保存

  // // 将接收到的数据追加到全局 jsonBuffer 中
  // if (jsonBufferIndex + *Len < MAX_JSON_SIZE)
  // {
  //   memcpy(jsonBuffer + jsonBufferIndex, Buf, *Len);
  //   jsonBufferIndex += *Len;
  // }
  // else
  // {
  //   // 若缓冲区溢出，则重置状态
  //   jsonBufferIndex = 0;
  //   jsonReceiving = false;
  //   const char *error = "{Buffer overflow}";
  //   CDC_Transmit_FS((uint8_t *)error, strlen(error));
  //   goto prepare_next;
  // }

  // // 判断当前段是否为终止段
  // // 条件: 当前段不足32字节（说明是最后一段）
  // if ((*Len) < SEGMENT_SIZE)
  // {
  //   // 如果终止段的首个字节为 '\0'
  //   // 则认为该段无有效数据，所以不必重复追加（取消这部分数据）
  //   if (Buf[0] == 0)
  //   {
  //     // 撤销这段追加的数据：将 jsonBufferIndex 回退
  //     jsonBufferIndex -= *Len;
  //   }
  //   // 添加结束符（如果空间允许）
  //   if (jsonBufferIndex < MAX_JSON_SIZE)
  //     jsonBuffer[jsonBufferIndex] = '\0';
  //   // // 以下这段可能会有很大问题
  //   // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
  //   // HAL_Delay(1000);
  //   // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
  //   // // 发送并处理完整数据
  //   // processRpcRequest(jsonBuffer, jsonBufferIndex);
  //   // 修改状态
  //   writing = true;
  //   // CDC_Transmit_FS(jsonBuffer, jsonBufferIndex);
  //   jsonReceiving = false;
  // }

// prepare_next:
//   // 准备接收下一数据块
//   USBD_CDC_SetRxBuffer(&hUsbDeviceFS, Buf);
//   USBD_CDC_ReceivePacket(&hUsbDeviceFS);
//   CDC_Transmit_FS(jsonBuffer, jsonBufferIndex);
//   return USBD_OK;
  /* USER CODE END 6 */
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  /* USER CODE END 7 */
  return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
