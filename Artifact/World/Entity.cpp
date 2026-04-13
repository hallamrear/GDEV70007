#include "pch.h"
#include "Entity.h"

Entity::Entity()
{
	m_ID = EntityID();
	CoCreateGuid(&m_ID);
	m_DisplayName = "Unnamed";
}

Entity::~Entity()
{
	m_ID = EntityID();
}

const EntityID Entity::GetID() const
{
	return m_ID;
}

const std::string& Entity::GetDisplayName() const
{
	return m_DisplayName;
}

void Entity::SetDisplayName(const std::string& displayName)
{
	m_DisplayName = displayName;
}

Rigidbody& Entity::GetRigidbody()
{
	return m_Rigidbody;
}

void Entity::Update(const float& deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);
}

void Entity::PostUpdate(const float& deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);
}

void Entity::Render()
{

}