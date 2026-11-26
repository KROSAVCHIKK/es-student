#pragma once

#include <stdint.h>

#define DEVICE_SERIAL_SIZE 17

typedef struct
{
    const char *board;
    char serial[DEVICE_SERIAL_SIZE];
    uint32_t manufacturer;
    uint32_t part;
    uint32_t revision;
    const char *sdk_version;
} device_info_t;

void device_get_info(device_info_t *info);
