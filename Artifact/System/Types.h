#pragma once

#include <DirectXMath.h>

/// <summary>
/// Graphics Dependent defines.
/// These may change depending on the final graphics API.
/// </summary>
typedef DirectX::XMFLOAT4X4 Matrix4x4;
typedef DirectX::XMFLOAT4 Vector4;
typedef DirectX::XMFLOAT3 Vector3;
typedef DirectX::XMFLOAT4 Quaternion;

/// <summary>
/// GUID Related Typedefs and Includes
/// </summary>
#include <combaseapi.h>
#include <guiddef.h>
typedef GUID EntityID;