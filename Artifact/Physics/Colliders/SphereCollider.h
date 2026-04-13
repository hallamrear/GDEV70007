#pragma once
#include <Physics/Colliders/Collider.h>

class SphereCollider : public Collider
{
private:
	float m_Radius;

public:
	SphereCollider(const Entity& entity, const float& radius);
	~SphereCollider();

	const float& GetRadius() const;

	void Render();
};

