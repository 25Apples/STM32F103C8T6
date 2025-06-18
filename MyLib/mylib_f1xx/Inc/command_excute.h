#ifndef __COMMAND_EXCUTE_H
#define __COMMAND_EXCUTE_H

#include "main.h"
#include "uart.h"
#include "print_cli.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_UART 128

extern UART_HandleTypeDef huart1;
extern uint8_t buff[BUFFER_UART];

void COMMAND_EXCUTE(char *buff, uint8_t start);

#endif
