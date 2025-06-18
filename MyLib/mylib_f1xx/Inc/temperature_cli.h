#ifndef __TEMPERATURE_CLI_H
#define __TEMPERATURE_CLI_H

#include "main.h"

#include <stdlib.h>

void getTemp(char **argv, uint8_t arr_token);
void setTempMax(char **argv, uint8_t arr_token);
void setTempMin(char **argv, uint8_t arr_token);

#endif
