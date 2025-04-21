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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// Motor directions
#define MOTOR_L_DIR GPIO_PIN_12
#define MOTOR_L_STEP GPIO_PIN_13
#define MOTOR_R_DIR GPIO_PIN_14
#define MOTOR_R_STEP GPIO_PIN_15

#define DIR_CW GPIO_PIN_SET
#define DIR_CCW GPIO_PIN_RESET
// #define DIR_STOP  PLACEHOLD

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
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
  htim1.Init.Prescaler = 99;
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
  sConfigOC.Pulse = 35;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
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
  htim2.Init.Prescaler = 99;
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
  sConfigOC.Pulse = 35;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
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
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_14, GPIO_PIN_RESET);

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

  /*Configure GPIO pins : PB10 PB12 PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_14;
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

  legacyMove(3, 3000, 1);
  HAL_Delay(1000);
  legacyMove(5, 3000, 1);
  HAL_Delay(1000);
  legacyMove(7, 3000, 1);
  HAL_Delay(1000);
  legacyMove(1, 3000, 1);
  HAL_Delay(1000);

  penup();
  HAL_Delay(1000);
  legacyMove(4, 871, 0);
  HAL_Delay(1000);
  legacyMove(6, 400, 0);
  HAL_Delay(1000);

  pendown();
  HAL_Delay(1000);
  legacyMove(2, 300, 1);
  HAL_Delay(1000);

  penup();
  HAL_Delay(1000);
  legacyMove(4, 200, 0);
  HAL_Delay(1000);
  legacyMove(6, 450, 0);
  HAL_Delay(1000);

  pendown();
  HAL_Delay(1000);
  legacyMove(2, 500, 1);
  HAL_Delay(1000);

  penup();
  HAL_Delay(1000);
  legacyMove(6, 200, 0);
  HAL_Delay(1000);

  pendown();
  HAL_Delay(1000);
  legacyMove(4, 900, 1);
  HAL_Delay(1000);

  penup();
  HAL_Delay(1000);
  legacyMove(0, 650, 0);
  HAL_Delay(1000);
  legacyMove(6, 150, 0);
  HAL_Delay(1000);

  pendown();
  HAL_Delay(1000);
  legacyMove(4, 350, 1);
  HAL_Delay(1000);

  penup();
  HAL_Delay(1000);
  legacyMove(0, 350, 0);
  HAL_Delay(1000);
  legacyMove(2, 300, 0);
  HAL_Delay(1000);

  pendown();
  HAL_Delay(1000);
  legacyMove(4, 350, 1);
  HAL_Delay(1000);

  penup();
  HAL_Delay(1000);
  legacyMove(2, 200, 0);
  HAL_Delay(1000);
  legacyMove(0, 825, 0);
  HAL_Delay(1000);

  pendown();
  HAL_Delay(1000);
  legacyMove(2, 400, 1);
  HAL_Delay(1000);

  penup();
  HAL_Delay(1000);
  legacyMove(6, 50, 0);
  HAL_Delay(1000);
  legacyMove(4, 150, 0);
  HAL_Delay(1000);

  pendown();
  HAL_Delay(1000);
  legacyMove(4, 200, 1);
  HAL_Delay(1000);
  legacyMove(6, 300, 1);
  HAL_Delay(1000);
  legacyMove(0, 200, 1);
  HAL_Delay(1000);
  legacyMove(2, 300, 1);
  HAL_Delay(1000);

  penup();
  HAL_Delay(1000);
  legacyMove(4, 350, 0);
  HAL_Delay(1000);
  legacyMove(2, 100, 0);
  HAL_Delay(1000);

  pendown();
  HAL_Delay(1000);
  legacyMove(4, 500, 1);
  HAL_Delay(1000);
  legacyMove(6, 500, 1);
  HAL_Delay(1000);
  legacyMove(0, 500, 1);
  HAL_Delay(1000);
  legacyMove(2, 500, 1);
  HAL_Delay(1000);

  penup();
  HAL_Delay(1000);
  legacyMove(6, 500, 0);
  HAL_Delay(1000);
  legacyMove(4, 225, 0);
  HAL_Delay(1000);

  pendown();
  HAL_Delay(1000);
  legacyMove(2, 500, 1);
  HAL_Delay(1000);

  penup();
  HAL_Delay(1000);
  legacyMove(0, 225, 0);
  HAL_Delay(1000);
  legacyMove(6, 225, 0);
  HAL_Delay(1000);

  pendown();
  HAL_Delay(1000);
  legacyMove(4, 500, 1);

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET); // disable
}

void drawCircle()
{
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET); // enable

  for (int i = 0; i < 72; i++)
  {
    if (!(i % 12))
    {
      uint32_t tick = HAL_GetTick();
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
      while (HAL_GetTick() - tick < 5)
      {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_11);
        HAL_Delay(1);
      }
    }
    moveAngle(25, i * 5);
  }

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET); // disable
}

void drawRegularPentagon()
{
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET); // enable

  uint32_t tick = HAL_GetTick();
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
  while (HAL_GetTick() - tick < 5)
  {
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_11);
    HAL_Delay(1);
  }
  for (int i = 0; i < 5; i++)
  {
    moveAngle(200, (float)i * 360 / 5);
  }

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET); // disable
}

void penup()
{
  uint32_t tick = HAL_GetTick();
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
  while (HAL_GetTick() - tick < 100)
  {
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_11);
    HAL_Delay(1);
  }
}

void pendown()
{
  uint32_t tick = HAL_GetTick();
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
  while (HAL_GetTick() - tick < 100)
  {
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_11);
    HAL_Delay(1);
  }
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
    // check motorL error
    if (error_L >= stepsL)
    {
      if (stepsL > 0)
      {
        HAL_GPIO_WritePin(GPIOB, MOTOR_L_STEP, GPIO_PIN_SET);
        HAL_Delay(DFLT_STEP_T);
        HAL_GPIO_WritePin(GPIOB, MOTOR_L_STEP, GPIO_PIN_RESET);
      }
      error_L -= maxSteps;
    }
    // step motorR
    if (error_R >= stepsR)
    {
      if (stepsR > 0)
      {
        HAL_GPIO_WritePin(GPIOB, MOTOR_R_STEP, GPIO_PIN_SET);
        HAL_Delay(DFLT_STEP_T);
        HAL_GPIO_WritePin(GPIOB, MOTOR_R_STEP, GPIO_PIN_RESET);
      }
      error_R -= maxSteps;
    }

    // update error
    error_L += stepsL;
    error_R += stepsR;

    HAL_Delay(1);
  }
}

// WARNING:
// deltas can be floats, steps and Global positions are int32_t
// steps are time dependent, overtime relative error will accumulate
void updateGlobalXY(float delta_X, float delta_Y)
{
  Global_X += (int32_t)delta_X;
  Global_Y += (int32_t)delta_Y;
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

#ifdef  USE_FULL_ASSERT
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
