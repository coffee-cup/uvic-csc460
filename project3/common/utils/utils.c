#include "utils.h"

long map_u(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

long constrain_u(long x, long min, long max) {
    if (x < min) return min;
    else if (x > max) return max;
    return x;
}
