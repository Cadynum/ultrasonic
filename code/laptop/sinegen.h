#pragma once

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

#define TAU (2*M_PI)

typedef struct {
	double phase;
	double w;
	double vib_phase;
	double vib_w;
	double s;
} state;  


void sinewave(state * const state, int16_t *buf, const uint samples, double w1, double lux);