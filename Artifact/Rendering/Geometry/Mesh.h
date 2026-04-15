#pragma once

class Mesh
{
private:
	friend class AssetLoader;
	std::string m_DisplayName;
	Vector3 m_MaxVertex;
	Vector3 m_MinVertex;
	Matrix4x4 m_OffsetMatrix;

	MESH_TOPOLOGY m_Topology;

protected:

public:
	Mesh();
	~Mesh();

};

