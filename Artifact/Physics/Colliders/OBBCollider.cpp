#include "pch.h"
#include "OBBCollider.h"
#include <World/Entity.h>

Vector3 OBBCollider::Points[8] =
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

Matrix4x4 OBBCollider::GetTransformMatrix() const
{
	Matrix4x4 transformMatrix;
	DirectX::XMStoreFloat4x4(&transformMatrix,
		DirectX::XMMatrixScaling(m_Size.x, m_Size.y, m_Size.z) *
		DirectX::XMLoadFloat4x4(&m_AttachedEntity.GetWorldMatrix()) *
		DirectX::XMLoadFloat4x4(&m_OffsetMatrix));
	return transformMatrix;
}

OBBCollider::OBBCollider(const Entity& entity, const Vector3& halfWidth) : Collider(COLLIDER_TYPE::COLLIDER_TYPE_OBB, entity)
{
	SetSize(halfWidth);
}

OBBCollider::OBBCollider(const Entity& entity, const Vector3& max, const Vector3& min) : Collider(COLLIDER_TYPE::COLLIDER_TYPE_OBB, entity)
{
	m_Size.x = (max.x - min.x) * 0.5f;
	m_Size.y = (max.y - min.y) * 0.5f;
	m_Size.z = (max.z - min.z) * 0.5f;
}

OBBCollider::~OBBCollider()
{
	m_Size = Vector3(0.0f, 0.0f, 0.0f);
}

void OBBCollider::SetSize(const Vector3& halfWidth)
{
	m_Size = halfWidth;
}

Vector3 OBBCollider::GetMaxCornerWorldSpace() const
{
	Vector3 max = m_Size;
	XMStoreFloat3(&max, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&max), DirectX::XMLoadFloat4x4(&GetAttachedEntity().GetWorldMatrix())));
	return max;
}

Vector3 OBBCollider::GetMinCornerWorldSpace() const
{
	Vector3 min = GetMinCornerLocalSpace();
	XMStoreFloat3(&min, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&min), DirectX::XMLoadFloat4x4(&GetAttachedEntity().GetWorldMatrix())));
	return min;
}

Vector3 OBBCollider::GetMaxCornerLocalSpace() const
{
	return m_Size;
}

Vector3 OBBCollider::GetMinCornerLocalSpace() const
{
	return Vector3(m_Size.x * -1.0f, m_Size.y * -1.0f, m_Size.z * -1.0f);
}

//void GetPoints(std::vector<Vector3>& points) const override;
void OBBCollider::GetPoints(std::vector<Vector3>& points) const
{
	points.clear();

	Matrix4x4 transformMatrix = GetTransformMatrix();

	points.resize(8);

	for (size_t i = 0; i < 8; i++)
	{
		DirectX::XMStoreFloat3(&points[i], DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&Points[i]), DirectX::XMLoadFloat4x4(&transformMatrix)));
	}
}

Vector3 OBBCollider::GetFurthestPointInDirection(const Vector3& direction) const
{
	Vector3 maxPoint = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3 corner = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3 scaledPoint = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	float maxDistance = -FLT_MAX;
	float distance = -FLT_MAX;

	for (int i = 0; i < 8; i++)
	{
		scaledPoint = Vector3(Points[i].x * m_Size.x, Points[i].y * m_Size.y, Points[i].z * m_Size.z);

		DirectX::XMStoreFloat3(&corner, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&scaledPoint), DirectX::XMLoadFloat4x4(&m_AttachedEntity.GetWorldMatrix())));

		DirectX::XMStoreFloat(&distance, DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&corner), DirectX::XMLoadFloat3(&direction)));

		if (distance >= maxDistance)
		{
			maxDistance = distance;
			maxPoint = corner;
		}
	}

	return maxPoint;
}

//void OBBCollider::Render(Renderer& renderer)
//{
//	if (m_ColliderModel == nullptr)
//	{
//		printf("Trying to draw a collider that doesn't have a model.\n");
//		throw;
//	}
//
//	renderer.SetDebugDrawMode();
//	renderer.Render(m_ColliderModel, GetTransformMatrix());
//	renderer.SetDefaultDrawMode();
//}