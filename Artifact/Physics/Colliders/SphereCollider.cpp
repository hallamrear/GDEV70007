#include "pch.h"
#include "SphereCollider.h"

SphereCollider::SphereCollider(const Entity& entity, const float& radius) : Collider(COLLIDER_TYPE::COLLIDER_TYPE_SPHERE, entity)
{

}

SphereCollider::~SphereCollider()
{

}

void SphereCollider::Render()
{

}