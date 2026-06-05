#pragma once
#include <glm/glm.hpp>
#include <Physics/Resolution/ContraintContact.h>

struct Constraint
{
	float PositionalError = 0.0f;
	float VelocityError = 0.0f;
	float EffectiveMass = 0.0f;
	float ImpulseSum = 0.0f;
	glm::vec2 ImpulseBounds = { 0.0f, 0.0f };
};

