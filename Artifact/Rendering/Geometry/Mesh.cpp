#include "pch.h"
#include "Mesh.h"

Mesh::Mesh()
{
	m_DisplayName = "Unnamed Mesh";
	m_MaxVertex = Vector3(0.0f, 0.0f, 0.0f);
	m_MinVertex = Vector3(0.0f, 0.0f, 0.0f);
	m_OffsetMatrix = Matrix4x4();
	m_Topology = MESH_TOPOLOGY::MESH_TOPOLOGY_UNDEFINED;
	m_Textures = std::vector<TextureRef>();
}

Mesh::~Mesh()
{
	m_DisplayName = "";
	m_MaxVertex = Vector3();
	m_MinVertex = Vector3();
	m_OffsetMatrix = Matrix4x4();
	m_Topology = MESH_TOPOLOGY::MESH_TOPOLOGY_UNDEFINED;

	if (m_Textures.size() >= 0)
	{
		m_Textures.clear();
	}
}

const std::vector<TextureRef> Mesh::GetTextures() const
{
	return m_Textures;
}
