#pragma once
#include <Rendering/Geometry/Model.h>
#include <Rendering/Texturing/Texture.h>

namespace tinygltf
{
	class Model;
	struct Mesh;
	struct Primitive;
}

class AssetLoader
{
private:
	friend class Model;
	friend class Mesh;

	static ModelRef CreateModelRefFromGLTF(const tinygltf::Model& gltfModel);
	static bool GetElementDataFromGLTFBuffer(const std::string& attributeName, byte*& data, const tinygltf::Model& model, const tinygltf::Primitive& primitive);
	static bool GetVertexDataFromGLTFPrimative(Mesh& mesh, const tinygltf::Model& gltfModel, const int& meshIndex, const int& primitiveIndex);

public:
	static ModelRef LoadModel(const std::string& assetLocation);
	static TextureRef LoadTexture(const std::string& assetLocation);
};