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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define CMD_ON "led_on"
#define CMD_OFF "led_off"

GPIO_TypeDef* LED_GPIO_PORT[7] = {
    LED_1_GPIO_Port,
    LED_2_GPIO_Port,
    LED_3_GPIO_Port,
    LED_4_GPIO_Port,
    LED_5_GPIO_Port,
    LED_6_GPIO_Port,
    LED_7_GPIO_Port
};

uint16_t LED_GPIO_PIN[7] = {
    LED_1_Pin,
    LED_2_Pin,
    LED_3_Pin,
    LED_4_Pin,
    LED_5_Pin,
    LED_6_Pin,
    LED_7_Pin
};


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t Data_Rx;
uint8_t Flag_Uart = 0;
uint8_t Index_Uart = 0;

char buff_uart[50] = "";

typedef struct {
    uint8_t* data;
    size_t size;
    size_t capacity;
} Vector_uint8_t;

void vector_init(Vector_uint8_t* v) {
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

void vector_push(Vector_uint8_t* v, uint8_t value) {
    if (v->size >= v->capacity) {
        v->capacity = (v->capacity == 0) ? 4 : v->capacity * 2;
        v->data = (uint8_t*) realloc(v->data, v->capacity * sizeof(uint8_t));
    }
    v->data[v->size++] = value;
}

void vector_free(Vector_uint8_t* v) {
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

Vector_uint8_t temp_on, temp_off;

void parse_led_command(const char* command, const char* label, Vector_uint8_t* out_vector) {
    const char* start = strstr(command, label);
    if (start != NULL) {
        const char* colon = strchr(start, ':');
        if (colon != NULL) {
            colon++;
            uint8_t temp_number = 0;
            while (*colon != ';' && *colon != '\0') {
                if (*colon >= '0' && *colon <= '9') {
                    temp_number = temp_number * 10 + (*colon - '0');
                } else if (*colon == ',' || *colon == ' ') {
                    vector_push(out_vector, temp_number);
                    temp_number = 0;
                }
                colon++;
            }
            if (temp_number > 0)
                vector_push(out_vector, temp_number);
        }
    }
}

void SET_LED(const char* uart_buffer) {
    vector_init(&temp_on);
    vector_init(&temp_off);
    parse_led_command(uart_buffer, CMD_ON, &temp_on);
    parse_led_command(uart_buffer, CMD_OFF, &temp_off);
}

void SET_UP_STATUS(Vector_uint8_t* on, Vector_uint8_t* off) {
    for (size_t i = 0; i < on->size; i++) {
        uint8_t pin_index = on->data[i];
        if (pin_index >= 1 && pin_index <= 7) {
            HAL_GPIO_WritePin(LED_GPIO_PORT[pin_index - 1], LED_GPIO_PIN[pin_index - 1], GPIO_PIN_SET);
        }
    }

    for (size_t i = 0; i < off->size; i++) {
        uint8_t pin_index = off->data[i];
        if (pin_index >= 1 && pin_index <= 7) {
            HAL_GPIO_WritePin(LED_GPIO_PORT[pin_index - 1], LED_GPIO_PIN[pin_index - 1], GPIO_PIN_RESET);
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == huart1.Instance)
	{
		Flag_Uart = 1;
		buff_uart [Index_Uart++] = Data_Rx;
		if (Data_Rx == '.')
		{
			buff_uart[Index_Uart] = '\0';
			Index_Uart = 0;
			memset(buff_uart, 0, sizeof(buff_uart));
			Data_Rx = '\0';
			HAL_UART_Receive_IT(&huart1, &Data_Rx, 1);
			vector_free(&temp_on);
			vector_free(&temp_off);
		}
		else
		{
			HAL_UART_Receive_IT(&huart1, &Data_Rx, 1);
		}
	}
}

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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart1, &Data_Rx, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (Flag_Uart)
	  {
		  SET_LED(buff_uart);
		  SET_UP_STATUS(&temp_on, &temp_off);
		  Flag_Uart = 0;
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_1_Pin|LED_2_Pin|LED_3_Pin|LED_4_Pin
                          |LED_5_Pin|LED_6_Pin|LED_7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_1_Pin LED_2_Pin LED_3_Pin LED_4_Pin
                           LED_5_Pin LED_6_Pin LED_7_Pin */
  GPIO_InitStruct.Pin = LED_1_Pin|LED_2_Pin|LED_3_Pin|LED_4_Pin
                          |LED_5_Pin|LED_6_Pin|LED_7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

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
