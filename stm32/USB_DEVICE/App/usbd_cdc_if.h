/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.h
  * @version        : v2.0_Cube
  * @brief          : Header for usbd_cdc_if.c file.
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

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __USBD_CDC_IF_H__
#define __USBD_CDC_IF_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc.h"

/* USER CODE BEGIN INCLUDE */
#include <stdbool.h>
/* USER CODE END INCLUDE */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief For Usb device.
  * @{
  */

/** @defgroup USBD_CDC_IF USBD_CDC_IF
  * @brief Usb VCP device module
  * @{
  */

/** @defgroup USBD_CDC_IF_Exported_Defines USBD_CDC_IF_Exported_Defines
  * @brief Defines.
  * @{
  */
/* Define size for the receive and transmit buffer over CDC */
#define APP_RX_DATA_SIZE  1024
#define APP_TX_DATA_SIZE  1024
/* USER CODE BEGIN EXPORTED_DEFINES */
#define MAX_JSON_SIZE 16384
#define SEGMENT_SIZE 32
#define JSON_TIMEOUT 100
#define MAX_INFO_SIZE 256
#define MAX_CHAR_SIZE 1024
#define READY "114514"
#define FINISH "1919810"
/* USER CODE END EXPORTED_DEFINES */

/**
* @}
*/

/** @defgroup USBD_CDC_IF_Exported_Types USBD_CDC_IF_Exported_Types
* @brief Types.
* @{
*/

/* USER CODE BEGIN EXPORTED_TYPES */

/* USER CODE END EXPORTED_TYPES */

/**
* @}
*/

/** @defgroup USBD_CDC_IF_Exported_Macros USBD_CDC_IF_Exported_Macros
* @brief Aliases.
* @{
*/

/* USER CODE BEGIN EXPORTED_MACRO */

/* USER CODE END EXPORTED_MACRO */

/**
* @}
*/

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
* @brief Public variables.
* @{
*/

/** CDC Interface callback. */
extern USBD_CDC_ItfTypeDef USBD_Interface_fops_FS;

/* USER CODE BEGIN EXPORTED_VARIABLES */
// extern uint8_t jsonBuffer[MAX_JSON_SIZE];
// extern int32_t jsonBufferIndex;
// extern bool writing;
uint8_t infoBuffer[MAX_INFO_SIZE];
int32_t infoBufferIndex;
uint8_t charBuffer[MAX_CHAR_SIZE];
int32_t charBufferIndex;
bool infoReceived; // 当收到 infoBuffer 完整数据后置 true
bool charReceived; // 当收到 charBuffer 完整数据后置 true
/* USER CODE END EXPORTED_VARIABLES */

/**
* @}
*/

/** @defgroup USBD_CDC_IF_Exported_FunctionsPrototype USBD_CDC_IF_Exported_FunctionsPrototype
* @brief Public functions declaration.
* @{
*/

uint8_t CDC_Transmit_FS(uint8_t *Buf, uint16_t Len);

/* USER CODE BEGIN EXPORTED_FUNCTIONS */

/* USER CODE END EXPORTED_FUNCTIONS */

/**
* @}
*/

/**
* @}
*/

/**
* @}
*/

#ifdef __cplusplus
 }
#endif

#endif /* __USBD_CDC_IF_H__ */

