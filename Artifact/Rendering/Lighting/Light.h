#pragma once

enum LIGHT_TYPE : unsigned int
{
	AMBIENT_LIGHT = 0,
	POINT_LIGHT = 1,
	DIRECTIONAL_LIGHT = 2,
	COUNT = 3,
};

static const std::string c_LightTypeNames[LIGHT_TYPE::COUNT] =
{
	"Ambient Light",
	"Point Light",
	"Directional Light",
};

class alignas(16) Light
{
private:

public:
	Light();
	~Light();

	LIGHT_TYPE Type;
	float SpecularStrength;
	int SpecularPower;
	float AmbientStrength;

	Vector4 Position;

	Vector4 Direction;

	Vector4 Colour;

	Vector4 Diffuse;

	Vector4 Attenuation;

	Vector4 Strength;

	BOOL IsEnabled;
	float Range;
	float ConeAngle;
	float Padding;

	const std::string& GetTypeName() const;
};

