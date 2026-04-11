#pragma once

class ISubsystem;

class ServiceLocator
{
private:
	ServiceLocator();
	~ServiceLocator();

public:
	bool AddSubsystem(ISubsystem* subsystem);
	ISubsystem* LocateSubsystem(const char* name);
};
