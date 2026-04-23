#include "pch.h"
#include "AssetLoader.h"
#include "AssetManagement.h"
#include <Rendering/Geometry/Mesh.h>
#include <Rendering/IMGUIIncludes.h>
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

void AssetManager::OnIMGUIRender()
{
	ImGui::Begin("Asset Manager");

	ImGui::SeparatorText("Loaded Textures");

	TextureMap::iterator textureItr = m_TextureMap.begin();
	while (textureItr != m_TextureMap.end())
	{
		TextureRef ref = GetTexture(textureItr->first);

		if (textureItr->second.expired())
		{
			printf("Invalid texture reference in the map.\n");
			textureItr++;
			continue;
		}

		const char* name = ref->GetDisplayName().c_str();

		float imageSize = ImGui::CalcTextSize(name).y;
		ImGui::ImageWithBg((ImTextureID)ref->GetGPUHandle().ptr, ImVec2(imageSize, imageSize));
		ImGui::SameLine();
		ImGui::Text(name);

		if (ImGui::BeginItemTooltip())
		{
			ImGui::ImageWithBg((ImTextureID)ref->GetGPUHandle().ptr, ImVec2(256, 256));
			ImGui::EndTooltip();
		}

		textureItr++;
	}

	ImGui::SeparatorText("Loaded Models");

	if (m_ModelMap.size() > 0)
	{
		if (ImGui::BeginTable("Model Data Set", 3))
		{
			ModelMap::iterator modelItr = m_ModelMap.begin();

			while (modelItr != m_ModelMap.end())
			{
				ModelRef ref = GetModel(modelItr->first);
				int meshCount = (int)ref->GetMeshes().size();

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(ref->GetDisplayName().c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("Mesh Count: %i\n", meshCount);

				if (ImGui::BeginItemTooltip())
				{
					for (size_t m = 0; m < meshCount; m++)
					{
						int textureCount = (int)ref->GetMeshes()[m]->GetTextures().size();
						ImGui::Text("Mesh %i [ %i Texture%s]\n", m, textureCount, (textureCount != 1) ? "s " : " ");

						for (size_t tc = 0; tc < textureCount; tc++)
						{
							TextureRef tex = ref->GetMeshes()[m]->GetTextures()[tc];
							ImGui::ImageWithBg((ImTextureID)tex->GetGPUHandle().ptr, ImVec2(64, 64));

							if(tc + 1 != textureCount)
								ImGui::SameLine();
						}
					}

					ImGui::EndTooltip();
				}

				modelItr++;
			}

			ImGui::EndTable();
		}

		
	}
	else
	{
		ImGui::Text("No models loaded.\n");
	}


	ImGui::End();
}


bool AssetManager::Initialise()
{
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
		printf("Trying to destroy an assetManager that doesn't exist.\n");
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

void AssetManager::PreloadFolder(const std::filesystem::path& folderPath)
{
	if (std::filesystem::exists(folderPath) == false)
	{
		printf("Failed to preload folder \"%s\". Folder does not exist.\n", folderPath.string().c_str());
		return;
	}

	for (const auto& directoryEntry : std::filesystem::recursive_directory_iterator(folderPath))
	{
		std::string extension = directoryEntry.path().extension().string();

		if (extension != ".png" && extension != ".PNG" &&
			extension != ".gltf" && extension != ".GLTF" &&
			extension != ".glb" &&  extension != ".GLB")
		{
			continue;
		}

		if (extension == ".png" || extension == ".PNG")
		{
			TextureRef ref = GetTexture(directoryEntry.path().string());

			if (ref == nullptr)
			{
				printf("Failed to preload texture %s\n", directoryEntry.path().string().c_str());
			}
		}
		else if (extension == ".gltf" || extension == ".GLTF" || extension == ".glb" || extension == ".GLB")
		{
			ModelRef ref = GetModel(directoryEntry.path().string());

			if (ref == nullptr)
			{
				printf("Failed to preload model %s\n", directoryEntry.path().string().c_str());
			}
		}
		else
		{
			printf("%s\n", directoryEntry.path().string().c_str());
		}
	}
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

	std::filesystem::path filepath = Engine::GetContentFolderLocation() / std::filesystem::path(path);
	if (filepath.has_extension() == false)
	{
		filepath += ".png";
	}

	TextureRef ref = AssetLoader::LoadTexture(filepath.string());

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

	std::filesystem::path filepath = Engine::GetContentFolderLocation() / std::filesystem::path(path);
	ModelRef ref = AssetLoader::LoadModel(filepath.string());

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