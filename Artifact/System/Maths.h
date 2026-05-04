#pragma once
#include <math.h>

#define DEGREES_TO_RADIANS (float)(M_PI / 180.0f)
#define RADIANS_TO_DEGREES (float)(180.0f / M_PI)

namespace Maths
{
	static inline unsigned int RoundToNearestBaseTwo(const unsigned int& integer)
	{
		unsigned int value = integer;
		value--;
		value |= value >> 1;
		value |= value >> 2;
		value |= value >> 4;
		value |= value >> 8;
		value |= value >> 16;
		value++;
		return value;
	}
};

