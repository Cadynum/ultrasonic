#include "butterworth.h"
#include "constants.h"


const long hz = 1000000/us_per_frame;


unsigned butterworth(bw_state *s, const unsigned breakfreq, const unsigned x) {
	long ret = 0;
	const long dividend = long(breakfreq)*(long(s->xp) + long(x)) - long(s->yp)*(long(breakfreq) - 2*hz);
	if (dividend >= 0) {
		ret = dividend/(long(breakfreq)+2*hz);
	}
    s->yp = ret;
    s->xp = x;
    return ret;
}