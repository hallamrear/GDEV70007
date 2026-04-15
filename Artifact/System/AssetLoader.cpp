#include "pch.h"
#include "AssetLoader.h"
#include <Rendering/Vertex.h>
#include <Rendering/Geometry/Mesh.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

ModelRef AssetLoader::LoadModel(const std::string& assetLocation)
{
	std::filesystem::path path = (assetLocation);

	if (std::filesystem::exists(path) == false)
	{
		return nullptr;
	}

	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool ret = false;

	if (path.extension() == ".glb")
	{
		ret = loader.LoadBinaryFromFile(&model, &err, &warn, assetLocation); // for binary glTF(.glb)
	}
	else if (path.extension() == ".gltf")
	{
		ret = loader.LoadASCIIFromFile(&model, &err, &warn, assetLocation);
	}
	else
	{
		printf("Trying to load an invalid gltf file: %s\n", assetLocation.c_str());
		return nullptr;
	}

	if (!warn.empty())
	{
		printf("Warn: %s\n", warn.c_str());
	}

	if (!err.empty())
	{
		printf("Err: %s\n", err.c_str());
	}

	if (!ret) 
	{
		printf("Failed to parse glTF: %s\n", assetLocation.c_str());
	}

	ModelRef ref = CreateModelRefFromGLTF(model);

	if (ref != nullptr && ref->IsLoaded())
	{
		ref->m_DisplayName = path.filename().string();
		ref->m_FileLocation = assetLocation;
	}

	return ref;
}

TextureRef AssetLoader::LoadTexture(const std::string& assetLocation)
{
	UNREFERENCED_PARAMETER(assetLocation);
	return TextureRef();
}

ModelRef AssetLoader::CreateModelRefFromGLTF(const tinygltf::Model& gltfModel)
{
	ModelRef myModel = std::make_shared<Model>();
	
	size_t nodeCount = gltfModel.nodes.size();

	for (size_t i = 0; i < nodeCount; i++)
	{
		const tinygltf::Node& node = gltfModel.nodes[i];
		int meshIndex = node.mesh;
		
		if (meshIndex < 0)
			continue;

		const tinygltf::Mesh& gltfMesh = gltfModel.meshes[meshIndex];
		const size_t primitiveCount = gltfMesh.primitives.size();

		for (size_t primitiveIndex = 0; primitiveIndex < primitiveCount; primitiveIndex++)
		{
			Mesh* mesh = new Mesh();

			if(GetVertexDataFromGLTFPrimative(*mesh, gltfModel, meshIndex, (int)primitiveIndex))
			{
				myModel->m_Meshes.push_back(mesh);

				if (node.matrix.size() > 0)
				{
					float matrix[16] = { 0.0f };

					for (size_t m = 0; m < 16; i++)
					{
						matrix[m] = (float)node.matrix[m];
					}

					mesh->m_OffsetMatrix = Matrix4x4();
					DirectX::XMStoreFloat4x4(&mesh->m_OffsetMatrix, DirectX::XMMatrixTranspose(XMLoadFloat4x4(&mesh->m_OffsetMatrix)));
				}
				else //SRT, Construct Matrix myself
				{
					Vector3 translation = { 0.0f, 0.0f, 0.0f };

					if (node.translation.size() == 3)
					{
						translation.x = (float)node.translation[0];
						translation.y = (float)node.translation[1];
						translation.z = (float)node.translation[2];
					}

					Vector4 quaternion = { 0.0f, 0.0f, 0.0f, 1.0f };

					if (node.rotation.size() > 0)
					{
						quaternion.x = (float)node.rotation[0];
						quaternion.y = (float)node.rotation[1];
						quaternion.z = (float)node.rotation[2];
						quaternion.w = (float)node.rotation[3];
					}

					Vector3 scale = { 1.0f, 1.0f, 1.0f };

					if (node.scale.size() == 3)
					{
						scale.x = (float)node.scale[0];
						scale.y = (float)node.scale[1];
						scale.z = (float)node.scale[2];
					}

					DirectX::XMStoreFloat4x4(&mesh->m_OffsetMatrix,
						DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z) *
						DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&quaternion)) *
						DirectX::XMMatrixScaling(scale.x, scale.y, scale.z));
				}
			}
			else
			{
				delete mesh;
				mesh = nullptr;
			}
		}
	}

	if (myModel->IsLoaded() == false)
	{
		//delete myModel;
		myModel = nullptr;
	}

	return myModel;
}

bool AssetLoader::GetElementDataFromGLTFBuffer(const std::string& attributeName, byte*& data, const tinygltf::Model& model, const tinygltf::Primitive& primitive)
{
	bool hasAttribute = (primitive.attributes.find(attributeName) != primitive.attributes.end());

	if (hasAttribute)
	{
		const int& attributeAccessorIndex = primitive.attributes.at(attributeName);
		const tinygltf::Accessor& attributeAccessor = model.accessors[attributeAccessorIndex];
		const tinygltf::BufferView& attributeBufferView = model.bufferViews[attributeAccessor.bufferView];
		const tinygltf::Buffer& attributeBuffer = model.buffers[attributeBufferView.buffer];
		size_t elementSize = tinygltf::GetComponentSizeInBytes(attributeAccessor.componentType) * tinygltf::GetNumComponentsInType(attributeAccessor.type);
		data = new byte[elementSize * attributeAccessor.count];
		memset(data, 0, elementSize * attributeAccessor.count * sizeof(byte));
		void* src = (void*)(attributeBuffer.data.data() + attributeBufferView.byteOffset + attributeAccessor.byteOffset);
		memcpy_s(data, elementSize * attributeAccessor.count, src, attributeBufferView.byteLength);
	}

	return hasAttribute;
}


bool AssetLoader::GetVertexDataFromGLTFPrimative(Mesh& mesh, const tinygltf::Model& gltfModel, const int& meshIndex, const int& primitiveIndex)
{
	const tinygltf::Mesh& gltfMesh = gltfModel.meshes[meshIndex];
	const tinygltf::Primitive& primitive = gltfMesh.primitives[primitiveIndex];

	if (primitive.attributes.size() <= 0)
	{
		printf("No primitivate attribute data in gltf file.\n");
		return false;
	}

	MESH_TOPOLOGY foundTopology = MESH_TOPOLOGY::MESH_TOPOLOGY_UNDEFINED;

	switch (primitive.mode)
	{
	case TINYGLTF_MODE_POINTS: { foundTopology = MESH_TOPOLOGY::MESH_TOPOLOGY_MODE_POINTS; } break;
	case TINYGLTF_MODE_LINE: { foundTopology = MESH_TOPOLOGY::MESH_TOPOLOGY_MODE_LINE; } break;
	case TINYGLTF_MODE_LINE_STRIP: { foundTopology = MESH_TOPOLOGY::MESH_TOPOLOGY_MODE_LINE_STRIP; } break;
	case TINYGLTF_MODE_TRIANGLES: { foundTopology = MESH_TOPOLOGY::MESH_TOPOLOGY_MODE_TRIANGLES; } break;
	case TINYGLTF_MODE_TRIANGLE_STRIP: { foundTopology = MESH_TOPOLOGY::MESH_TOPOLOGY_MODE_TRIANGLE_STRIP; } break;
	case TINYGLTF_MODE_TRIANGLE_FAN: { foundTopology = MESH_TOPOLOGY::MESH_TOPOLOGY_MODE_TRIANGLE_FAN; } break;

	case TINYGLTF_MODE_LINE_LOOP:
	default:
		printf("Unsupported topology in GLTF file.\n");
		return false;
		break;
	}

	int positionAttributionAccessor = primitive.attributes.at("POSITION");
	tinygltf::Accessor positions = gltfModel.accessors[positionAttributionAccessor];

	byte* positionData = nullptr;
	byte* normalData = nullptr;
	byte* tangentData = nullptr;
	byte* texCoordData = nullptr;

	bool hasPositions = GetElementDataFromGLTFBuffer("POSITION", positionData, gltfModel, primitive);
	bool hasNormals = GetElementDataFromGLTFBuffer("NORMAL", normalData, gltfModel, primitive);
	bool hasTangents = GetElementDataFromGLTFBuffer("TANGENT", tangentData, gltfModel, primitive);
	bool hasTexCoords = GetElementDataFromGLTFBuffer("TEXCOORD_0", texCoordData, gltfModel, primitive);

	std::vector<Vertex> vertices = std::vector<Vertex>();
	size_t vertexCount = positions.count;
	vertices.resize(vertexCount);

	for (size_t v = 0; v < vertexCount; v++)
	{
		vertices[v].Position =	hasPositions ? *(Vector3*)(positionData + (sizeof(Vector3) * v)) : Vector3();
		vertices[v].Normal =	hasNormals	 ? *(Vector3*)(  normalData + (sizeof(Vector3) * v)) : Vector3();
		vertices[v].Tangent =	hasTangents  ? *(Vector3*)( tangentData + (sizeof(Vector3) * v)) : Vector3();
		vertices[v].UV =		hasTexCoords ? *(Vector2*)(texCoordData + (sizeof(Vector2) * v)) : Vector2();
	}

	mesh.m_MaxVertex = Vector3((float)positions.maxValues[0], (float)positions.maxValues[1], (float)positions.maxValues[2]);
	mesh.m_MinVertex = Vector3((float)positions.minValues[0], (float)positions.minValues[1], (float)positions.minValues[2]);
	mesh.m_Topology = foundTopology;
	mesh.m_DisplayName = gltfMesh.name;

	return true;
}