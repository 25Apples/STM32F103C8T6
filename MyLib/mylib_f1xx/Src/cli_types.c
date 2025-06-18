#include "cli_types.h"
#include "temperature_cli.h"

cli_command_t list_cmd[] = {
    {
        .cmd_name = "getTemp",
        .func = getTemp,
        .help = "Cai dat nhiet do"
    },
    {
        .cmd_name = "setTempMax",
        .func = setTempMax,
        .help = "Cai Nhiet Do Max"
    },
    {
        .cmd_name = "setTempMin",
        .func = setTempMin,
        .help = "Cai Nhiet Do Min"
    },
    {NULL, NULL, NULL}
};
