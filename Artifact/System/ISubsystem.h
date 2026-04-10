#pragma once

class ISubsystem
{
private:
	const char* m_Name;
	bool m_IsInitialised;

	ISubsystem();

public:
	virtual ~ISubsystem() = 0;

	virtual bool Initialise() = 0; 
	virtual bool Shutdown() = 0;

	virtual void Update(const float& deltaTime);
	virtual void PostUpdate(const float& deltaTime);
	virtual void Render();
};

