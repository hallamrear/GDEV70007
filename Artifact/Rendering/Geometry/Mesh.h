#pragma once
#include <Rendering/Texturing/Texture.h>
#include <Rendering/VertexBuffer.h>
#include <Rendering/IndexBuffer.h>

class Mesh
{
private:
	friend class AssetLoader;
	std::string m_DisplayName;
	Vector3 m_MaxVertex;
	Vector3 m_MinVertex;
	Matrix4x4 m_OffsetMatrix;

	MESH_TOPOLOGY m_Topology;
	std::vector<TextureRef> m_Textures;

	VertexBuffer m_VertexBuffer;
	IndexBuffer m_IndexBuffer;

protected:

public:
	Mesh();
	~Mesh();

	const Matrix4x4& GetOffsetMatrix() const;
	const IndexBuffer& GetIndexBuffer() const;
	const VertexBuffer& GetVertexBuffer() const;
	const std::vector<TextureRef> GetTextures() const;
	const MESH_TOPOLOGY& GetTopologyType() const;
};

