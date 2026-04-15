#pragma once
#include <Rendering/Texturing/Texture.h>

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

protected:

public:
	Mesh();
	~Mesh();

	const std::vector<TextureRef> GetTextures() const;
};

