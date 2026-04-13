#pragma once
#pragma once

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif // !_USE_MATH_DEFINES

#include <math.h>
#include <System/Types.h>

#define DEGREES_TO_RADIANS (float)(M_PI * 180.0f)
#define RADIANS_TO_DEGREES (float)(180.0f / M_PI)

constexpr Vector3 BASIS_RIGHT_VECTOR = { 1.0f, 0.0f, 0.0f };
constexpr Vector3 BASIS_UP_VECTOR = { 0.0f, 1.0f, 0.0f };
constexpr Vector3 BASIS_FORWARD_VECTOR = { 0.0f, 0.0f, 1.0f };

#define MAX_NUM_ENTITIES 1024
#define MAX_LOADABLE_TEXTURES 64
#define DEFAULT_NEAR_PLANE 0.01f
#define DEFAULT_FAR_PLANE 10000.0f