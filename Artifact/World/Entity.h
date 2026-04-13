#pragma once
#include <Physics/Rigidbody.h>

class Entity
{
private:
	std::string m_DisplayName;
	EntityID m_ID;
	Rigidbody m_Rigidbody;

public:
	Entity();
	~Entity();

	const EntityID GetID() const;
	const std::string& GetDisplayName() const;
	void SetDisplayName(const std::string& displayName);
	Rigidbody& GetRigidbody();
	
	void Update(const float& deltaTime);
	void PostUpdate(const float& deltaTime);
	void Render();
};