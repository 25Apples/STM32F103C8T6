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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define ADDRESS_DEVICE 0x68 << 1

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

char temperature[8];

typedef struct
{
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t date;
	uint8_t month;
	uint8_t year;

} __attribute__((packed)) DateTime;

DateTime date_time;

uint8_t Decimal2BCD(uint8_t num)
{
    return (num / 10) << 4 | (num % 10);
}

uint8_t BCD2Decimal(uint8_t num)
{
    return (num >> 4) * 10 + (num & 0x0F);
}


void RTC_WRITE_TIME(DateTime *pDt)
{
	uint8_t data_buff[8];
	data_buff [0] = 0x00;
	data_buff [1] = Decimal2BCD(pDt->sec);
	data_buff [2] = Decimal2BCD(pDt->min);
	data_buff [3] = Decimal2BCD(pDt->hour);
	data_buff [4] = Decimal2BCD(pDt->day);
	data_buff [5] = Decimal2BCD(pDt->date);
	data_buff [6] = Decimal2BCD(pDt->month);
	data_buff [7] = Decimal2BCD(pDt->year);
	HAL_I2C_Master_Transmit(&hi2c1, ADDRESS_DEVICE, data_buff, sizeof(data_buff), 100);
}

char* RTC_READ_TEMP()
{
    static char temp_str[16];
    uint8_t add_reg = 0x11;
    uint8_t data_buff[2];

    if (HAL_I2C_Master_Transmit(&hi2c1, ADDRESS_DEVICE, &add_reg, 1, 100) != HAL_OK)
    {
        strcpy(temp_str, "ERR_TX");
        return temp_str;
    }

    if (HAL_I2C_Master_Receive(&hi2c1, ADDRESS_DEVICE, data_buff, 2, 100) != HAL_OK)
    {
        strcpy(temp_str, "ERR_RX");
        return temp_str;
    }

    int8_t temp_int = data_buff[0];
    uint8_t frac_bits = data_buff[1] >> 6;
    int frac_val = frac_bits * 25;

    sprintf(temp_str, "%d.%02dC", temp_int, frac_val);
    return temp_str;
}


void RTC_READ_TIME(DateTime *pDt)
{
	uint8_t add_reg = 0x00;
	uint8_t data_buff[7];
	HAL_I2C_Master_Transmit(&hi2c1, ADDRESS_DEVICE, &add_reg, sizeof(add_reg), 100);
	HAL_I2C_Master_Receive(&hi2c1, ADDRESS_DEVICE, data_buff, sizeof(data_buff), 100);
	pDt->sec = BCD2Decimal(data_buff [0]);
	pDt->min = BCD2Decimal(data_buff [1]);
	pDt->hour = BCD2Decimal(data_buff [2]);
	pDt->day = BCD2Decimal(data_buff [3]);
	pDt->date = BCD2Decimal(data_buff [4]);
	pDt->month = BCD2Decimal(data_buff [5]);
	pDt->year = BCD2Decimal(data_buff [6]);
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
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

//  date_time.sec = 0;
//  date_time.min = 22;
//  date_time.hour = 23;
//  date_time.day= 2;
//  date_time.date = 16;
//  date_time.month = 6;
//  date_time.year = 25;
//  RTC_WRITE_TIME(&date_time);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  static uint32_t time_read = 0;
	  if (HAL_GetTick() - time_read >= 1000)
	  {
		  time_read = HAL_GetTick();
		  RTC_READ_TIME(&date_time);
		  strcpy(temperature, RTC_READ_TEMP(&date_time));

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
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

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
