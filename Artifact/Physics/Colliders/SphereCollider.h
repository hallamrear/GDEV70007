#pragma once
#include <Physics/Colliders/Collider.h>

class SphereCollider : public Collider
{
public:
	SphereCollider(const Entity& entity);
	~SphereCollider();
};

