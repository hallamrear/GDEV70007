#include "pch.h"
#include "BoxCollider.h"

BoxCollider::BoxCollider(const Entity& entity, const Vector3& halfWidth) : Collider(COLLIDER_TYPE::COLLIDER_TYPE_AABB, entity)
{
	SetSize(halfWidth);
}

BoxCollider::BoxCollider(const Entity& entity, const Vector3& max, const Vector3& min) : Collider(COLLIDER_TYPE::COLLIDER_TYPE_AABB, entity)
{
	m_Size.x = (max.x - min.x) / 2.0f;
	m_Size.y = (max.y - min.y) / 2.0f;
	m_Size.z = (max.z - min.z) / 2.0f;
}

BoxCollider::~BoxCollider()
{
	m_Size = Vector3(0.0f, 0.0f, 0.0f);
}

void BoxCollider::SetSize(const Vector3& halfWidth)
{
	m_Size = halfWidth;
}