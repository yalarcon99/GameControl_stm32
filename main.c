/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "iks01a3_motion_sensors.h" //Motion sensor structure define
#include "math.h"   //Math library to compute trigonometric functions
#include "stdio.h"  //Standard input/output library
#include "stdlib.h" //Standard library
#include "string.h" //Library for general types of strings
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
//Initialization for Axis angular Acceleration reading (Accelerometer)
int32_t returnstatus_A = 0;
int32_t xAxisReading_A = 0;
int32_t yAxisReading_A = 0;
int32_t zAxisReading_A = 0;
//Initialization for Axis angular Velocities reading (Gyroscope)
int32_t returnstatus_G = 0;
int32_t xAxisReading_G = 0;
int32_t yAxisReading_G = 0;
int32_t zAxisReading_G = 0;
//Initialization for Axis angular Acceleration Filtered
int32_t xAxisReadingA_Fil = 0;
int32_t yAxisReadingA_Fil = 0;
int32_t zAxisReadingA_Fil = 0;
//Variables that contains the previous value of each angular axis velocity (Gyroscope)
int32_t xVector1 = 0;
int32_t yVector1 = 0;
int32_t zVector1 = 0;
//Variables that contains the current value of each angular axis velocity (Gyroscope)
int32_t xVector2 = 0;
int32_t yVector2 = 0;
int32_t zVector2 = 0;
//Variables that contains the result of the integration to obtain the gyroscope angles
int32_t positionX = 0;
int32_t positionY = 0;
int32_t positionZ = 0;
//Variables that temporarily save the previous angular velocity
int32_t Valueprex = 0;
int32_t Valueprey = 0;
int32_t Valueprez = 0;
//Useful variables counters to be used in timer and external interrupt respectively
uint32_t i2 = 0;
uint32_t exticounter = 0;
//Variables that contains the calculations of the arctangent functions
int thetax = 0;
int thetay = 0;
//Variables that contains the combination of each axis angle from Gyroscope and Accelerometer
int anglex = 0;
int angley = 0;
int anglez = 0;
//Accumulator for axis acceleration to be processed by the moving average filter
int accx_f[5] = {0,0,0,0,0};
int accy_f[5] = {0,0,0,0,0};
int accz_f[5] = {0,0,0,0,0};
//Variable that contains the angles obtained from the Gyroscope
int anglez_g = 0;
int anglex_g = 0;
int angley_g = 0;
//Constant defined to execute trigonometric calculations
#define pi 3.141592
//Confirmation variables for USART protocol
int volatile corrSentData = 0;
int volatile corrReceData = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
//Declaration of the integration function to find the angular position of the sensor
int position_int(int zVector1, int zVector2);
//Declaration of the interrupt generated by the timer when it reach its maximum count
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim3);
//Declaration of the inverse tangent function to calculate acceleration angles
int arctangent(int ax, int ay, int az, int y);
//Declaration of the moving average filter function
int filter(int accf[5]);
//Transmitter-Receiver Interruption declaration of USART protocol
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart2);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart2);
//Declaration of the external interruption generated by the B1 button
void HAL_GPIO_EXTI_Callback(uint16_t BUTTON);
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
  MX_TIM3_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  //Initialization of Accelerometer and Axis Variable Reference assign
  IKS01A3_MOTION_SENSOR_Init(1,MOTION_ACCELERO);
  IKS01A3_MOTION_SENSOR_Enable(1,MOTION_ACCELERO);
  IKS01A3_MOTION_SENSOR_Axes_t axes_A;
  returnstatus_A = IKS01A3_MOTION_SENSOR_GetAxes(1,MOTION_ACCELERO,&axes_A);
  //Initialization of Gyroscope and Axis Variable Reference assign
  IKS01A3_MOTION_SENSOR_Init(0,MOTION_GYRO);
  IKS01A3_MOTION_SENSOR_Enable(0,MOTION_GYRO);
  IKS01A3_MOTION_SENSOR_Axes_t axes_G;
  returnstatus_G = IKS01A3_MOTION_SENSOR_GetAxes(0,MOTION_GYRO,&axes_G);
  //Initialize the interruption generated by the timer 3
  HAL_TIM_Base_Start_IT(&htim3);
  //Defining the letters commands to be interpreted by the Python code
  //and execute the Keyboard command
  char stop[2];
  sprintf(stop,"\n");
  char left[3];
  sprintf(left,"a\n");
  char right[3];
  sprintf(right,"d\n");
  char forward[3];
  sprintf(forward,"w\n");
  char enter[3];
  sprintf(enter,"e\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //Obtaining data from axis acceleration
	  xAxisReading_A = axes_A.x;
	  yAxisReading_A = axes_A.y;
	  zAxisReading_A = axes_A.z;
	  //Obtaining data from axis velocities
	  xAxisReading_G = axes_G.x;
	  yAxisReading_G = axes_G.y;
	  zAxisReading_G = axes_G.z;
	  //Return status for each module
	  returnstatus_A = IKS01A3_MOTION_SENSOR_GetAxes(1,MOTION_ACCELERO,&axes_A);
	  returnstatus_G = IKS01A3_MOTION_SENSOR_GetAxes(0,MOTION_GYRO,&axes_G);
	  //Conditional For-Loop to add new data to the accumulative vector of acceleration
	  for(int i=0; i<5; i++){
		  if(i==4){
			  accx_f[i] = axes_A.x;
			  accy_f[i] = axes_A.y;
			  accz_f[i] = axes_A.z;
		  }else{
			  accx_f[i] = accx_f[i+1];
			  accy_f[i] = accy_f[i+1];
			  accz_f[i] = accz_f[i+1];
		  }
	  }
	  //Applying Moving Average Filter function
	  xAxisReadingA_Fil = filter(accx_f);
	  yAxisReadingA_Fil = filter(accy_f);
	  zAxisReadingA_Fil = filter(accz_f);
	  //Complementary filter
	  anglex = arctangent(xAxisReadingA_Fil, yAxisReadingA_Fil, zAxisReadingA_Fil, 1)*92/100+anglex_g*8/100;
	  angley = arctangent(xAxisReadingA_Fil, yAxisReadingA_Fil, zAxisReadingA_Fil, 2)*92/100+angley_g*8/100;
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  //Enter:
	  if(exticounter > 0){
		  	HAL_UART_Transmit_IT(&huart2,(uint8_t *)enter,strlen(enter));
		  	HAL_Delay(2);
		  	exticounter = 0;
	  }
	  //Drive forward:
	  if(anglex > -10 && anglex < 10 && angley < 30 && angley > -20){
		  	HAL_UART_Transmit_IT(&huart2,(uint8_t *)forward,strlen(forward));
		  	HAL_Delay(2);
	  //Move forward and turn left:
	  }else if(anglex > 10 && angley < 30 && angley > -20){
			HAL_UART_Transmit_IT(&huart2,(uint8_t *)forward,strlen(forward));
		    HAL_Delay(2);
		    HAL_UART_Transmit_IT(&huart2,(uint8_t *)left,strlen(left));
		  	HAL_Delay(2);
	  //Move forward and turn right:
  	  }else if(anglex < -15 && angley < 30 && angley > -20){
  		  	HAL_UART_Transmit_IT(&huart2,(uint8_t *)forward,strlen(forward));
			HAL_Delay(2);
			HAL_UART_Transmit_IT(&huart2,(uint8_t *)right,strlen(right));
			HAL_Delay(2);
  	  //Turn left
  	  }else if(anglez > 20 && angley > 30){
  		  	HAL_UART_Transmit_IT(&huart2,(uint8_t *)left,strlen(left));
  			HAL_Delay(2);
  	  //Turn right
  	  }else if(anglez < -20 && angley > 30){
  			HAL_UART_Transmit_IT(&huart2,(uint8_t *)right,strlen(right));
  			HAL_Delay(2);
	  //Stop
	  }else{
		    HAL_UART_Transmit_IT(&huart2,(uint8_t *)stop,strlen(stop));
		  	HAL_Delay(2);
	  }
	  //Confirmation for USART communication
	  if(corrSentData == 1){
	  		  corrSentData = 0;
	  }
	  if(corrReceData == 1){
	 		  corrReceData = 0;
	  }
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 8400;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 100;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BUTTON_Pin */
  GPIO_InitStruct.Pin = BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
int position_int(int zVector1_1, int zVector2_1){
	//The equation for the integration is: position = (zVector2+zVector1)*deltaTime/2
	int32_t sum = zVector2_1+zVector1_1;
	//In this case the time interval is 10 ms
	//Therefore, we have to divide by 200 including the division by two of the formula
	int32_t resultPos = sum/200;
	return resultPos;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim3){
	//Operations to be performed each time the Timer counter reaches the value of the ARR register
	//Removing offset from raw data
	//Assign to actual variable
	zVector2 = (zAxisReading_G+350);
	xVector2 = (xAxisReading_G-315);
	yVector2 = (yAxisReading_G-70);
	//Conditional to assign previous and actual Gyroscope velocity values for integration process
	i2++;
	if(i2 > 0){
		Valueprez = zAxisReading_G;
		Valueprey = yAxisReading_G;
		Valueprex = xAxisReading_G;
	}else if(i2 > 1){
		zVector1 = (Valueprez+350);
		xVector1 = (Valueprex-315);
		yVector1 = (Valueprey-70);
		i2 = 0;
	}
	//Accumulative and integration process to obtain angle x,y and z from the gyroscope data
	positionZ += position_int(zVector1, zVector2);
	positionX += position_int(xVector1, xVector2);
	positionY += position_int(yVector1, yVector2);
    //Filter processing to normalize data into readable values
    anglez = positionZ/500;
	anglex_g = positionX/500;
    angley_g = positionY/500;
    anglez_g = positionZ/500;
}
//Function to calculate the arctangent given three components, in this case, acceleration
int arctangent(int ax, int ay, int az, int y){
	int angle;
	//Trigonometric equations to obtain the angles given axis components
	//And conversion from radians to degree
	thetax = atan(ax/sqrt((ay*ay)+(az*az)))*(180/pi);
	thetay = atan(ay/sqrt((ax*ax)+(az*az)))*(180/pi);
	//Recursive condition to return different angles with same function
	if(y==1){
		angle = thetax;
	}else if(y==2){
		angle = thetay;
	}
	return angle;
}
//Moving average filter with 5 samples
int filter(int acc[5]){
	//Accumulative filter variable initially set to 0
	int accf = 0;
	//Summation and division process to obtain the average
	for(int i = 0; i < 5; i++)
		accf += acc[i]/5;
	return accf;
}
//Variable confirmation assignment for USART protocol
//Transmitter
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart2){
	corrSentData = 1;
}
//Receiver
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart2){
	corrReceData = 1;
}
//External interruption activation
void HAL_GPIO_EXTI_Callback(uint16_t BUTTON){
	//Turn-ON led
	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
	exticounter++;
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
