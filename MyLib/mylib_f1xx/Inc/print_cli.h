#ifndef __PRINT_CLI_H
#define __PRINT_CLI_H

#include "main.h"
#include "stm32f1xx_hal_uart.h"

#include <stdio.h>
#include <stdarg.h>

#define BUFFER_UART 128

extern UART_HandleTypeDef huart1;

void PRINT_CLI(char *str, ...);

#endif

