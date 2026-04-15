#pragma once
#include <Rendering/Geometry/Model.h>
#include <Rendering/Texturing/Texture.h>

namespace tinygltf
{
	class Model;
	struct Mesh;
	struct Primitive;
	class Node;
}

class AssetLoader
{
private:
	static int m_TextureCounter;
	friend class Model;
	friend class Mesh;

	static bool LoadTexturesFromGLTFPrimitive(Mesh& mesh, const tinygltf::Model& gltfModel, const int& meshIndex, const int& primitiveIndex);
	static ModelRef CreateModelRefFromGLTF(const tinygltf::Model& gltfModel);
	static bool SetMeshMatrixFromFile(Matrix4x4& offsetMatrix, const tinygltf::Node& node);
	static bool GetElementDataFromGLTFBuffer(const std::string& attributeName, byte*& data, const tinygltf::Model& model, const tinygltf::Primitive& primitive);
	static bool GetVertexDataFromGLTFPrimitive(Mesh& mesh, const tinygltf::Model& gltfModel, const int& meshIndex, const int& primitiveIndex);

public:
	static ModelRef LoadModel(const std::string& assetLocation);
	static TextureRef LoadTexture(const std::string& assetLocation);
	static TextureRef LoadTexture(const byte* buffer, const size_t& bufferLength);
};