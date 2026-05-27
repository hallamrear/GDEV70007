#include "pch.h"
#include "AABBCollider.h"
#include <World/Entity.h>

Vector3 AABBCollider::Points[8] =
{
	Vector3(+0.5f, +0.5f, +0.5f),
	Vector3(-0.5f, +0.5f, +0.5f),
	Vector3(-0.5f, +0.5f, -0.5f),
	Vector3(+0.5f, +0.5f, -0.5f),
	Vector3(+0.5f, -0.5f, +0.5f),
	Vector3(-0.5f, -0.5f, +0.5f),
	Vector3(-0.5f, -0.5f, -0.5f),
	Vector3(+0.5f, -0.5f, -0.5f),
};

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
	Matrix4x4 entityMatrix = IdentityMatrix;
	DirectX::XMStoreFloat4x4(&entityMatrix,
		DirectX::XMMatrixTranslation(m_AttachedEntity.GetPosition().x, m_AttachedEntity.GetPosition().y, m_AttachedEntity.GetPosition().z));

	Vector3 max = m_Size;
	XMStoreFloat3(&max, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&max), DirectX::XMLoadFloat4x4(&entityMatrix)));
	return max;
}

Matrix4x4 AABBCollider::GetTransformMatrix() const
{
	Matrix4x4 transformMatrix = IdentityMatrix;
	DirectX::XMStoreFloat4x4(&transformMatrix,
		DirectX::XMMatrixScaling(m_Size.x, m_Size.y, m_Size.z) *
		DirectX::XMMatrixTranslation(m_AttachedEntity.GetPosition().x, m_AttachedEntity.GetPosition().y, m_AttachedEntity.GetPosition().z) *
		DirectX::XMLoadFloat4x4(&m_OffsetMatrix));
	return transformMatrix;
}

Vector3 AABBCollider::GetMinCornerWorldSpace() const
{
	Matrix4x4 entityMatrix = IdentityMatrix;
	DirectX::XMStoreFloat4x4(&entityMatrix,
		DirectX::XMMatrixTranslation(m_AttachedEntity.GetPosition().x, m_AttachedEntity.GetPosition().y, m_AttachedEntity.GetPosition().z));

	Vector3 min = GetMinCornerLocalSpace();
	XMStoreFloat3(&min, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&min), DirectX::XMLoadFloat4x4(&entityMatrix)));
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

Vector3 AABBCollider::GetFurthestPointInDirection(const Vector3& direction) const
{
	Vector3 maxPoint = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3 corner = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3 scaledPoint = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	float maxDistance = -FLT_MAX;
	float distance = -FLT_MAX;

	Matrix4x4 transformMatrix = GetTransformMatrix();

	for (int i = 0; i < 8; i++)
	{
		DirectX::XMStoreFloat3(&corner,
			DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&Points[i]), DirectX::XMLoadFloat4x4(&transformMatrix)));

		distance = Maths::Dot(corner, direction);

		if (distance >= maxDistance)
		{
			maxDistance = distance;
			maxPoint = corner;
		}
	}

	return maxPoint;
}

void AABBCollider::GetPoints(std::vector<Vector3>& points) const
{
	points.clear();
	points.resize(8);
	Matrix4x4 transformMatrix = GetTransformMatrix();

	for (int i = 0; i < 8; i++)
	{
		DirectX::XMStoreFloat3(&points[i], DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&Points[i]), DirectX::XMLoadFloat4x4(&transformMatrix)));
	}
}

Vector3 AABBCollider::ClosestPoint()
{
	//https://www.youtube.com/watch?v=VmtNPguCTjQ
	//17:40
	return Vector3();
}
