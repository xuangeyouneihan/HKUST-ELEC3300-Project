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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
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

struct Document *CreateDocument(int segmentCount, float page_height, float top_margin, float bottom_margin, float left_margin, float right_margin, float page_width);
struct Segment *CreateSegment(int characterCount, float ascender, float descender, float line_gap, float paragraph_spacing);
struct Character *CreateCharacter(int strokeCount, float advance_width, float left_side_bearing, bool is_line_feed);
struct Stroke *CreateStroke(int pointCount);

void drawFu();
void drawCircle();
void drawRegularPentagon();
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
  float scale = 1; // TODO: find the relationship between mm and moving time

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

// create:
// empty strctures & empty array
// CAUTION: Use add functions to add data in to them later
// WARNING: ownership problems in add functions, not important in this project but do mind that

struct Document *CreateDocument(int segmentCount, float page_height,
                                float top_margin, float bottom_margin,
                                float left_margin, float right_margin,
                                float page_width)
{
  struct Document *Doc = (struct Document *)malloc(sizeof(struct Document));
  if (Doc == NULL)
    return NULL; // alloc failed case

  Doc->page_width = page_width;
  Doc->page_height = page_height;
  Doc->top_margin = top_margin;
  Doc->bottom_margin = bottom_margin;
  Doc->left_margin = left_margin;
  Doc->right_margin = right_margin;
  Doc->segmentCount = segmentCount;
  // if content is given,
  // create the segments array with segmentCount,
  // then use AddSegmentToDocument() to later add segments
  if (segmentCount > 0)
  {
    Doc->segments = (struct Segment *)malloc(sizeof(struct Segment) * segmentCount);
    if (Doc->segments == NULL)
    {
      free(Doc);
      return NULL;
    } // alloc failed case
    memset(Doc->segments, 0, sizeof(struct Segment) * segmentCount); // init mem to zero
  }
  else
  {
    Doc->segments = NULL;
  }

  return Doc;
}

struct Segment *CreateSegment(int characterCount, float ascender,
                              float descender, float line_gap,
                              float paragraph_spacing)
{
  struct Segment *seg = (struct Segment *)malloc(sizeof(struct Segment));
  if (seg == NULL)
    return NULL; // alloc failed case

  seg->ascender = ascender;
  seg->descender = descender;
  seg->line_gap = line_gap;
  seg->paragraph_spacing = paragraph_spacing;
  seg->characterCount = characterCount;

  if (characterCount > 0)
  {
    seg->characters = (struct Character *)malloc(sizeof(struct Character) * characterCount);
    if (seg->characters == NULL)
    {
      free(seg);
      return NULL;
    } // alloc failed case
    memset(seg->characters, 0, sizeof(struct Character) * characterCount); // init mem to zero
  }
  else
  {
    seg->characters = NULL;
  }

  return seg;
}

struct Character *CreateCharacter(int strokeCount, float advance_width,
                                  float left_side_bearing, bool is_line_feed)
{
  struct Character *chara = (struct Character *)malloc(sizeof(struct Character));
  if (chara == NULL)
    return NULL; // alloc failed case

  chara->advance_width = advance_width;
  chara->left_side_bearing = left_side_bearing;
  chara->is_line_feed = is_line_feed;
  chara->strokeCount = strokeCount;

  if (strokeCount > 0)
  {
    chara->strokes = (struct Stroke *)malloc(sizeof(struct Stroke) * strokeCount);
    if (chara->strokes == NULL)
    {
      free(chara);
      return NULL;
    } // alloc failed case
    memset(chara->strokes, 0, sizeof(struct Stroke) * strokeCount); // init mem to zero
  }
  else
  {
    chara->strokes = NULL;
  }

  return chara;
}

struct Stroke *CreateStroke(int pointCount)
{
  struct Stroke *stroke = (struct Stroke *)malloc(sizeof(struct Stroke));
  if (stroke == NULL)
    return NULL; // alloc failed case

  stroke->pointCount = pointCount;

  if (pointCount > 0)
  {
    stroke->points = (struct Point *)malloc(sizeof(struct Point) * pointCount);
    if (stroke->points == NULL)
    {
      free(stroke);
      return NULL;
    } // alloc failed case
    memset(stroke->points, 0, sizeof(struct Point) * pointCount); // init mem to zero
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
