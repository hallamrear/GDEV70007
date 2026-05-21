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

Vector3 AABBCollider::GetFurthestPointInDirection(const Vector3& direction) const
{
	Vector3 maxPoint = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3 corner = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3 scaledPoint = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	float maxDistance = -FLT_MAX;
	float distance = -FLT_MAX;

	for (int i = 0; i < 8; i++)
	{
		scaledPoint = Vector3(Points[i].x * m_Size.x, Points[i].y * m_Size.y, Points[i].z * m_Size.z);

		DirectX::XMStoreFloat3(&corner,
		DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&scaledPoint), DirectX::XMLoadFloat4x4(&m_AttachedEntity.GetWorldMatrix())));

		DirectX::XMStoreFloat(&distance, DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&corner), DirectX::XMLoadFloat3(&direction)));

		if (distance >= maxDistance)
		{
			maxDistance = distance;
			maxPoint = corner;
		}
	}

	return maxPoint;
}

void AABBCollider::Render(Renderer& renderer)
{
	if (m_ColliderModel == nullptr)
	{
		printf("Trying to draw a collider that doesn't have a model.\n");
		throw;
	}

	Matrix4x4 entityMatrix = IdentityMatrix;
	DirectX::XMStoreFloat4x4(&entityMatrix,
		DirectX::XMMatrixTranslation(m_AttachedEntity.GetPosition().x, m_AttachedEntity.GetPosition().y, m_AttachedEntity.GetPosition().z));

	renderer.SetDebugDrawMode();
	Matrix4x4 worldMatrix = IdentityMatrix;
	DirectX::XMStoreFloat4x4(&worldMatrix,
		DirectX::XMMatrixScaling(m_Size.x, m_Size.y, m_Size.z) *
		DirectX::XMLoadFloat4x4(&entityMatrix) *
		DirectX::XMLoadFloat4x4(&m_OffsetMatrix));
	renderer.Render(m_ColliderModel, worldMatrix);
	renderer.SetDefaultDrawMode();
}