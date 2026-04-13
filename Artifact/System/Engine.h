#pragma once

class Renderer;
class World;

class Engine
{
private:
	static std::filesystem::path m_ContentFolderLocation;
	bool m_IsInitialised;
	bool m_IsRunning;

	Renderer* m_Renderer;
	World* m_World;

	Engine();
	~Engine();

	bool InitialiseSubsystems(HWND windowHandle, const std::filesystem::path& contentFolderLocation);

public:
	static Engine* CreateEngine(HWND windowHandle, const std::filesystem::path& contentFolderLocation);
	static bool DestroyEngine(Engine* engine);

	void Start();
	void Stop();
	const bool& IsRunning() const;

	void Update(const float& deltaTime);
	void Render();
};

