#pragma once


/// <summary>
/// Graphics Dependent defines.
/// These may change depending on the final graphics API.
/// </summary>
#ifdef RENDERER_DX12

#include <DirectXMath.h>
typedef DirectX::XMFLOAT4X4 Matrix4x4;
typedef DirectX::XMFLOAT3X3 Matrix3x3;
typedef DirectX::XMFLOAT4 Vector4;
typedef DirectX::XMFLOAT3 Vector3;
typedef DirectX::XMFLOAT2 Vector2;
typedef DirectX::XMFLOAT4 Quaternion;

#include <dxgiformat.h>
#define TEXTURE_FORMAT DXGI_FORMAT
struct ID3D12Device;
#define GraphicsDevice ID3D12Device
#define INPUT_LAYOUT_ELEMENT D3D12_INPUT_ELEMENT_DESC

#include <directx/d3d12.h>
enum MESH_TOPOLOGY
{
	MESH_TOPOLOGY_MODE_POINTS		  = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
	MESH_TOPOLOGY_MODE_LINE			  = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_LINELIST,
	MESH_TOPOLOGY_MODE_LINE_STRIP	  = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
	MESH_TOPOLOGY_MODE_TRIANGLES	  = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
	MESH_TOPOLOGY_MODE_TRIANGLE_STRIP = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
	MESH_TOPOLOGY_MODE_TRIANGLE_FAN	  = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN,
	MESH_TOPOLOGY_UNDEFINED			  =	D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_UNDEFINED
};

#endif

/// <summary>
/// GUID Related Typedefs and Includes
/// </summary>
#include <combaseapi.h>
#include <guiddef.h>
typedef GUID EntityID;


static inline Vector3 operator-(const Vector3& lhs, const Vector3& rhs)
{
	return Vector3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

static inline Vector3 operator-(const Vector3& inverseVec)
{
	return Vector3(-inverseVec.x, -inverseVec.y, -inverseVec.z);
}