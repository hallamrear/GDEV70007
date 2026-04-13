#pragma once

class Renderer
{
private:
	bool m_IsInitialised;

	Renderer();
	~Renderer();

	bool Initialise();
	bool Shutdown();

public:
	const bool& IsInitialised() const;

	static Renderer* CreateRenderer();
	static bool DestroyRenderer(Renderer* renderer);
};

