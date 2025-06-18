#include "temperature_cli.h"
#include "print_cli.h"
#include "cli_types.h"

void getTemp(char **argv, uint8_t arr_token)
{
	if (arr_token != 2)
	{
		PRINT_CLI("Too much argument\n");
		return;
	}
	PRINT_CLI("Nhiet do hien tai CHANNEL %d: \n", atoi(argv[1]));
}

void setTempMax(char **argv, uint8_t arr_token)
{
	if (arr_token != 3)
	{
		PRINT_CLI("Too much argument\n");
		return;
	}
	if (atoi(argv[1])  > 5 )
	{
		PRINT_CLI("CHANNEL Error\n");
		return;
	}
	if (atoi(argv[2])  > 100 )
	{
		PRINT_CLI("Temperature Error\n");
		return;
	}
	PRINT_CLI("Cai Nhiet Do Max CHANNEL %d: %d \n", atoi(argv[1]), atoi(argv[2]));
}

void setTempMin(char **argv, uint8_t arr_token)
{
	if (arr_token != 3)
	{
		PRINT_CLI("Too much argument\n");
		return;
	}
	if (atoi(argv[1])  > 5 )
	{
		PRINT_CLI("CHANNEL Error\n");
		return;
	}
	if (atoi(argv[2])  < - 100 )
	{
		PRINT_CLI("Temperature Error\n");
		return;
	}
	PRINT_CLI("Cai Nhiet Do Min CHANNEL %d: %d \n", atoi(argv[1]), atoi(argv[2]));
}
