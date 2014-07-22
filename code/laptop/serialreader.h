#pragma once
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>


typedef struct {
	uint16_t lux, range;
} serial_vars;

void* serialreader (serial_vars *sv);