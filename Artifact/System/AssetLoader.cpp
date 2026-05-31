#include "pch.h"
#include "AssetLoader.h"
#include <Rendering/Renderer.h>
#include <Rendering/Vertex.h>
#include <Rendering/Geometry/Mesh.h>
#include <Physics/Quickhull/Quickhull.h>
#include <System/AssetManagement.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

int AssetLoader::m_TextureCounter = 0;

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
	Renderer* renderer = ServiceLocator::Locate<Renderer>();

	if (renderer == nullptr)
	{
		printf("Failed to load texture due to invalid renderer reference.\n");
		return nullptr;
	}

	if (std::filesystem::exists(assetLocation) == false)
	{
		printf("Failed to create texture. Invalid file location.\n");
		return nullptr;
	}

	int w = 0;
	int h = 0;
	int componentCount = 0;

	stbi_uc* pixelBuffer = stbi_load(assetLocation.c_str(), &w, &h, &componentCount, 4);
	size_t pixelBufferSize = sizeof(char) * w * h * componentCount;

	if (pixelBuffer == nullptr)
	{
		printf("Failed to load texture from file: %s\n", assetLocation.c_str());
		return nullptr;
	}

	TextureRef texture = renderer->BindTextureData(m_TextureCounter, pixelBuffer, pixelBufferSize, Vector2((float)w, (float)h));

	if (texture != nullptr)
	{
		m_TextureCounter++;
		texture->m_FileLocation = assetLocation;
	}
	else
	{
		printf("Renderer failed to create texture graphics object.\n");
	}

	delete[] pixelBuffer;
	pixelBuffer = nullptr;

	return texture;
}

TextureRef AssetLoader::LoadTexture(const byte* buffer, const size_t& bufferLength)
{
	Renderer* renderer = ServiceLocator::Locate<Renderer>();

	if (renderer == nullptr)
	{
		printf("Failed to load texture due to invalid renderer reference.\n");
		return nullptr;
	}

	if (buffer == nullptr || bufferLength <= 0)
	{
		printf("Failed to bind texture. Invalid data or buffer length.\n");
		return nullptr;
	}

	int w = 0;
	int h = 0;
	int componentCount = 0;

	stbi_uc* pixelBuffer = stbi_load_from_memory(buffer, (int)bufferLength, &w, &h, &componentCount, 4);

	if (pixelBuffer == nullptr)
	{
		printf("Failed to load texture from memory.\n");
		return nullptr;
	}

	TextureRef texture = renderer->BindTextureData(m_TextureCounter, pixelBuffer, bufferLength, Vector2((float)w, (float)h));

	if (texture != nullptr)
	{
		m_TextureCounter++;
		texture->m_FileLocation = "Loaded from gltf file.\n";
	}
	else
	{
		printf("Renderer failed to create texture graphics object.\n");
	}

	delete[] pixelBuffer;
	pixelBuffer = nullptr;

	return texture;
}

bool AssetLoader::LoadTexturesFromGLTFPrimitive(Mesh& mesh, const tinygltf::Model& gltfModel, const int& meshIndex, const int& primitiveIndex)
{
	const tinygltf::Mesh& gltfMesh = gltfModel.meshes[meshIndex];
	const tinygltf::Primitive& primitive = gltfMesh.primitives[primitiveIndex];

	int materialIndex = primitive.material;

	if (materialIndex < 0)
	{
		printf("No material data therefore no texture to load. This is fine.\n");
		return true;
	}

	const tinygltf::Material material = gltfModel.materials[primitive.material];

	int textureIndex = material.pbrMetallicRoughness.baseColorTexture.index;

	if (textureIndex < 0)
	{
		printf("No textures to load. This is fine.\n");
		return true;
	}

	const tinygltf::Texture& gltfTexture = gltfModel.textures[textureIndex];
	int textureImageIndex = gltfTexture.source;

	const tinygltf::Image textureImage = gltfModel.images[textureImageIndex];
	std::string textureName = textureImage.name;

	if (textureImage.uri == "" && textureImage.bufferView < 0)
	{
		printf("Error fetching texture from gltf file.\n");
		return false;
	}

	TextureRef texture = nullptr;

	AssetManager* assetManager = ServiceLocator::Locate<AssetManager>();

	if (assetManager != nullptr)
	{
		texture = assetManager->GetTexture(textureName);
	}

	if (texture == nullptr)
	{
		if (textureImage.uri != "")
		{
			texture = LoadTexture(textureImage.uri);
		}
		else
		{
			int textureBufferViewIndex = textureImage.bufferView;
			const tinygltf::BufferView textureBufferView = gltfModel.bufferViews[textureBufferViewIndex];
			const tinygltf::Buffer& textureImageBuffer = gltfModel.buffers[textureBufferView.buffer];

			if (textureImage.mimeType == "image/jpeg")
			{
				const byte* data = &textureImageBuffer.data[textureBufferView.byteOffset];
				texture = LoadTexture(data, textureBufferView.byteLength);
			}
			else if (textureImage.mimeType == "image/png")
			{
				const byte* data = &textureImageBuffer.data[textureBufferView.byteOffset];
				texture = LoadTexture(data, textureBufferView.byteLength);
			}
			else
			{
				printf("Error fetching texture from gltf file. Invalid MIME type.\n");
				return false;
			}
		}
	}

	if (texture != nullptr)
	{
		texture->m_DisplayName = textureName;
		mesh.m_Textures.push_back(texture);
	}

	return true;
}

ModelRef AssetLoader::CreateModelRefFromGLTF(const tinygltf::Model& gltfModel)
{
	ModelRef myModel = std::make_shared<Model>();
	
	size_t nodeCount = gltfModel.nodes.size();

	for (size_t n = 0; n < nodeCount; n++)
	{
		const tinygltf::Node& node = gltfModel.nodes[n];
		int meshIndex = node.mesh;
		
		if (meshIndex < 0)
			continue;

		const tinygltf::Mesh& gltfMesh = gltfModel.meshes[meshIndex];
		const size_t primitiveCount = gltfMesh.primitives.size();

		for (size_t primitiveIndex = 0; primitiveIndex < primitiveCount; primitiveIndex++)
		{
			Mesh* mesh = new Mesh();

			if(GetVertexDataFromGLTFPrimitive(*mesh, gltfModel, meshIndex, (int)primitiveIndex))
			{
				myModel->m_Meshes.push_back(mesh);

				bool usesIndices = GetIndexDataFromGLTFPrimitive(*mesh, gltfModel, meshIndex, (int)primitiveIndex);

				if (!usesIndices)
				{
					printf("Model loaded with indices.\n");
				}

				SetMeshMatrixFromFile(mesh->m_OffsetMatrix, node);
				LoadTexturesFromGLTFPrimitive(*mesh, gltfModel, meshIndex, (int)primitiveIndex);
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

bool AssetLoader::SetMeshMatrixFromFile(Matrix4x4& offsetMatrix, const tinygltf::Node& node)
{
	if (node.matrix.size() > 0)
	{
		float matrix[16] = { 0.0f };

		for (size_t m = 0; m < 16; m++)
		{
			matrix[m] = (float)node.matrix[m];
		}

		offsetMatrix = Matrix4x4();
		XMStoreFloat4x4(&offsetMatrix, XMMatrixTranspose(XMLoadFloat4x4(&offsetMatrix)));
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

		XMStoreFloat4x4(&offsetMatrix,
			DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z) *
			DirectX::XMMatrixRotationQuaternion(XMLoadFloat4(&quaternion)) *
			DirectX::XMMatrixScaling(scale.x, scale.y, scale.z));
	}

	return true;
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

bool AssetLoader::GetIndexDataFromGLTFPrimitive(Mesh& mesh, const tinygltf::Model& gltfModel, const int& meshIndex, const int& primitiveIndex)
{
	Renderer* renderer = ServiceLocator::Locate<Renderer>();

	if (renderer == nullptr)
	{
		printf("Failed to load vertex data due to invalid renderer reference.\n");
		return false;
	}

	const tinygltf::Mesh& gltfMesh = gltfModel.meshes[meshIndex];
	const tinygltf::Primitive& primitive = gltfMesh.primitives[primitiveIndex];

	if (primitive.attributes.size() <= 0)
	{
		printf("No primitivate attribute data in gltf file.\n");
		return false;
	}

	int indicesAttributor = primitive.indices;
	const tinygltf::Accessor& indicesAccessor = gltfModel.accessors[indicesAttributor];

	bool isShortIndex = false;

	switch (indicesAccessor.componentType)
	{
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
		isShortIndex = true;
		break;
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
		break;

	default:
		printf("Undefined index format in gltf file.\n");
		return false;
		break;
	}

	int indexCount = (int)indicesAccessor.count;
	const tinygltf::BufferView& indexBufferView = gltfModel.bufferViews[indicesAccessor.bufferView];
	const tinygltf::Buffer& indexBuffer = gltfModel.buffers[indexBufferView.buffer];
	int indicesByteStride = indicesAccessor.ByteStride(indexBufferView);

	if (indicesByteStride == -1)
	{
		printf("Failed to get buffer view stride.\n");
		return false;
	}

	if (isShortIndex)
	{
		if (indicesByteStride != sizeof(unsigned short))
		{
			printf("ByteStride from buffer view and expected index size do not match.\n");
			return false;
		}
	}
	else
	{
		if (indicesByteStride != sizeof(unsigned int))
		{
			printf("ByteStride from buffer view and expected index size do not match.\n");
			return false;
		}
	}

	size_t elementSize = tinygltf::GetComponentSizeInBytes(indicesAccessor.componentType) * tinygltf::GetNumComponentsInType(indicesAccessor.type);

	if (elementSize <= 0)
	{
		printf("Index data elemet size evaluated to zero.\n");
		return false;
	}

	const void* src = indexBuffer.data.data() + indexBufferView.byteOffset + indicesAccessor.byteOffset;

	size_t indexDataSize = indexCount * elementSize;
	std::vector<byte> indexData = std::vector<byte>();
	indexData.resize(indexDataSize);
	memcpy(indexData.data(), src, indexDataSize);

	bool boundIndexData = renderer->BindIndexData(mesh.m_IndexBuffer, indexData.data(), indexDataSize, isShortIndex);

	if (boundIndexData == false)
	{
		printf("Failed to bind index data during model loading.\n");
		return false;
	}

	std::wstring indexBufferName = std::wstring(gltfMesh.name.begin(), gltfMesh.name.end()) + L" Index Buffer";
	mesh.m_IndexBuffer.GetResource()->SetName(indexBufferName.c_str());

	return true;
}

bool AssetLoader::GetVertexDataFromGLTFPrimitive(Mesh& mesh, const tinygltf::Model& gltfModel, const int& meshIndex, const int& primitiveIndex)
{
	Renderer* renderer = ServiceLocator::Locate<Renderer>();

	if (renderer == nullptr)
	{
		printf("Failed to load vertex data due to invalid renderer reference.\n");
		return false;
	}

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

	Vector3 centroid = Vector3(0.0f, 0.0f, 0.0f);

	std::vector<Vertex> vertices = std::vector<Vertex>();
	size_t vertexCount = positions.count;
	vertices.resize(vertexCount);

	std::vector<Vector3> points = std::vector<Vector3>();
	points.resize(vertexCount);

	for (size_t v = 0; v < vertexCount; v++)
	{
		vertices[v].Position =	hasPositions ? *(Vector3*)(positionData + (sizeof(Vector3) * v)) : Vector3();
		vertices[v].Normal =	hasNormals	 ? *(Vector3*)(  normalData + (sizeof(Vector3) * v)) : Vector3();
		vertices[v].Tangent =	hasTangents  ? *(Vector3*)( tangentData + (sizeof(Vector3) * v)) : Vector3();
		vertices[v].UV =		hasTexCoords ? *(Vector2*)(texCoordData + (sizeof(Vector2) * v)) : Vector2();

		points[v] = vertices[v].Position;
		centroid.x += points[v].x;
		centroid.y += points[v].y;
		centroid.z += points[v].z;
	}	

	centroid.x /= vertexCount;
	centroid.y /= vertexCount;
	centroid.z /= vertexCount;

	size_t vertexBufferLength = sizeof(Vertex) * vertices.size();
	
	bool boundVertexData = renderer->BindVertexData(mesh.m_VertexBuffer, vertices.data(), vertexBufferLength);

	if (boundVertexData == false)
	{
		printf("Failed to bind vertex data during model loading.\n");
		return false;
	}

	if (mesh.m_VertexBuffer.GetResource() == nullptr)
	{
		printf("Failed to create vertex buffer resource during model loading.\n");
		return false;
	}

	std::wstring vertexBufferName = std::wstring(gltfMesh.name.begin(), gltfMesh.name.end()) + L" Vertex Buffer";
	mesh.m_VertexBuffer.GetResource()->SetName(vertexBufferName.c_str());
	mesh.m_MaxVertex = Vector3((float)positions.maxValues[0], (float)positions.maxValues[1], (float)positions.maxValues[2]);
	mesh.m_MinVertex = Vector3((float)positions.minValues[0], (float)positions.minValues[1], (float)positions.minValues[2]);
	mesh.m_Topology = foundTopology;
	mesh.m_DisplayName = gltfMesh.name;

	if (points.size() > 0)
	{
		std::vector<Vector3> pointCloud
		{
		 {0.000000f, -1.25000f, -1.000000f},
		 {0.000000f, -1.25000f, -1.000000f},
		 {0.000000f, -1.25000f, -1.000000f},
		 {0.000000f, 1.250000f,-1.000000f},
		 {0.000000f, 1.250000f,-1.000000f},
		 {0.000000f, 1.250000f,-1.000000f},
		 {0.587785f, -1.25000f, -0.809017f},
		 {0.587785f, -1.25000f, -0.809017f},
		 {0.587785f, -1.25000f, -0.809017f},
		 {0.587785f, 1.250000f, -0.809017f},
		 {0.587785f, 1.250000f, -0.809017f},
		 {0.587785f, 1.250000f, -0.809017f},
		 {0.951057f, -1.25000f, -0.309017f},
		 {0.951057f, -1.25000f, -0.309017f},
		 {0.951057f, -1.25000f, -0.309017f},
		 {0.951057f, 1.250000f, -0.309017f},
		 {0.951057f, 1.250000f, -0.309017f},
		 {0.951057f, 1.250000f, -0.309017f},
		 {0.951056f, -1.25000f, 0.309017f},
		 {0.951056f, -1.25000f, 0.309017f},
		 {0.951056f, -1.25000f, 0.309017f},
		 {0.951056f, 1.250000f, 0.309017f},
		 {0.951056f, 1.250000f, 0.309017f},
		 {0.951056f, 1.250000f, 0.309017f},
		 {0.587785f, -1.25000f, 0.809017f},
		 {0.587785f, -1.25000f, 0.809017f},
		 {0.587785f, -1.25000f, 0.809017f},
		 {0.587785f, 1.250000f, 0.809017f},
		 {0.587785f, 1.250000f, 0.809017f},
		 {0.587785f, 1.250000f, 0.809017f},
		 {-0.000000f, -1.2500f, 1.000000f},
		 {-0.000000f, -1.2500f, 1.000000f},
		 {-0.000000f, -1.2500f, 1.000000f},
		 {-0.000000f, 1.25000f, 1.000000f},
		 {-0.000000f, 1.25000f, 1.000000f},
		 {-0.000000f, 1.25000f, 1.000000f},
		 {-0.587785f, -1.2500f, 0.809017f},
		 {-0.587785f, -1.2500f, 0.809017f},
		 {-0.587785f, -1.2500f, 0.809017f},
		 {-0.587785f, 1.25000f, 0.809017f},
		 {-0.587785f, 1.25000f, 0.809017f},
		 {-0.587785f, 1.25000f, 0.809017f},
		 {-0.951056f, -1.2500f, 0.309017f},
		 {-0.951056f, -1.2500f, 0.309017f},
		 {-0.951056f, -1.2500f, 0.309017f},
		 {-0.951056f, 1.25000f, 0.309017f},
		 {-0.951056f, 1.25000f, 0.309017f},
		 {-0.951056f, 1.25000f, 0.309017f},
		 {-0.951056f, -1.2500f, -0.309017f},
		 {-0.951056f, -1.2500f, -0.309017f},
		 {-0.951056f, -1.2500f, -0.309017f},
		 {-0.951056f, 1.25000f, -0.309017f},
		 {-0.951056f, 1.25000f, -0.309017f},
		 {-0.951056f, 1.25000f, -0.309017f},
		 {-0.587785f, -1.2500f, -0.809017f},
		 {-0.587785f, -1.2500f, -0.809017f},
		 {-0.587785f, -1.2500f, -0.809017f},
		 {-0.587785f, 1.25000f, -0.809017f},
		 {-0.587785f, 1.25000f, -0.809017f},
		 {-0.587785f, 1.25000f, -0.809017f},
		 {0.556884f, 1.250000f, -0.766484f},
		 {0.556884f, 1.250000f, -0.766484f},
		 {0.556884f, 1.250000f, -0.766484f},
		 {-0.000000f, 1.25000f, -0.947427f},
		 {-0.000000f, 1.25000f, -0.947427f},
		 {-0.000000f, 1.25000f, -0.947427f},
		 {0.901057f, 1.250000f, -0.292771f},
		 {0.901057f, 1.250000f, -0.292771f},
		 {0.901057f, 1.250000f, -0.292771f},
		 {0.901056f, 1.250000f, 0.292771f},
		 {0.901056f, 1.250000f, 0.292771f},
		 {0.901056f, 1.250000f, 0.292771f},
		 {0.556884f, 1.250000f, 0.766484f},
		 {0.556884f, 1.250000f, 0.766484f},
		 {0.556884f, 1.250000f, 0.766484f},
		 {-0.000000f, 1.25000f, 0.947427f},
		 {-0.000000f, 1.25000f, 0.947427f},
		 {-0.000000f, 1.25000f, 0.947427f},
		 {-0.556884f, 1.25000f, 0.766484f},
		 {-0.556884f, 1.25000f, 0.766484f},
		 {-0.556884f, 1.25000f, 0.766484f},
		 {-0.901056f, 1.25000f, 0.292771f},
		 {-0.901056f, 1.250000f, 0.292771f},
		 {-0.901056f, 1.250000f, 0.292771f},
		 {-0.901056f, 1.250000f, -0.292771f},
		 {-0.901056f, 1.250000f, -0.292771f},
		 {-0.901056f, 1.250000f, -0.292771f},
		 {-0.556883f, 1.250000f, -0.766485f},
		 {-0.556883f, 1.250000f, -0.766485f},
		 {-0.556883f, 1.250000f, -0.766485f},
		 {-0.000000f, -1.250000f, -0.947427f},
		 {-0.000000f, -1.250000f, -0.947427f},
		 {-0.000000f, -1.250000f, -0.947427f},
		 {0.556884f, -1.250000f, -0.766484f},
		 {0.556884f, -1.250000f, -0.766484f},
		 {0.556884f, -1.250000f, -0.766484f},
		 {0.901057f, -1.250000f, -0.292771f},
		 {0.901057f, -1.250000f, -0.292771f},
		 {0.901057f, -1.250000f, -0.292771f},
		 {0.901056f, -1.250000f, 0.292771f},
		 {0.901056f, -1.250000f, 0.292771f},
		 {0.901056f, -1.250000f, 0.292771f},
		 {0.556884f, -1.250000f, 0.766484f},
		 {0.556884f, -1.250000f, 0.766484f},
		 {0.556884f, -1.250000f, 0.766484f},
		 {-0.000000f, -1.250000f, 0.947427f},
		 {-0.000000f, -1.250000f, 0.947427f},
		 {-0.000000f, -1.250000f, 0.947427f},
		 {-0.556884f, -1.250000f, 0.766484f},
		 {-0.556884f, -1.250000f, 0.766484f},
		 {-0.556884f, -1.250000f, 0.766484f},
		 {-0.901056f, -1.250000f, 0.292771f},
		 {-0.901056f, -1.250000f, 0.292771f},
		 {-0.901056f, -1.250000f, 0.292771f},
		 {-0.901056f, -1.250000f, -0.292771f},
		 {-0.901056f, -1.250000f, -0.292771f},
		 {-0.901056f, -1.250000f, -0.292771f},
		 {-0.556883f, -1.250000f, -0.766485f},
		 {-0.556883f, -1.250000f, -0.766485f},
		 {-0.556883f, -1.250000f, -0.766485f},
		 {0.556884f, -1.200000f, -0.766484f},
		 {0.556884f, -1.200000f, -0.766484f},
		 {0.556884f, -1.200000f, -0.766484f},
		 {0.000000f, -1.200000f, -0.000000f},
		 {-0.000000f, -1.200000f, -0.947427f},
		 {-0.000000f, -1.200000f, -0.947427f},
		 {-0.000000f, -1.200000f, -0.947427f},
		 {0.901057f, -1.200000f, -0.292771f},
		 {0.901057f, -1.200000f, -0.292771f},
		 {0.901057f, -1.200000f, -0.292771f},
		 {0.901056f, -1.200000f, 0.292771f},
		 {0.901056f, -1.200000f, 0.292771f},
		 {0.901056f, -1.200000f, 0.292771f},
		 {0.556884f, -1.200000f, 0.766484f},
		 {0.556884f, -1.200000f, 0.766484f},
		 {0.556884f, -1.200000f, 0.766484f},
		 {-0.000000f, -1.200000f, 0.947427f},
		 {-0.000000f, -1.200000f, 0.947427f},
		 {-0.000000f, -1.200000f, 0.947427f},
		 {-0.556884f, -1.200000f, 0.766484f},
		 {-0.556884f, -1.200000f, 0.766484f},
		 {-0.556884f, -1.200000f, 0.766484f},
		 {-0.901056f, -1.200000f, 0.292771f},
		 {-0.901056f, -1.200000f, 0.292771f},
		 {-0.901056f, -1.200000f, 0.292771f},
		 {-0.901056f, -1.200000f, -0.292771f},
		 {-0.901056f, -1.200000f, -0.292771f},
		 {-0.901056f, -1.200000f, -0.292771f},
		 {-0.556883f, -1.200000f, -0.766485f},
		 {-0.556883f, -1.200000f, -0.766485f},
		 {-0.556883f, -1.200000f, -0.766485f},
		 {-0.000000f, 1.200000f, -0.947427f},
		 {-0.000000f, 1.200000f, -0.947427f},
		 {-0.000000f, 1.200000f, -0.947427f},
		 {0.000000f, 1.200000f, -0.000000f},
		 {0.556884f, 1.200000f, -0.766484f},
		 {0.556884f, 1.200000f, -0.766484f},
		 {0.556884f, 1.200000f, -0.766484f},
		 {0.901057f, 1.200000f, -0.292771f},
		 {0.901057f, 1.200000f, -0.292771f},
		 {0.901057f, 1.200000f, -0.292771f},
		 {0.901056f, 1.200000f, 0.292771f},
		 {0.901056f, 1.200000f, 0.292771f},
		 {0.901056f, 1.200000f, 0.292771f},
		 {0.556884f, 1.200000f, 0.766484f},
		 {0.556884f, 1.200000f, 0.766484f},
		 {0.556884f, 1.200000f, 0.766484f},
		 {-0.000000f, 1.200000f, 0.947427f},
		 {-0.000000f, 1.200000f, 0.947427f},
		 {-0.000000f, 1.200000f, 0.947427f},
		 {-0.556884f, 1.200000f, 0.766484f},
		 {-0.556884f, 1.200000f, 0.766484f},
		 {-0.556884f, 1.200000f, 0.766484f},
		 {-0.901056f, 1.200000f, 0.292771f},
		 {-0.901056f, 1.200000f, 0.292771f},
		 {-0.901056f, 1.200000f, 0.292771f},
		 {-0.901056f, 1.200000f, -0.292771f},
		 {-0.901056f, 1.200000f, -0.292771f},
		 {-0.901056f, 1.200000f, -0.292771f},
		 {-0.556883f, 1.200000f, -0.766485f},
		 {-0.556883f, 1.200000f, -0.766485f},
		 {-0.556883f, 1.200000f, -0.766485f},
		};

		ConvexHull* convexHull = Quickhull::GenerateConvexHull(pointCloud);

		if (convexHull)
		{
			std::vector<Vector3> lineList;
			convexHull->GetEdgesAsLineList(lineList);
			mesh.m_ConvexHull = convexHull;

			std::vector<Vertex> hullVertices;
			for (size_t v = 0; v < lineList.size(); v++)
			{
				Vertex vertex;
				vertex.Position = lineList[v];
				hullVertices.push_back(vertex);
			}

			size_t hullVertexCount = sizeof(Vertex) * hullVertices.size();
			bool boundHullVertexData = renderer->BindVertexData(convexHull->m_RenderingVertexBuffer, hullVertices.data(), hullVertexCount);
			assert(boundHullVertexData);
		}
	}

	return true;
}
