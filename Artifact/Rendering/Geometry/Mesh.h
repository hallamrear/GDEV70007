#pragma once
#include <Rendering/Texturing/Texture.h>
#include <Rendering/VertexBuffer.h>
#include <Rendering/IndexBuffer.h>

class ConvexHull;

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

	ConvexHull* m_ConvexHull;
	VertexBuffer m_VertexBuffer;
	IndexBuffer m_IndexBuffer;
	Vector3 m_Centroid;

protected:

public:
	Mesh();
	~Mesh();

	const ConvexHull* GetConvexHull() const;
	const Vector3& GetCentroid() const;
	const Matrix4x4& GetOffsetMatrix() const;
	const IndexBuffer& GetIndexBuffer() const;
	const VertexBuffer& GetVertexBuffer() const;
	const std::vector<TextureRef>& GetTextures() const;
	std::vector<TextureRef>& GetTextures();
	const MESH_TOPOLOGY& GetTopologyType() const;

	const Vector3& GetMaxVertexLocalSpace() const;
	const Vector3& GetMinVertexLocalSpace() const;
};

