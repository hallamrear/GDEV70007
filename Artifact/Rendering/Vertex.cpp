#include "pch.h"
#include "Vertex.h"
#include <d3d12.h>

Vertex::Vertex()
{
	Position = { 0.0f, 0.0f, 0.0f };
	Normal = { 0.0f, 0.0f, 0.0f };
	Tangent = { 0.0f, 0.0f, 0.0f };
	UV = { 0.0f, 0.0f };
}

Vertex::Vertex(const DirectX::XMFLOAT3& _position, const DirectX::XMFLOAT3& _normal, const DirectX::XMFLOAT3& _tangent, const DirectX::XMFLOAT2& _uv)
{
	Position = _position;
	Normal = _normal;
	Tangent = _tangent;
	UV = _uv;
}

Vertex::~Vertex()
{
	Position = { 0.0f, 0.0f, 0.0f };
	Normal = { 0.0f, 0.0f, 0.0f };
	Tangent = { 0.0f, 0.0f, 0.0f };
	UV = { 0.0f, 0.0f };
}

const UINT Vertex::GetStride()
{
	return sizeof(Vertex);
}

const UINT Vertex::GetOffset()
{
	return 0;
}

void Vertex::GetElementDescription(std::vector<D3D12_INPUT_ELEMENT_DESC>& vectorToFill)
{
	vectorToFill.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Position), D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	vectorToFill.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Normal), D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	vectorToFill.push_back({ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Tangent), D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	vectorToFill.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, UV), D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
}