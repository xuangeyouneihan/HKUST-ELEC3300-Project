/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "usbd_cdc_if.h"
#include "cJSON.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

// Data structures for character information
// Data hierarchy:
// Document -> Segment -> Character -> Stroke -> Point
struct Point
{
  float x;
  float y;
}; // Point structure to represent coordinates

struct Stroke
{
  struct Point *points;
  int pointCount;
}; // Stroke structure to represent a series of points

struct Character
{
  bool is_line_feed;
  float advance_width;
  float left_side_bearing;
  struct Stroke *strokes;
  int strokeCount;
}; // Character structure to represent a character with its strokes

struct Segment
{
  float ascender;
  float descender;
  float line_gap;
  float paragraph_spacing;
  struct Character *characters;
  int characterCount;
}; // Segment structure to represent a segment of text with its characters

struct Document
{
  float page_width;
  float page_height;
  float top_margin;
  float bottom_margin;
  float left_margin;
  float right_margin;
  struct Segment *segments;
  int segmentCount;
}; // Document structure to represent a document with its segments

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// Motor directions
#define MOTOR_L_DIR GPIO_PIN_10 // B10
#define MOTOR_L_STEP GPIO_PIN_2 // A2
#define MOTOR_R_DIR GPIO_PIN_9  // B9
#define MOTOR_R_STEP GPIO_PIN_3 // A3
#define MOTOR_Z_DIR GPIO_PIN_12 // B12
#define MOTOR_Z_STEP GPIO_PIN_8 // A8

#define DIR_CW GPIO_PIN_SET
#define DIR_CCW GPIO_PIN_RESET
// #define DIR_STOP

#define DFLT_STEP_T 1
#define DFLT_STEP_FREQ 1000
#define DFLT_STEP_CYCLE 50

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

int32_t Global_X = 0;
int32_t Global_Y = 0;
// may use
// int32_t Relative_X = 0;
// int32_t Relative_Y = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void penup();
void pendown();
void legacyMove(uint8_t direction, uint32_t time, uint8_t draw);

void moveToXY(float delta_X, float delta_Y);
void moveAngle(float distance, float angle);
void moveToAbsoluteXY(int32_t target_X, int32_t target_Y);
void motorControl(int32_t delta_MotorL, int32_t delta_MotorR);
void updateGlobalXY(float delta_X, float delta_Y);

void drawDocument(struct Document *document);
void drawSegment(struct Segment *segment, float *Global_X, float *Global_Y, float page_width, float page_height, float top_margin, float bottom_margin, float left_margin, float right_margin, float scale);
void drawCharacter(struct Character *character, float startX, float startY, float scale);
void drawStroke(struct Stroke *stroke);

void freeAllData(struct Document *document);
void freeDocument(struct Document *document);
void freeSegment(struct Segment *segment);
void freeCharacter(struct Character *character);
void freeStroke(struct Stroke *stroke);

bool initDocument(struct Document *document, float page_width, float page_height, float top_margin, float bottom_margin, float left_margin, float right_margin, struct Segment *segments, int segmentCount);
bool initSegment(struct Segment *segment, float ascender, float descender, float line_gap, float paragraph_spacing, struct Character *characters, int characterCount);
bool initCharacter(struct Character *character, bool is_line_feed, float advance_width, float left_side_bearing, struct Stroke *strokes, int strokeCount);
bool initStroke(struct Stroke *stroke, struct Point *points, int pointCount);

void drawFu();
void drawCircle();
void drawRegularPentagon();
void drawOneStrokeFromDoc();

bool parseDocument(struct Document *document, cJSON *json);
bool parseSegment(struct Segment *segment, cJSON *json);
bool parseCharacter(struct Character *character, cJSON *json);
void processRpcRequest(uint8_t *buffer, uint16_t length);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET); // enable
  moveToXY(100, 0);
  moveToXY(-100, 0);
  moveToXY(0, 100);
  moveToXY(0, -100);
  penup();
  pendown();
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET); // disable
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0))
    {
      drawFu();
    }
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13))
    {
      drawCircle();
      drawRegularPentagon();
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief TIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 1999;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 71;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 36;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);
}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 71;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 36;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB12 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

//
// Discrete stepper motor control
//

void drawFu()
{
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET); // enable

  // legacyMove(3, 3000, 1);
  moveAngle(300, 315);
  HAL_Delay(50);
  // legacyMove(5, 3000, 1);
  moveAngle(300, 225);
  HAL_Delay(50);
  // legacyMove(7, 3000, 1);
  moveAngle(300, 135);
  HAL_Delay(50);
  // legacyMove(1, 3000, 1);
  moveAngle(300, 45);
  HAL_Delay(50);

  penup();
  HAL_Delay(50);
  // legacyMove(4, 871, 0);
  moveAngle(174, 270);
  HAL_Delay(50);
  // legacyMove(6, 400, 0);
  moveAngle(80, 180);
  HAL_Delay(50);

  pendown();
  HAL_Delay(50);
  // legacyMove(2, 300, 1);
  moveAngle(60, 0);
  HAL_Delay(50);

  penup();
  HAL_Delay(50);
  // legacyMove(4, 200, 0);
  moveAngle(40, 270);
  HAL_Delay(50);
  // legacyMove(6, 450, 0);
  moveAngle(90, 180);
  HAL_Delay(50);

  pendown();
  HAL_Delay(50);
  // legacyMove(2, 500, 1);
  moveAngle(100, 0);
  HAL_Delay(50);

  penup();
  HAL_Delay(50);
  // legacyMove(6, 200, 0);
  moveAngle(40, 180);
  HAL_Delay(50);

  pendown();
  HAL_Delay(50);
  // legacyMove(4, 900, 1);
  moveAngle(180, 270);
  HAL_Delay(50);

  penup();
  HAL_Delay(50);
  // legacyMove(0, 650, 0);
  moveAngle(130, 90);
  HAL_Delay(50);
  // legacyMove(6, 150, 0);
  moveAngle(50, 180);
  HAL_Delay(50);

  pendown();
  HAL_Delay(50);
  // legacyMove(4, 350, 1);
  moveAngle(70, 270);
  HAL_Delay(50);

  penup();
  HAL_Delay(50);
  // legacyMove(0, 350, 0);
  moveAngle(70, 90);
  HAL_Delay(50);
  // legacyMove(2, 300, 0);
  moveAngle(60, 0);
  HAL_Delay(50);

  pendown();
  HAL_Delay(50);
  // legacyMove(4, 350, 1);
  moveAngle(70, 270);
  HAL_Delay(50);

  penup();
  HAL_Delay(50);
  // legacyMove(2, 200, 0);
  moveAngle(40, 0);
  HAL_Delay(50);
  // legacyMove(0, 825, 0);
  moveAngle(165, 90);
  HAL_Delay(50);

  pendown();
  HAL_Delay(50);
  // legacyMove(2, 400, 1);
  moveAngle(80, 0);
  HAL_Delay(50);

  penup();
  HAL_Delay(50);
  // legacyMove(6, 50, 0);
  moveAngle(10, 180);
  HAL_Delay(50);
  // legacyMove(4, 150, 0);
  moveAngle(30, 270);
  HAL_Delay(50);

  pendown();
  HAL_Delay(50);
  // legacyMove(4, 200, 1);
  moveAngle(40, 270);
  HAL_Delay(50);
  // legacyMove(6, 300, 1);
  moveAngle(60, 180);
  HAL_Delay(50);
  // legacyMove(0, 200, 1);
  moveAngle(40, 90);
  HAL_Delay(50);
  // legacyMove(2, 300, 1);
  moveAngle(60, 0);
  HAL_Delay(50);

  penup();
  HAL_Delay(50);
  // legacyMove(4, 350, 0);
  moveAngle(70, 270);
  HAL_Delay(50);
  // legacyMove(2, 100, 0);
  moveAngle(20, 0);
  HAL_Delay(150);

  pendown();
  HAL_Delay(50);
  // legacyMove(4, 500, 1);
  moveAngle(100, 270);
  HAL_Delay(50);
  // legacyMove(6, 500, 1);
  moveAngle(100, 180);
  HAL_Delay(50);
  // legacyMove(0, 500, 1);
  moveAngle(100, 90);
  HAL_Delay(50);
  // legacyMove(2, 500, 1);
  moveAngle(100, 0);
  HAL_Delay(50);

  penup();
  HAL_Delay(50);
  // legacyMove(6, 500, 0);
  moveAngle(100, 180);
  HAL_Delay(50);
  // legacyMove(4, 225, 0);
  moveAngle(45, 270);
  HAL_Delay(50);

  pendown();
  HAL_Delay(50);
  // legacyMove(2, 500, 1);
  moveAngle(100, 0);
  HAL_Delay(50);

  penup();
  HAL_Delay(50);
  // legacyMove(0, 225, 0);
  moveAngle(45, 90);
  HAL_Delay(50);
  // legacyMove(6, 225, 0);
  moveAngle(45, 180);
  HAL_Delay(50);

  pendown();
  HAL_Delay(50);
  // legacyMove(4, 500, 1);
  moveAngle(100, 270);

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET); // disable
}

void drawCircle()
{
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET); // enable

  for (int i = 0; i < 180; i++)
  {
    if (!(i % 30))
    {
      HAL_GPIO_WritePin(GPIOB, MOTOR_Z_DIR, GPIO_PIN_SET);
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
      HAL_Delay(5);
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    }
    moveAngle(5, i * 2);
  }

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET); // disable
}

void drawRegularPentagon()
{
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET); // enable

  for (int i = 0; i < 5; i++)
  {
    HAL_GPIO_WritePin(GPIOB, MOTOR_Z_DIR, GPIO_PIN_SET);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_Delay(5);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    moveAngle(160, (float)i * 360 / 5);
  }

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET); // disable
}

void penup()
{
  HAL_GPIO_WritePin(GPIOB, MOTOR_Z_DIR, GPIO_PIN_RESET);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_Delay(100);
  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
}

void pendown()
{
  HAL_GPIO_WritePin(GPIOB, MOTOR_Z_DIR, GPIO_PIN_SET);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_Delay(100);
  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
}

void legacyMove(uint8_t direction, uint32_t time, uint8_t draw)
{
  uint32_t tick = HAL_GetTick();
  if (draw)
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
    while (HAL_GetTick() - tick < 5)
    {
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_11);
      HAL_Delay(1);
    }
  }
  switch (direction)
  {
  // 上
  case 0:
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    while (HAL_GetTick() - tick < time + 50)
    {
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
      HAL_Delay(1);
    }
    break;

  // 右上 1
  case 1:
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    while (HAL_GetTick() - tick < time + 50)
    {
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
      HAL_Delay(1);
    }
    break;

  // 右
  case 2:
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    while (HAL_GetTick() - tick < time + 50)
    {
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
      HAL_Delay(1);
    }
    break;

  // 右下 1
  case 3:
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    while (HAL_GetTick() - tick < time + 50)
    {
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
      HAL_Delay(1);
    }
    break;

  // 下
  case 4:
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    while (HAL_GetTick() - tick < time + 50)
    {
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
      HAL_Delay(1);
    }
    break;

  // 左下
  case 5:
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    while (HAL_GetTick() - tick < time + 50)
    {
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
      HAL_Delay(1);
    }
    break;

  // 左
  case 6:
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    while (HAL_GetTick() - tick < time + 50)
    {
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
      HAL_Delay(1);
    }
    break;

  // 左上
  case 7:
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    while (HAL_GetTick() - tick < time + 50)
    {
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
      HAL_Delay(1);
    }
    break;
  }
}

//
// Quick search:
// TODO; CAUTION; PROBLEM
// WARNING; BAD
//

//
// Continuous (stepping) motor controls
//

// WARNING:
//  float are supported, but steps and Global Pos are int32_t, relative error will accumulate
void moveToXY(float delta_X, float delta_Y)
{
  // quantize to steps (default scenrio: L = CCW, R = STOP)
  int32_t delta_MotorL = (int32_t)(delta_X + delta_Y);
  int32_t delta_MotorR = (int32_t)(delta_X - delta_Y);

  motorControl(delta_MotorL, delta_MotorR);

  updateGlobalXY(delta_X, delta_Y);
}

void moveAngle(float distance, float angle)
{
  float delta_X = distance * cos(angle * M_PI / 180.0f);
  float delta_Y = distance * sin(angle * M_PI / 180.0f);

  moveToXY(delta_X, delta_Y);
}

void moveToAbsoluteXY(int32_t target_X, int32_t target_Y)
{
  // calculate deltas
  float delta_X = target_X - Global_X;
  float delta_Y = target_Y - Global_Y;

  moveToXY(delta_X, delta_Y);
}

void motorControl(int32_t delta_MotorL, int32_t delta_MotorR)
{
  // motor directions
  int8_t dirL = (delta_MotorL > 0) ? DIR_CW : DIR_CCW;
  int8_t dirR = (delta_MotorR > 0) ? DIR_CW : DIR_CCW;
  HAL_GPIO_WritePin(GPIOB, MOTOR_L_DIR, dirL);
  HAL_GPIO_WritePin(GPIOB, MOTOR_R_DIR, dirR);

  // absolute steps count // to determine which axis for base step
  int32_t stepsL = (delta_MotorL > 0) ? delta_MotorL : -delta_MotorL;
  int32_t stepsR = (delta_MotorR > 0) ? delta_MotorR : -delta_MotorR;
  int32_t maxSteps = (stepsL > stepsR) ? stepsL : stepsR;
  if (stepsL == 0 && stepsR == 0)
    return; // no movement

  // Activate PWM channels for hardware pulse generation
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3); // motorL
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4); // motorR
  // start with 0% duty cycle
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 0);

  //
  int32_t remainingStepsL = stepsL;
  int32_t remainingStepsR = stepsR;

  // Bresenham's line algorithm
  // Basic:
  // move using unit-length of the longer axis, then apply change of the other axis with error accumulation.
  // set unit-length of the longer axis to aviod sharp jumps.
  // Mutation:
  // half steps for each and have the algo in both motor, error threshold is maxSteps.
  // check threshold for each, apply change and sutract error if threshold is reached.
  // increment absolute steps into error each iteration.
  int32_t error_L = maxSteps / 2;
  int32_t error_R = maxSteps / 2;

  for (int i = 0; i < maxSteps; i++)
  {
    bool motorL_working = false;
    bool motorR_working = false;

    // left motor should step?
    if (error_L >= stepsL)
    {
      if (remainingStepsL > 0)
      {
        // Lagacy: delay stepping
        // HAL_GPIO_WritePin(GPIOB, MOTOR_L_STEP, GPIO_PIN_SET);
        // HAL_Delay(DFLT_STEP_T);
        // HAL_GPIO_WritePin(GPIOB, MOTOR_L_STEP, GPIO_PIN_RESET);

        // PWM stepping
        motorL_working = true;
        remainingStepsL--;
      }
      error_L -= maxSteps;
    }
    // right motor should step?
    if (error_R >= stepsR)
    {
      if (remainingStepsR > 0)
      {
        // Lagacy: delay stepping
        // HAL_GPIO_WritePin(GPIOB, MOTOR_R_STEP, GPIO_PIN_SET);
        // HAL_Delay(DFLT_STEP_T);
        // HAL_GPIO_WritePin(GPIOB, MOTOR_R_STEP, GPIO_PIN_RESET);

        // PWM stepping
        motorR_working = true;
        remainingStepsR--;
      }
      error_R -= maxSteps;
    }
    // update error
    error_L += stepsL;
    error_R += stepsR;

    // PWM stepping
    if (motorL_working)
    {
      // PWM 50% (72)
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 36);
    }
    else
    {
      //  PWM 0%
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);
    }

    if (motorR_working)
    {
      // PWM 50% (72)
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 36);
    }
    else
    {
      //  PWM 0%
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 0);
    }

    HAL_Delay(1);
  }

  // stop PWM channels
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 0);
  HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3);
  HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_4);
}

// WARNING:
// deltas can be floats, steps and Global positions are int32_t
// steps are time dependent, overtime relative error will accumulate
void updateGlobalXY(float delta_X, float delta_Y)
{
  Global_X += (int32_t)delta_X;
  Global_Y += (int32_t)delta_Y;
}

//
// document handling
//
// work flow:
// 1. Document: Starting at page margins and working through segments
// 2. Segment: Group up characters
// 3. Character: Calculate starting position, Set up to strokes
// 4. Stroke: Move through subsequent points
//
// amement: page function discarded, there is no auto paper feeder for that
//

// PROBLEM:
// The coordinate systems is not sattled yet.
// having negative Y or not affects greatly on line changing.
// where the origin is, to be defined

void drawDocument(struct Document *document)
{
  // set the starting position at the top left corner of the page
  float Global_X = document->left_margin;
  float Global_Y = -(document->top_margin);
  float scale = 0.2; // 1 step = 0.2 mm

  // iterate through each segment in the document
  for (int i = 0; i < document->segmentCount; i++)
  {
    struct Segment *segment = &document->segments[i];

    // check if the segment is empty
    if (segment->characterCount > 0)
    {
      // draw the segment at the current position
      drawSegment(segment, &Global_X, &Global_Y, document->page_width, document->page_height, document->top_margin, document->bottom_margin, document->left_margin, document->right_margin, scale);
    }
  }
}

void drawSegment(struct Segment *segment, float *Global_X,
                 float *Global_Y, float page_width,
                 float page_height, float top_margin,
                 float bottom_margin, float left_margin,
                 float right_margin, float scale)
{
  float current_X = *Global_X;
  float current_Y = *Global_Y - segment->ascender;

  for (int i = 0; i < segment->characterCount; i++)
  {
    struct Character *character = &segment->characters[i];

    // check if a new line is needed
    if (current_X + character->advance_width >= page_width - right_margin || character->is_line_feed)
    {
      current_X = left_margin;                                                   // reset X position
      current_Y -= (segment->ascender + segment->descender + segment->line_gap); // move to next line
    }
    else
    {
      // draw the character at the current position
      drawCharacter(character, current_X, current_Y, scale);
      // update the current X position for the next character
      current_X += character->advance_width;
    }
  }

  current_Y += segment->ascender;
  // update global XY
  *Global_X = current_X;
  *Global_Y = current_Y;
  // TODO: detect if the current position exceeds the bottom edge
}

void drawCharacter(struct Character *character, float startX, float startY, float scale)
{
  // get the relative starting position of the character
  float CharStartX = startX + character->left_side_bearing;

  // draw the strokes of the character
  for (int i = 0; i < character->strokeCount; i++)
  {
    struct Stroke *stroke = &character->strokes[i];
    // adjust next stroke position
    for (int j = 0; j < stroke->pointCount; j++)
    {
      stroke->points[j].x = (stroke->points[j].x + CharStartX) * scale;
      stroke->points[j].y = (stroke->points[j].y + startY) * scale;
    }
    drawStroke(stroke);
    // reset the points to the original position
    for (int j = 0; j < stroke->pointCount; j++)
    {
      stroke->points[j].x = (stroke->points[j].x / scale) - CharStartX;
      stroke->points[j].y = (stroke->points[j].y / scale) - startY;
    }
  }
}

// Strokes are one continuous line, quantized to points
void drawStroke(struct Stroke *stroke)
{
  if (stroke->pointCount <= 0)
    return; // No points to draw

  // move to the first point
  moveToAbsoluteXY(stroke->points[0].x, stroke->points[0].y);
  pendown();

  // draw the strokes
  for (int i = 1; i < stroke->pointCount; i++)
  {
    moveToAbsoluteXY(stroke->points[i].x, stroke->points[i].y);
  }
  penup();
}

//
// data handling: free, create, add
//

// Free: Document -> Segment -> Character -> Stroke -> Points

// free all data
void freeAllData(struct Document *document)
{
  if (document)
  {
    freeDocument(document); // Free the document and its segments
    free(document);         // Free the document itself
  }
}

// free document memory
void freeDocument(struct Document *document)
{
  if (document && document->segments)
  {
    for (int i = 0; i < document->segmentCount; i++)
    {
      freeSegment(&document->segments[i]); // Free each segment
    }
    free(document->segments); // Free the segments array
    document->segments = NULL;
  }
}

// free Sergment memory
void freeSegment(struct Segment *segment)
{
  if (segment && segment->characters)
  {
    for (int i = 0; i < segment->characterCount; i++)
    {
      freeCharacter(&segment->characters[i]); // Free each character
    }
    free(segment->characters); // Free the characters array
    segment->characters = NULL;
  }
}

// free character memory
void freeCharacter(struct Character *character)
{
  if (character && character->strokes)
  {
    for (int i = 0; i < character->strokeCount; i++)
    {
      freeStroke(&character->strokes[i]); // Free each stroke
    }
    free(character->strokes); // Free the strokes array
    character->strokes = NULL;
  }
}

// free strokes and point memory
void freeStroke(struct Stroke *stroke)
{
  if (stroke && stroke->points)
  {                       // && to first check stroke, aviod segmentation fault
    free(stroke->points); // Free the points array (no more pointers, should be done)
    stroke->points = NULL;
  }
}

// init the components:

bool initDocument(struct Document *document, float page_width,
                  float page_height, float top_margin,
                  float bottom_margin, float left_margin,
                  float right_margin, struct Segment *segments,
                  int segmentCount)
{
  if (document == NULL)
    return false;

  document->page_width = page_width;
  document->page_height = page_height;
  document->top_margin = top_margin;
  document->bottom_margin = bottom_margin;
  document->left_margin = left_margin;
  document->right_margin = right_margin;
  document->segmentCount = segmentCount;
  // if content is given and segments == NULL,
  // create the segments array with segmentCount,
  // then use AddSegmentToDocument() to later add segments
  if (segmentCount > 0 && segments == NULL)
  {
    document->segments = (struct Segment *)malloc(sizeof(struct Segment) * segmentCount);
    if (document->segments == NULL)
    {
      return false;
    } // alloc failed case
    memset(document->segments, 0, sizeof(struct Segment) * segmentCount); // init mem to zero
  }
  else if (segmentCount > 0 && segments != NULL) // 直接将传入的segments当作segments数组
  {
    document->segments = segments;
  }
  else
  {
    document->segments = NULL;
  }

  return true;
}

bool initSegment(struct Segment *segment, float ascender,
                 float descender, float line_gap,
                 float paragraph_spacing, struct Character *characters,
                 int characterCount)
{
  if (segment == NULL)
    return false;

  segment->ascender = ascender;
  segment->descender = descender;
  segment->line_gap = line_gap;
  segment->paragraph_spacing = paragraph_spacing;
  segment->characterCount = characterCount;

  if (characterCount > 0 && characters == NULL)
  {
    segment->characters = (struct Character *)malloc(sizeof(struct Character) * characterCount);
    if (segment->characters == NULL)
    {
      return false;
    } // alloc failed case
    memset(segment->characters, 0, sizeof(struct Character) * characterCount); // init mem to zero
  }
  else if (characterCount > 0 && characters != NULL)
  {
    segment->characters = characters;
  }
  else
  {
    segment->characters = NULL;
  }

  return true;
}

bool initCharacter(struct Character *character, bool is_line_feed,
                   float advance_width, float left_side_bearing,
                   struct Stroke *strokes, int strokeCount)
{
  if (character == NULL)
    return false;

  character->advance_width = advance_width;
  character->left_side_bearing = left_side_bearing;
  character->is_line_feed = is_line_feed;
  character->strokeCount = strokeCount;

  if (strokeCount > 0 && strokes == NULL)
  {
    character->strokes = (struct Stroke *)malloc(sizeof(struct Stroke) * strokeCount);
    if (character->strokes == NULL)
    {
      return false;
    } // alloc failed case
    memset(character->strokes, 0, sizeof(struct Stroke) * strokeCount); // init mem to zero
  }
  else if (strokeCount > 0 && strokes != NULL)
  {
    character->strokes = strokes;
  }
  else
  {
    character->strokes = NULL;
  }

  return true;
}

bool initStroke(struct Stroke * stroke, struct Point *points,
                int pointCount)
{
  if (stroke == NULL)
    return NULL;

  stroke->pointCount = pointCount;

  if (pointCount > 0 && points == NULL)
  {
    stroke->points = (struct Point *)malloc(sizeof(struct Point) * pointCount);
    if (stroke->points == NULL)
    {
      return NULL;
    } // alloc failed case
    memset(stroke->points, 0, sizeof(struct Point) * pointCount); // init mem to zero
  }
  else if (pointCount > 0 && points != NULL)
  {
    stroke->points = points;
  }
  else
  {
    stroke->points = NULL;
  }

  return stroke;
}

// add:
// use these functions to add data in empty structures, or add new data
// CAUTION: the is no chain adding atm, do them in main

// WARNING: **CONSIDER REBOOT OVER FREE ATM**
// Mem ownership:
// - Add functions perform shallow copies. After calling:
// The parent structure gains references to the child's data.
// potentially cause double-free issues or memory leaks to the original pointer.

bool AddSegmentToDocument(struct Document *document, struct Segment *segment)
{
  if (document == NULL || segment == NULL)
    return false; // check for null pointers

  // expand array for new segment (assume the count is always matched with the object in the array)
  int newCount = document->segmentCount + 1; // dont do ++ here dumbass
  struct Segment *newSegments;

  if (document->segments == NULL) // no segments beforehand
  {
    newSegments = (struct Segment *)malloc(sizeof(struct Segment) * newCount);
    if (newSegments == NULL)
      return false; // alloc failed case
  }
  else // segments already exist
  {    // BAD: realloc may need to copy the whole array (actually I dont care)
    newSegments = (struct Segment *)realloc(document->segments, sizeof(struct Segment) * newCount);
    if (newSegments == NULL)
      return false; // alloc failed case
  }

  document->segments = newSegments; // copy the new segments array
  // copy the new segment into the array
  // BAD: this is a shallow copy, but as long as they work and free without error I dont care
  document->segments[document->segmentCount] = *segment;
  document->segmentCount++; // increment the segment count

  return true;
}

bool AddCharacterToSegment(struct Segment *segment, struct Character *character)
{
  if (segment == NULL || character == NULL)
    return false;

  int newCount = segment->characterCount + 1;
  struct Character *newCharacters;

  if (segment->characters == NULL)
  {
    newCharacters = (struct Character *)malloc(sizeof(struct Character) * newCount);
    if (newCharacters == NULL)
      return false;
  }
  else
  {
    newCharacters = (struct Character *)realloc(segment->characters, sizeof(struct Character) * newCount);
    if (newCharacters == NULL)
      return false;
  }

  segment->characters = newCharacters;
  segment->characters[segment->characterCount] = *character;
  segment->characterCount++;

  return true;
}

bool AddStrokeToCharacter(struct Character *character, struct Stroke *stroke)
{
  if (character == NULL || stroke == NULL)
    return false;

  int newCount = character->strokeCount + 1;
  struct Stroke *newStrokes;

  if (character->strokes == NULL)
  {
    newStrokes = (struct Stroke *)malloc(sizeof(struct Stroke) * newCount);
    if (newStrokes == NULL)
      return false;
  }
  else
  {
    newStrokes = (struct Stroke *)realloc(character->strokes, sizeof(struct Stroke) * newCount);
    if (newStrokes == NULL)
      return false;
  }

  character->strokes = newStrokes;
  character->strokes[character->strokeCount] = *stroke;
  character->strokeCount++;

  return true;
}

bool AddPointToStroke(struct Stroke *stroke, struct Point *point)
{
  if (stroke == NULL || point == NULL)
    return false;

  int newCount = stroke->pointCount + 1;
  struct Point *newPoints;

  if (stroke->points == NULL)
  {
    newPoints = (struct Point *)malloc(sizeof(struct Point) * newCount);
    if (newPoints == NULL)
      return false; // alloc failed case
  }
  else
  {
    newPoints = (struct Point *)realloc(stroke->points, sizeof(struct Point) * newCount);
    if (newPoints == NULL)
      return false; // alloc failed case
  }

  stroke->points = newPoints;
  stroke->points[stroke->pointCount] = *point;
  stroke->pointCount++;

  return true;
}

//
// writing template
//
// WANRING: NO DATA IS FREED!!! REBOOT AFTER USE!!!

// void drawOneStrokeFromDoc()
// {
//   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET); // enable

//   struct Document *doc = CreateDocument(1, 297.0f, 20.0f, 20.0f, 20.0f, 20.0f, 210.0f); // A4 size in mm
//   struct Segment *seg = CreateSegment(1, 10.0f, 3.0f, 2.0f, 5.0f);
//   struct Character *chara = CreateCharacter(1, 12.0f, 1.0f, false);
//   struct Stroke *stroke = CreateStroke(2);

//   struct Point pointA1_1 = {0.0f, 0.0f};
//   struct Point pointA1_2 = {10.0f, -10.0f};

//   AddPointToStroke(stroke, &pointA1_1);
//   AddPointToStroke(stroke, &pointA1_2);

//   AddStrokeToCharacter(chara, stroke);

//   AddCharacterToSegment(seg, chara);

//   AddSegmentToDocument(doc, seg);

//   drawDocument(doc);

//   // WANRING: NO DATA IS FREED!!! REBOOT AFTER USE!!!

//   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET); // disable
// }

//
// data parsing
//

// 解析 JSON
bool parseDocument(struct Document *document, cJSON *json)
{
  cJSON *raw_page_width = cJSON_GetObjectItem(json, "page_width");
  if (raw_page_width == NULL || !cJSON_IsNumber(raw_page_width))
  {
    return false;
  }
  float page_width = (float)(raw_page_width->valuedouble);
  cJSON *raw_page_height = cJSON_GetObjectItem(json, "page_height");
  if (raw_page_height == NULL || !cJSON_IsNumber(raw_page_height))
  {
    return false;
  }
  float page_height = (float)(raw_page_height->valuedouble);
  cJSON *raw_top_margin = cJSON_GetObjectItem(json, "top_margin");
  if (raw_top_margin == NULL || !cJSON_IsNumber(raw_top_margin))
  {
    return false;
  }
  float top_margin = (float)(raw_top_margin->valuedouble);
  cJSON *raw_bottom_margin = cJSON_GetObjectItem(json, "bottom_margin");
  if (raw_bottom_margin == NULL || !cJSON_IsNumber(raw_bottom_margin))
  {
    return false;
  }
  float bottom_margin = (float)(raw_bottom_margin->valuedouble);
  cJSON *raw_left_margin = cJSON_GetObjectItem(json, "left_margin");
  if (raw_left_margin == NULL || !cJSON_IsNumber(raw_left_margin))
  {
    return false;
  }
  float left_margin = (float)(raw_left_margin->valuedouble);
  cJSON *raw_right_margin = cJSON_GetObjectItem(json, "right_margin");
  if (raw_right_margin == NULL || !cJSON_IsNumber(raw_right_margin))
  {
    return false;
  }
  float right_margin = (float)(raw_right_margin->valuedouble);

  // 获取 segments 数组
  cJSON *raw_segments = cJSON_GetObjectItem(json, "segments");
  if (!cJSON_IsArray(raw_segments))
    return false;

  int segmentCount = cJSON_GetArraySize(raw_segments);
  struct Segment *segments = (struct Segment *)malloc(sizeof(struct Segment) * segmentCount);
  for (int i = 0; i < segmentCount; i++)
  {
    // 获取每个 segment 对象
    cJSON *seg_item = cJSON_GetArrayItem(raw_segments, i);
    // 对该 segment 深拷贝（递归拷贝所有子项）
    cJSON *new_seg_json = cJSON_Duplicate(seg_item, 1);
    if (new_seg_json != NULL)
    {
      // new_seg_json 就是一个独立的新的 JSON 对象
      // 这里可以传给其它函数处理，比如：
      if (!parseSegment(&segments[i], new_seg_json))
      {
        return false;
      }
    }
    // 使用后释放内存
    cJSON_Delete(new_seg_json);
  }
  // 初始化 Document 对象
  if (!initDocument(document, page_width, page_height, top_margin, bottom_margin, left_margin, right_margin, segments, segmentCount))
  {
    return false;
  }

  return true;
}

bool parseSegment(struct Segment *segment, cJSON *json)
{
  cJSON *raw_ascender = cJSON_GetObjectItem(json, "ascender");
  if (raw_ascender == NULL || !cJSON_IsNumber(raw_ascender))
  {
    return false;
  }
  float ascender = (float)(raw_ascender->valuedouble);
  cJSON *raw_descender = cJSON_GetObjectItem(json, "descender");
  if (raw_descender == NULL || !cJSON_IsNumber(raw_descender))
  {
    return false;
  }
  float descender = (float)(raw_descender->valuedouble);
  cJSON *raw_line_gap = cJSON_GetObjectItem(json, "line_gap");
  if (raw_line_gap == NULL || !cJSON_IsNumber(raw_line_gap))
  {
    return false;
  }
  float line_gap = (float)(raw_line_gap->valuedouble);
  cJSON *raw_paragraph_spacing = cJSON_GetObjectItem(json, "paragraph_spacing");
  if (raw_paragraph_spacing == NULL || !cJSON_IsNumber(raw_paragraph_spacing))
  {
    return false;
  }
  float paragraph_spacing = (float)(raw_paragraph_spacing->valuedouble);

  // 获取 characters 数组
  cJSON *raw_characters = cJSON_GetObjectItem(json, "characters");
  if (!cJSON_IsArray(raw_characters))
    return false;

  int characterCount = cJSON_GetArraySize(raw_characters);
  struct Character *characters = (struct Character *)malloc(sizeof(struct Character) * characterCount);
  for (int i = 0; i < characterCount; i++)
  {
    // 获取每个 character 对象
    cJSON *char_item = cJSON_GetArrayItem(raw_characters, i);
    // 对该 character 深拷贝（递归拷贝所有子项）
    cJSON *new_char_json = cJSON_Duplicate(char_item, 1);
    if (new_char_json != NULL)
    {
      // new_char_json 就是一个独立的新的 JSON 对象
      // 这里可以传给其它函数处理，比如：
      if (!parseCharacter(&characters[i], new_char_json))
      {
        return false;
      }
    }
    // 使用后释放内存
    cJSON_Delete(new_char_json);
  }
  // 初始化 Segment 对象
  if (!initSegment(segment, ascender, descender, line_gap, paragraph_spacing, characters, characterCount))
  {
    return false;
  }

  return true;
}

bool parseCharacter(struct Character *character, cJSON *json)
{
  cJSON *raw_is_line_feed = cJSON_GetObjectItem(json, "is_line_feed");
  if (raw_is_line_feed == NULL || !cJSON_IsBool(raw_is_line_feed))
  {
    return false;
  }
  bool is_line_feed = (bool)(raw_is_line_feed->valueint);
  cJSON *raw_advance_width = cJSON_GetObjectItem(json, "advance_width");
  if (raw_advance_width == NULL || !cJSON_IsNumber(raw_advance_width))
  {
    return false;
  }
  float advance_width = (float)(raw_advance_width->valuedouble);
  cJSON *raw_left_side_bearing = cJSON_GetObjectItem(json, "left_side_bearing");
  if (raw_left_side_bearing == NULL || !cJSON_IsNumber(raw_left_side_bearing))
  {
    return false;
  }
  float left_side_bearing = (float)(raw_left_side_bearing->valuedouble);

  // 获取 strokes 数组
  cJSON *raw_strokes = cJSON_GetObjectItem(json, "strokes");
  if (!cJSON_IsArray(raw_strokes))
    return false;

  int strokeCount = cJSON_GetArraySize(raw_strokes);
  struct Stroke *strokes = (struct Stroke *)malloc(sizeof(struct Stroke) * strokeCount);
  for (int i = 0; i < strokeCount; i++)
  {
    // 获取每个 stroke 对象
    cJSON *stroke_item = cJSON_GetArrayItem(raw_strokes, i);
    // 对该 stroke 深拷贝（递归拷贝所有子项）
    cJSON *new_stroke_json = cJSON_Duplicate(stroke_item, 1);
    if (!cJSON_IsArray(new_stroke_json))
    {
      cJSON_free(new_stroke_json);
      return false;
    }
    strokes[i].pointCount = cJSON_GetArraySize(new_stroke_json);
    strokes[i].points = (struct Point *)malloc(sizeof(struct Point) * strokes[i].pointCount);
    for (int j = 0; j < strokes[i].pointCount; j++)
    {
      cJSON *point_item = cJSON_GetArrayItem(new_stroke_json, j);
      cJSON *new_point_json = cJSON_Duplicate(point_item, 1);
      if (!cJSON_IsArray(new_point_json))
      {
        cJSON_free(new_point_json);
        return false;
      }
      cJSON *raw_x = cJSON_GetArrayItem(new_point_json, 0);
      if (!cJSON_IsNumber(raw_x))
      {
        cJSON_free(new_point_json);
        return false;
      }
      strokes[i].points[j].x = (float)(raw_x->valuedouble);
      cJSON *raw_y = cJSON_GetArrayItem(new_point_json, 1);
      if (!cJSON_IsNumber(raw_y))
      {
        cJSON_free(new_point_json);
        return false;
      }
      strokes[i].points[j].y = (float)(raw_y->valuedouble);
      cJSON_free(new_point_json);
    }
    // 使用后释放内存
    cJSON_Delete(new_stroke_json);
  }
  // 初始化 Character 对象
  if (!initCharacter(character, is_line_feed, advance_width, left_side_bearing, strokes, strokeCount))
  {
    return false;
  }

  return true;
}

// RPC 处理函数，解析收到的 JSON 数据并绘制文档
void processRpcRequest(uint8_t *buffer, uint16_t length)
{
  // 动态分配缓冲区，确保能够保存整个数据并以 '\0' 结尾
  char *json_data = (char *)malloc(length + 1);
  if (json_data == NULL)
  {
    const char *error_response = "{\"error\":\"内存分配失败\"}";
    CDC_Transmit_FS((uint8_t *)error_response, strlen(error_response));
    return;
  }
  memcpy(json_data, buffer, length);
  json_data[length] = '\0';

  cJSON *root = cJSON_Parse(json_data);
  free(json_data); // 解析后释放动态分配的内存
  if (root == NULL)
  {
    const char *error_response = "{\"error\":\"JSON解析失败\"}";
    CDC_Transmit_FS((uint8_t *)error_response, strlen(error_response));
    return;
  }

  // 解析 Document 结构（包括页面参数、segments、characters、strokes 等）
  struct Document *document = (struct Document *)malloc(sizeof(struct Document));
  if (document == NULL)
  {
    const char *error_response = "{\"error\":\"内存分配失败\"}";
    CDC_Transmit_FS((uint8_t *)error_response, strlen(error_response));
    cJSON_Delete(root);
    return;
  }
  if (!parseDocument(document, root))
  {
    const char *error_response = "{\"error\":\"解析Document失败\"}";
    CDC_Transmit_FS((uint8_t *)error_response, strlen(error_response));
    cJSON_Delete(root);
    return;
  }

  // 调用绘制函数，开始绘制文档（你的 drawDocument() 已定义）
  drawDocument(document);

  // 绘制完成后释放内存（注意本示例中可能需要在实际工程中完善内存管理）
  freeAllData(document);

  const char *success_response = "{\"result\":\"绘制完成\"}";
  CDC_Transmit_FS((uint8_t *)success_response, strlen(success_response));
  cJSON_Delete(root);
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
