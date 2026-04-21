#pragma once

class World;
class Renderer;
class AssetManager;

class Engine
{
private:
	static std::filesystem::path m_ContentFolderLocation;
	static std::filesystem::path m_ExecutableLocation;
	bool m_IsInitialised;
	bool m_IsRunning;

	AssetManager* m_AssetManager;
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

	static const std::filesystem::path& GetContentFolderLocation();

	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void Update(const float& deltaTime);
	void Render();
};

