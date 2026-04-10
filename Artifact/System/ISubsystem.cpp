#include "pch.h"
#include "System/ISubsystem.h"

ISubsystem::ISubsystem()
{
	m_Name = "Unnamed Subsystem";
	m_IsInitialised = false;
}

ISubsystem::~ISubsystem()
{
	
}

bool ISubsystem::Initialise()
{
	return false;
}

bool ISubsystem::Shutdown()
{
	return false;
}

void ISubsystem::Update(const float& _deltaTime)
{

}

void ISubsystem::PostUpdate(const float& deltaTime)
{
}

void ISubsystem::Render()
{

}
