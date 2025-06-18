#include "print_cli.h"

void PRINT_CLI(char *str, ...)
{
	char stringArray [BUFFER_UART];
	va_list args;
	va_start(args, str);
	uint8_t len_str = vsprintf(stringArray, str, args);
	va_end(args);

	HAL_UART_Transmit(&huart1, (uint8_t*) stringArray, len_str, 100);

}
