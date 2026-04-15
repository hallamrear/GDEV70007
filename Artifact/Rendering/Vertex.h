#pragma once

struct Vertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector3 Tangent;
	Vector2 UV;

	Vertex();
	Vertex(const Vector3& position, const Vector3& normal, const Vector3& _tangent, const Vector2& uv);
	~Vertex();

	static const UINT GetStride();
	static const UINT GetOffset();
	static void GetElementDescription(std::vector<INPUT_LAYOUT_ELEMENT>& vectorToFill);
};

