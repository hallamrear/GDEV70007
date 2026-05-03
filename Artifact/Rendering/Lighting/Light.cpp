#include "pch.h"
#include "Light.h"

Light::Light()
{
	Type = LIGHT_TYPE::AMBIENT_LIGHT;
	IsEnabled = false;
	AmbientStrength = 0.0f;
	Position = Vector3(0.0f, 0.0f, 0.0f);
	Direction = Vector3(0.0f, 0.0f, 0.0f);
	Colour = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	InnerCutoff = 0.0f;
	OuterCutoff = 0.0f;
	Diffuse = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	SpecularStrength = 0.0f;
	SpecularPower = 0.0f;
	Attenuation = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	Strength = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	
	memset(&Padding, 0, sizeof(bool) * _countof(Padding));
}

Light::~Light()
{
    IsEnabled = false;
}

const std::string& Light::GetTypeName() const
{
    return c_LightTypeNames[Type];
}