#pragma once

class Renderer;
class World;

class Engine
{
private:
	static std::filesystem::path m_ContentFolderLocation;
	bool m_IsInitialised;

	Renderer* m_Renderer;
	World* m_World;

	Engine();
	~Engine();

	bool InitialiseSubsystems(const std::filesystem::path& contentFolderLocation);

public:
	static Engine* CreateEngine(const std::filesystem::path& contentFolderLocation);
	static bool DestroyEngine(Engine* engine);

	void Update(const float& deltaTime);
	void Render();
};

