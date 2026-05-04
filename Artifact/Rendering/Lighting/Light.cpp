#include "pch.h"
#include "Light.h"
#include <System/Maths.h>

Light::Light()
{
	Type = LIGHT_TYPE::AMBIENT_LIGHT;
	IsEnabled = false;
	AmbientStrength = 0.2f;
	Position = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	Direction = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	Colour = Vector4(0.3f, 0.3f, 0.3f, 0.3f);
	Range = 10.0f;
	ConeAngle = 20.0f;
	Diffuse = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	SpecularStrength = 0.5f;
	SpecularPower = 1;
	Attenuation = Vector4(1.0f, 0.09f, 0.032f, 0.0f);
	Strength = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	Padding = 0;
}

Light::~Light()
{
    IsEnabled = false;
}

const std::string& Light::GetTypeName() const
{
    return c_LightTypeNames[Type];
}