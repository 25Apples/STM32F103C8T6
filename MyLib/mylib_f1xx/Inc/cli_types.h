#ifndef __CLI_TYPES_H
#define __CLI_TYPES_H

#include <stdint.h>

typedef void (*CLI_COMMAND_FUN_T)(char **argv, uint8_t arr_token);

typedef struct
{
	char *cmd_name;
	CLI_COMMAND_FUN_T func;
	char *help;
} cli_command_t;


#endif
