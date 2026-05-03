#pragma once

enum LIGHT_TYPE : int
{
	AMBIENT_LIGHT = 0,
	POINT_LIGHT = 1,
	DIRECTIONAL_LIGHT = 2,
	SPOT_LIGHT = 3,
	COUNT = 4,
};

static const std::string c_LightTypeNames[LIGHT_TYPE::COUNT] =
{
	"Ambient Light",
	"Point Light",
	"Directional Light",
	"Spot Light"
};

class alignas(16) Light
{
private:

public:
	Light();
	~Light();

	LIGHT_TYPE Type;
	float SpecularStrength;
	float SpecularPower;
	float AmbientStrength;
	Vector3 Position;
	float InnerCutoff;
	float OuterCutoff;
	Vector3 Direction;
	Vector4 Colour;
	Vector4 Diffuse;
	Vector4 Attenuation;
	Vector4 Strength;
	bool IsEnabled;
	bool Padding[15];

	const std::string& GetTypeName() const;
};

