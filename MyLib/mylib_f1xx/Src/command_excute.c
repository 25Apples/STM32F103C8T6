#include "command_excute.h"
#include "temperature_cli.h"
#include "cli_types.h"

extern cli_command_t list_cmd[];

cli_command_t* find_commmand(char* cmd)
{
	for (uint8_t i = 0; list_cmd[i].cmd_name != NULL; i++) {
		if (!strcmp(list_cmd[i].cmd_name , cmd))
		{
			return &list_cmd[i];
		}
	}
	return NULL;
}

void COMMAND_EXCUTE(char *buff, uint8_t argc)
{
	char *argv[10];
	uint8_t arr_token = 0;

	char *token = strtok(buff, " ");
	while (token != NULL)
	{
		argv[arr_token++] = token;
	    token = strtok(NULL, " ");
	}
	cli_command_t *command = find_commmand(argv[0]);
	if (command == NULL)
	{
		PRINT_CLI("Command not found\n");
	}
	else
	{
		command -> func(argv, arr_token);
	}
}
