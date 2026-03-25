#pragma once
class Engine
{
private:
	static std::filesystem::path m_ContentFolderLocation;

	Engine();
	~Engine();

	bool InitialiseSubSystems();

public:
	static Engine* CreateEngine(const std::filesystem::path& _contentFolderLocation);
	static bool DestroyEngine(Engine* _engine);

	void Update(const float& _deltaTime);
	void Render();
};

