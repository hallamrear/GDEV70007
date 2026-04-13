#pragma once


/// <summary>
/// Graphics Dependent defines.
/// These may change depending on the final graphics API.
/// </summary>
#ifdef RENDERER_DX12

#include <DirectXMath.h>
typedef DirectX::XMFLOAT4X4 Matrix4x4;
typedef DirectX::XMFLOAT4 Vector4;
typedef DirectX::XMFLOAT3 Vector3;
typedef DirectX::XMFLOAT2 Vector2;
typedef DirectX::XMFLOAT4 Quaternion;

#include <dxgiformat.h>
#define TEXTURE_FORMAT DXGI_FORMAT
struct ID3D12Device;
#define GraphicsDevice ID3D12Device

#endif

/// <summary>
/// GUID Related Typedefs and Includes
/// </summary>
#include <combaseapi.h>
#include <guiddef.h>
typedef GUID EntityID;