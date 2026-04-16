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

	if (m_VertexBuffer.IsLoaded())
	{
		m_VertexBuffer.Destroy();
	}

	if (m_IndexBuffer.IsLoaded())
	{
		m_IndexBuffer.Destroy();
	}
}

const Matrix4x4& Mesh::GetOffsetMatrix() const
{
	return m_OffsetMatrix;
}

const IndexBuffer& Mesh::GetIndexBuffer() const
{
	return m_IndexBuffer;
}

const VertexBuffer& Mesh::GetVertexBuffer() const
{
	return m_VertexBuffer;
}

const std::vector<TextureRef> Mesh::GetTextures() const
{
	return m_Textures;
}

const MESH_TOPOLOGY& Mesh::GetTopologyType() const
{
	return m_Topology;
}
