#pragma once
#pragma once

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif // !_USE_MATH_DEFINES

#include <System/Types.h>

constexpr Vector3 BASIS_RIGHT_VECTOR = { 1.0f, 0.0f, 0.0f };
constexpr Vector3 BASIS_UP_VECTOR = { 0.0f, 1.0f, 0.0f };
constexpr Vector3 BASIS_FORWARD_VECTOR = { 0.0f, 0.0f, 1.0f };

#define MAX_DISPLAY_NAME_LENGTH 256

#define VECTOR_W_POSITION 1.0f
#define VECTOR_W_DIRECTION 0.0f

#define MAX_LIGHT_COUNT 32
#define MAX_NUM_ENTITIES 1024
#define MAX_LOADABLE_TEXTURES 64
#define DEFAULT_NEAR_PLANE 0.01f
#define DEFAULT_FAR_PLANE 10000.0f

#define IdentityMatrix Matrix4x4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);