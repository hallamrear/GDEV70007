#include "pch.h"
#include "SphereCollider.h"
#include "BoxCollider.h"

SphereCollider::SphereCollider(const Entity& entity, const float& radius) : Collider(COLLIDER_TYPE::COLLIDER_TYPE_SPHERE, entity)
{
	SetSize(Vector3(radius, 0.0f, 0.0f));
}

SphereCollider::SphereCollider(const Entity& entity, const Vector3& size) : Collider(COLLIDER_TYPE::COLLIDER_TYPE_SPHERE, entity)
{
	SetSize(size);
}

SphereCollider::~SphereCollider()
{
	m_Size = Vector3(0.0f, 0.0f, 0.0f);
}

void SphereCollider::SetSize(const Vector3& size)
{
	Vector3 absSize = Vector3(std::abs(size.x), std::abs(size.y), std::abs(size.z));
	float r = std::max(absSize.x, std::max(absSize.y, absSize.z));
	m_Size.x = r;
	m_Size.y = r;
	m_Size.z = r;
}