#include "uart.h"

uint8_t data_rx;
uint8_t buff[BUFFER_UART];
uint8_t index_uart;
uint8_t Flag_UART;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == huart1.Instance)
	{
		buff[index_uart++] = data_rx;
		if (data_rx == '\n')
		{
			Flag_UART = 1;

		}
		HAL_UART_Receive_IT(&huart1, &data_rx, sizeof(data_rx));
	}
}

void UART_Init(UART_HandleTypeDef *huart)
{
	HAL_UART_Receive_IT(huart, &data_rx, sizeof(data_rx));
}

void UART_HANDLE()
{
	if (Flag_UART)
	{
		COMMAND_EXCUTE((char*) buff, index_uart);
		memset(buff, 0, sizeof(buff));
		index_uart = 0;
		Flag_UART = 0;
	}

}
