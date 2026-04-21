#include "pch.h"
#include "SphereCollider.h"

SphereCollider::SphereCollider(const Entity& entity, const float& radius) : Collider(COLLIDER_TYPE::COLLIDER_TYPE_SPHERE, entity)
{
	m_Radius = radius;
}

SphereCollider::~SphereCollider()
{

}

const float& SphereCollider::GetRadius() const
{
	return m_Radius;
}