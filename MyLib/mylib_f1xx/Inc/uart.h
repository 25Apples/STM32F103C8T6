#ifndef __UART_H
#define __UART_H

#include <string.h>

#include "main.h"
#include "print_cli.h"
#include "command_excute.h"

#define BUFFER_UART 128

extern UART_HandleTypeDef huart1;

extern uint8_t data_rx;
extern uint8_t buff[BUFFER_UART];
extern uint8_t index_uart;
extern uint8_t Flag_UART;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void UART_Init(UART_HandleTypeDef *huart);
void UART_HANDLE();

#endif
