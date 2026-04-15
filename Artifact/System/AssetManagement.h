#pragma once
#include <unordered_map>
#include <Rendering/Texturing/Texture.h>
#include <Rendering/Geometry/Model.h>

typedef std::unordered_map<std::string, ModelPtr> ModelMap;
typedef std::unordered_map<std::string, TexturePtr> TextureMap;

class AssetBundle
{
private:
	std::vector<TextureRef> m_Textures;
	std::vector<ModelRef> m_Models;

public:
	AssetBundle();
	~AssetBundle();

	void AddTexture(TextureRef textureRef)
	{

	}
};

class AssetManager
{
private:
	ModelMap m_ModelMap;
	TextureMap m_TextureMap;
	bool m_IsInitialised;

protected:
	AssetManager();
	~AssetManager();

	bool Initialise();
	bool Shutdown();

public:
	static AssetManager* CreateAssetDatabase();
	static bool DestroyAssetDatabase(AssetManager* assetManager);

	bool LoadBundle(const std::string& bundleFilePath, AssetBundle* bundle);

	TextureRef GetTexture(const std::string& path);
	ModelRef GetModel(const std::string& path);

	const bool& IsInitialised() const;
};

