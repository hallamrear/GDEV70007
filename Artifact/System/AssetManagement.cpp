#include "pch.h"
#include "AssetLoader.h"
#include "AssetManagement.h"
#include <Rendering/Geometry/Mesh.h>
#include <System/Engine.h>

AssetManager::AssetManager()
{
	m_IsInitialised = false;
	m_TextureMap = TextureMap();
	m_ModelMap = ModelMap();
}

AssetManager::~AssetManager()
{
	assert(!m_IsInitialised);
}

const bool& AssetManager::IsInitialised() const
{
	return m_IsInitialised;
}

static ModelRef testModel = nullptr;
#include <Rendering/Renderer.h>
#include <Rendering/Texturing/Texture.h>
#include <Rendering/IMGUIIncludes.h>
#include <Rendering/DX12Includes.h>
void AssetManager::DebugDraw()
{
	ImGui::Begin("TEST DRAW");

	TextureMap::iterator itr = m_TextureMap.begin();
	while (itr != m_TextureMap.end())
	{
		TextureRef ref = GetTexture(itr->first);
		ImGui::Image((ImTextureID)ref->GetGPUHandle().ptr, ImVec2(128.0f, 128.0f));
		itr++;
	}

	ImGui::End();

	Matrix4x4 identity;
	XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());
	Model* model = testModel.get();
	ServiceLocator::Locate<Renderer>()->Render(*model, identity);
}

bool AssetManager::Initialise()
{
	testModel = GetModel("Barrel.glb");
	return true;
}

AssetManager* AssetManager::CreateAssetDatabase()
{
	AssetManager* am = new AssetManager();
	
	if (am->Initialise())
	{
		printf("Initialised asset manager.\n");
	}
	else
	{
		printf("Failed to initialise asset manager.\n");

		delete am;
		am = nullptr;
	}

	return am;
}	

bool AssetManager::Shutdown()
{
	for (auto& texture : m_TextureMap)
	{
		if (texture.second.expired() == false)
		{
			texture.second.reset();
		}
	}

	for (auto& model : m_ModelMap)
	{
		if (model.second.expired() == false)
		{
			model.second.reset();
		}
	}

	return true;
}

bool AssetManager::DestroyAssetDatabase(AssetManager* assetManager)
{
	if (assetManager == nullptr)
	{
		return false;
	}

	if (!assetManager->IsInitialised())
	{
		printf("Trying to destroy an assetManager that doesn't exist.");
		return false;
	}

	return assetManager->Shutdown();
}

bool AssetManager::LoadBundle(const std::string& bundleFilePath, AssetBundle* bundle)
{
	std::filesystem::path path = bundleFilePath;

	if (std::filesystem::exists(path) == false)
	{
		printf("Failed to find asset bundle file: %s\n", bundleFilePath.c_str());
		return false;
	}

	return (bundle != nullptr);
}

TextureRef AssetManager::GetTexture(const std::string& path)
{
	TextureMap::iterator itr = m_TextureMap.find(path);

	if (itr != m_TextureMap.end())
	{
		if (TextureRef ref = itr->second.lock())
		{
			return ref;
		}
	}

	TextureRef ref = AssetLoader::LoadTexture(path);

	if (ref != nullptr)
	{
		m_TextureMap[ref->GetDisplayName()] = ref;
	}

	return ref;
}

ModelRef AssetManager::GetModel(const std::string& path)
{
	ModelMap::iterator itr = m_ModelMap.find(path);

	if (itr != m_ModelMap.end())
	{
		if (ModelRef ref = itr->second.lock())
		{
			return ref;
		}
	}

	ModelRef ref = AssetLoader::LoadModel(path);

	if (ref != nullptr)
	{
		for (auto& mesh : ref->GetMeshes())
		{
			for (auto& texture : mesh->GetTextures())
			{
				m_TextureMap[texture->GetDisplayName()] = texture;
			}
		}

		m_ModelMap[path] = ref;
	}

	return ref;
}