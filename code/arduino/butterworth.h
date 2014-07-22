#pragma once

struct bw_state {
    unsigned int yp, xp;
};

unsigned butterworth(bw_state *s, const unsigned breakfreq, const unsigned x);
