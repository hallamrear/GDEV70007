#pragma once
#include <DirectXMath.h>

struct D3D12_INPUT_ELEMENT_DESC;

struct Vertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector3 Tangent;
	Vector2 UV;

	Vertex();
	Vertex(const DirectX::XMFLOAT3& _position, const DirectX::XMFLOAT3& _normal, const DirectX::XMFLOAT3& _tangent, const DirectX::XMFLOAT2& _uv);
	~Vertex();

	static const UINT GetStride();
	static const UINT GetOffset();
	static void GetElementDescription(std::vector<D3D12_INPUT_ELEMENT_DESC>& vectorToFill);
};

