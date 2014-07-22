#include "sinegen.h"

#define FHZ 48000
#define K_BASE (16384* M_E)


static inline uint min(uint a, uint b) {
	if (a <= b) {
		return a;
	} else {
		return b;
	}
}


void sinewave(state * const state, int16_t *buf, const uint samples, double w1, double lux) {
	const double d_vib = lux/FHZ;
	const double d = state->w/FHZ;

	const double k = state->w <= w1 ? K_BASE : 1/K_BASE;

	bool chirp = state->w != w1;
	double freq = state->w;

	for (uint i=0; i < samples; i++) {
		if (chirp) {
			const double t = (double)state->s/FHZ;
			freq = state->w * pow(k, t);
			if (freq >= w1) {
				chirp = false;
				state->w = w1;
				state->s = 1;
				state->phase += d;
			} else {
				freq = state->w;
				state->s++;
				state->phase += freq/FHZ;
			}
		} else {
			state->phase += d;
		}

		//vibrato
		state->vib_phase += d_vib;
		state->phase += (state->w/20 * sin(state->vib_phase))/FHZ;
		

		double y = 0;
		double deltatone = 1;

		y += sin (state->phase * deltatone) * 0.8;
		  +  sin (state->phase * deltatone*2) * exp(-1) * 0.8
		  +  sin (state->phase * deltatone*3) * exp(-2) * 0.8;

		
		*buf++ = y*((double)INT16_MAX);
		// printf("%f\n", state->phase);
	}
}