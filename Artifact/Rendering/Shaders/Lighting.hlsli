#ifndef __LIGHTING_HLSL__
#define __LIGHTING_HLSL__
#include "Structures.hlsli"

#define LIGHT_AMBIENT 0
#define LIGHT_POINT 1
#define LIGHT_DIRECTIONAL 2
#define LIGHT_SPOT 3

float CalculateAttenuation(float distance, float constantAtt, float linearAtt, float quadraticAtt)
{
    return 1.0f / (constantAtt + (linearAtt * distance) + (quadraticAtt * (distance * distance)));
}

//Lambert's Cosine Law
float CosineLaw(float3 toLightVector, float3 normal)
{
    return saturate(dot(normalize(normal), normalize(toLightVector)));
}

float4 CalculateDirectionalLighting(Light light, float4 sampleColour, float3 worldPosition, float3 normal)
{
    float3 toLight = normalize(-light.Direction.xyz);
    
    float NdotL = CosineLaw(normalize(toLight), normalize(normal));
    float3 diffuse = light.Colour * NdotL * sampleColour;
    
    float3 toEye = normalize(CameraPosition.xyz - worldPosition.xyz);    
    float3 halfway = normalize(toEye + toLight);
    float NdotH = CosineLaw(normalize(halfway), normalize(normal));
    float3 specular = light.SpecularStrength * pow(NdotH, light.SpecularPower) * light.Colour;

    return float4(saturate(diffuse + specular), sampleColour.a);
}

float4 CalculatePointLighting(Light light, float4 sampleColour, float3 worldPosition, float3 normal)
{
    float3 toLight = light.Position.xyz - worldPosition;

    float NdotL = CosineLaw(normalize(toLight), normalize(normal));
    float3 diffuse = light.Colour * NdotL * sampleColour;
    
    return float4(saturate(diffuse), sampleColour.a);
    
    float3 toEye = normalize(CameraPosition.xyz - worldPosition.xyz);    
    float3 halfway = normalize(toEye + toLight);
    float NdotH = CosineLaw(normalize(halfway), normalize(normal));
    float3 specular = light.SpecularStrength * pow(NdotH, light.SpecularPower) * light.Colour;

    float distance = length(toLight);
    float attenuation = CalculateAttenuation(distance, light.Attenuation.x, light.Attenuation.y, light.Attenuation.z);

    diffuse *= attenuation;
    specular *= attenuation;    

    return float4(saturate(diffuse + specular), sampleColour.a);
}

float4 CalculateSpotLight(Light light, float4 sampleColour, float3 worldPosition, float3 normal)
{
    float3 toLight = light.Position.xyz - worldPosition;
    float distance = length(toLight);
    if (distance > light.Range)
        return float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float NdotL = CosineLaw(normalize(toLight), normalize(normal));
    float3 diffuse = light.Colour * NdotL * sampleColour;

    float3 toEye = normalize(CameraPosition.xyz - worldPosition.xyz);    
    float3 halfway = normalize(toEye + toLight);
    float NdotH = CosineLaw(normalize(halfway), normalize(normal));
    float3 specular = light.SpecularStrength * pow(NdotH, light.SpecularPower) * light.Colour;
    
    //Calculate falloff from center to edge of pointlight cone
    float falloff = pow(max(dot(-toLight, normalize(light.Direction.xyz)), 0.0f), light.ConeAngle);

    float attenuation = CalculateAttenuation(distance, light.Attenuation.x, light.Attenuation.y, light.Attenuation.z);

    diffuse *= attenuation;
    specular *= attenuation;    

    return float4(saturate(diffuse + specular), sampleColour.a);
}

float4 CalculateLighting(float4 sampleColour, float3 worldPosition, float3 normal)
{
    float4 lightColour = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
	for (int i = 0; i < MAX_LIGHT_COUNT; i++)
    {
        Light light = LightData[i];
      
        if(light.IsEnabled == false)
            continue;

        switch (light.Type)
        {
            case LIGHT_AMBIENT:
                lightColour += light.AmbientStrength * light.Colour;
                break;

            case LIGHT_DIRECTIONAL:
                lightColour += CalculateDirectionalLighting(light, sampleColour, worldPosition, normal);
                break;
          
            case LIGHT_POINT:
                lightColour += CalculatePointLighting(light, sampleColour, worldPosition, normal);
                break;
          
            case LIGHT_SPOT:
                lightColour += CalculateSpotLight(light, sampleColour, worldPosition, normal);
                break;            
          
            default:
                break;
        }
    }

	return lightColour;
}

#endif //__LIGHTING_HLSL__