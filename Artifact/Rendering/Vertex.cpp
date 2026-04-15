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

Vertex::Vertex(const Vector3& position, const Vector3& normal, const Vector3& tangent, const Vector2& uv)
{
	Position = position;
	Normal = normal;
	Tangent = tangent;
	UV = uv;

}


Vertex::~Vertex()
{
	Position = { 0.0f, 0.0f, 0.0f };
	Normal = { 0.0f, 0.0f, 0.0f };
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

void Vertex::GetElementDescription(std::vector<INPUT_LAYOUT_ELEMENT>& vectorToFill)
{
	vectorToFill.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Position), D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	vectorToFill.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Normal), D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	vectorToFill.push_back({ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Tangent), D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	vectorToFill.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, UV), D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
}