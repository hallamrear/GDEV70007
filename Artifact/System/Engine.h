#pragma once
#include <System/InputListener.h>

class World;
class Renderer;
class AssetManager;

class Engine : IIMGUIRenderable
{
private:
	std::chrono::steady_clock::time_point m_RenderStart;
	std::chrono::steady_clock::time_point m_RenderEnd;
	std::chrono::steady_clock::time_point m_UpdateStart;
	std::chrono::steady_clock::time_point m_UpdateEnd;
	std::vector<std::pair<double, std::pair<long double, long double>>> m_SampleFrameData;
	static std::filesystem::path m_ContentFolderLocation;
	static std::filesystem::path m_ExecutableLocation;
	bool m_IsInitialised;
	bool m_IsRunning;
	double m_FrameTime;
	int m_SampleFramesRemaining;
	int m_SampleSetsRemaining;
	float TimeBetweenSamples = 0.5f;

	InputListener m_InputListener;
	AssetManager* m_AssetManager;
	Renderer* m_Renderer;
	World* m_World;

	Engine();
	~Engine();

	bool InitialiseSubsystems(HWND windowHandle, const std::filesystem::path& contentFolderLocation);

public:
	static std::chrono::duration<long double> UpdateDelta;
	static std::chrono::duration<long double> RenderDelta;

	static Engine* CreateEngine(HWND windowHandle, const std::filesystem::path& contentFolderLocation);
	static bool DestroyEngine(Engine* engine);

	void Start();
	void Stop();
	const bool& IsRunning() const;

	static const std::filesystem::path& GetContentFolderLocation();

	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void FixedUpdate();
	void Update(const double& deltaTime);
	void Render();
	void CalculateTimings();

	void OnIMGUIRender() override;
};

