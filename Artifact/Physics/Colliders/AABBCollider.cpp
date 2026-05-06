#include "pch.h"
#include "AABBCollider.h"
#include <World/Entity.h>

AABBCollider::AABBCollider(const Entity& entity, const Vector3& halfWidth) : Collider(COLLIDER_TYPE::COLLIDER_TYPE_AABB, entity)
{
	SetSize(halfWidth);
}

AABBCollider::AABBCollider(const Entity& entity, const Vector3& max, const Vector3& min) : Collider(COLLIDER_TYPE::COLLIDER_TYPE_AABB, entity)
{
	m_Size.x = (max.x - min.x) * 0.5f;
	m_Size.y = (max.y - min.y) * 0.5f;
	m_Size.z = (max.z - min.z) * 0.5f;
}

AABBCollider::~AABBCollider()
{
	m_Size = Vector3(0.0f, 0.0f, 0.0f);
}

void AABBCollider::SetSize(const Vector3& halfWidth)
{
	m_Size = halfWidth;
}

Vector3 AABBCollider::GetMaxCornerWorldSpace() const
{
	Vector3 max = m_Size;
	XMStoreFloat3(&max, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&max), DirectX::XMLoadFloat4x4(&GetAttachedEntity().GetWorldMatrix())));
	return max;
}

Vector3 AABBCollider::GetMinCornerWorldSpace() const
{
	Vector3 min = GetMinCornerLocalSpace();
	XMStoreFloat3(&min, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&min), DirectX::XMLoadFloat4x4(&GetAttachedEntity().GetWorldMatrix())));
	return min;
}

Vector3 AABBCollider::GetMaxCornerLocalSpace() const
{
	return m_Size;
}

Vector3 AABBCollider::GetMinCornerLocalSpace() const
{
	return Vector3(m_Size.x * -1.0f, m_Size.y * -1.0f, m_Size.z * -1.0f);
}