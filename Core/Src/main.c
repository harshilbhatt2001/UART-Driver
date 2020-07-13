/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>	//printf
#include <stdlib.h> //malloc
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// Add printf functionality
#ifdef __GNUC__
	#define PUTCHAR_PROTOTYPE int __io_fputc(int ch, FILE *f)
#else
	#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define POLY 0x8408		//for crc16 CCITT

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// TODO: Move queue code into different file.

void OnBoard_Led_PWM(uint8_t number);

struct Queue
{
	int front, rear, size;
	unsigned capacity;
	uint8_t* array;
};

struct Queue* createQueue(unsigned capacity)
{

    struct Queue* rxBuffer = (struct Queue*)malloc(
        sizeof(struct Queue));
    rxBuffer->capacity = capacity;
    rxBuffer->front = rxBuffer->size = 0;
      rxBuffer->rear = capacity - 1;
    rxBuffer->array = (uint8_t*)malloc(
        rxBuffer->capacity * sizeof(uint8_t));
    return rxBuffer;
}

int isFull(struct Queue* rxBuffer)
{
    return (rxBuffer->size == rxBuffer->capacity);
}

int isEmpty(struct Queue* rxBuffer)
{
    return (rxBuffer->size == 0);
}

void enqueue(struct Queue* rxBuffer, int item)
{
    if (isFull(rxBuffer))
        return;
    rxBuffer->rear = (rxBuffer->rear + 1)
                  % rxBuffer->capacity;
    rxBuffer->array[rxBuffer->rear] = item;
    rxBuffer->size = rxBuffer->size + 1;
}

int dequeue(struct Queue* rxBuffer)
{
    if (isEmpty(rxBuffer))
        return -1;
    int item = rxBuffer->array[rxBuffer->front];
    rxBuffer->front = (rxBuffer->front + 1)
                   % rxBuffer->capacity;
    rxBuffer->size = rxBuffer->size - 1;
    return item;
}

struct Queue* rxBuffer;

int num;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  rxBuffer = createQueue(25);
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
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  // ** Fixed Size 1 byte


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  HAL_UART_Receive_DMA(&huart3, (uint8_t *)rxBuffer->array, 1);
	  // TODO: crc16 check, if true send confirmation

	  OnBoard_Led_PWM(num);


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

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
PUTCHAR_PROTOTYPE
{
	HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xFFFF);
	return ch;
}

unsigned short crc16(char *data_p, unsigned short length)
{
      unsigned char i;
      unsigned int data;
      unsigned int crc = 0xffff;

      if (length == 0)
            return (~crc);

      do
      {
            for (i=0, data=(unsigned int)0xff & *data_p++;
                 i < 8;
                 i++, data >>= 1)
            {
                  if ((crc & 0x0001) ^ (data & 0x0001))
                        crc = (crc >> 1) ^ POLY;
                  else  crc >>= 1;
            }
      } while (--length);

      crc = ~crc;
      data = crc;
      crc = (crc << 8) | (data >> 8 & 0xff);

      return (crc);
}

void Check_Divisibility(int num)
{
	/* Number divisible by 4, Print "Rightbot"
	   * Number divisible by 7, Print "Labs"
	   * Number divisible by 4 and 7, Print "Rightbot Pvt Ltd"
	   * Number not divisible, Print Number
	   */
	if ((num & 3) ==0)
	  {
		  printf ("Rightbot");
		  if (num % 7 == 0)
		  {
			  printf (" Pvt Ltd");
			  char* data = "Rightbot Pvt Ltd";
			  printf ("%X", crc16(data, 16));
		  }
		  char* data = "Rightbot";
		  printf ("%X", crc16(data, 8));
	  }
	  else if (num % 7 == 0)
	  {
		  printf ("Labs");
		  char* data = "Labs";
		  printf ("%X", crc16(data, 4));
	  }
	  else
	  {
		  printf ("%d", num);
		  printf ("%X", crc16((char *)num, 1));
	  }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_UART_RxCpltCallback could be implemented in the user file
   */
  num = dequeue(rxBuffer);
  if (num > 0 || num < 100)
  {
	  Check_Divisibility(num);
	  crc16((char *)num, 1);
	  printf ("Received");
  }
  else
  {
	  HAL_UART_Transmit_DMA(&huart3, (uint8_t *)rxBuffer->array, 1);
  }
}

void OnBoard_Led_PWM(uint8_t number)
{
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	htim1.Instance->CCR1 = number;
	HAL_Delay(10000);
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
