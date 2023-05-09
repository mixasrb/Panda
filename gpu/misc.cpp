#include "misc.hpp"

int16_t psx::gpu::min(int16_t* p, const uint8_t count) {
	int16_t min = *p;
	for (int i = 1; i < count; i++)
		if (p[i] < min)
			min = p[i];
	return min;
}

int16_t psx::gpu::max(int16_t* p, const uint8_t count) {
	int16_t max = *p;
	for (int i = 1; i < count; i++)
		if (p[i] > max)
			max = p[i];
	return max;
}
