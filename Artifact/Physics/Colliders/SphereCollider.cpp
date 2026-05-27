#include "pch.h"
#include "SphereCollider.h"
#include <World/Entity.h>

Matrix4x4 SphereCollider::GetTransformMatrix() const
{
	return IdentityMatrix;
}

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

/// <summary>
/// Returns the furthest distance on the collider in a given direction. Point is given in world space.
/// </summary>
Vector3 SphereCollider::GetFurthestPointInDirection(const Vector3& direction) const
{
	Vector3 furthestPoint(0.0f, 0.0f, 0.0f);
	DirectX::XMStoreFloat3(&furthestPoint, DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&direction)), m_Size.x));
	furthestPoint.x += m_AttachedEntity.GetPosition().x;
	furthestPoint.y += m_AttachedEntity.GetPosition().y;
	furthestPoint.z += m_AttachedEntity.GetPosition().z;
	return furthestPoint;
}

void SphereCollider::GetPoints(std::vector<Vector3>& points) const
{
	points.clear();
	throw new std::exception("Function not implemented.");
}
