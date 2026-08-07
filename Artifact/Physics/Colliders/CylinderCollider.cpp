#include "pch.h"
#include "CylinderCollider.h"
#include <World/Entity.h>
#include <System/AssetManagement.h>

CylinderCollider::CylinderCollider(const Entity& entity, const float& radius, const float& height) : Collider(COLLIDER_TYPE::COLLIDER_TYPE_CAPSULE, entity)
{
	SetSize(Vector3(radius, height, radius));
}

CylinderCollider::~CylinderCollider()
{
	SetSize(Vector3(0.0f, 0.0f, 0.0f));
}

Matrix4x4 CylinderCollider::GetTransformMatrix() const
{
	Matrix4x4 transformMatrix = IdentityMatrix;
	DirectX::XMStoreFloat4x4(&transformMatrix,
		DirectX::XMMatrixScaling(m_Size.x, m_Size.y, m_Size.z) *
		DirectX::XMLoadFloat4x4(&m_AttachedEntity.GetWorldMatrix()) *
		DirectX::XMLoadFloat4x4(&m_OffsetMatrix));
	return transformMatrix;
}

void CylinderCollider::SetSize(const Vector3& radius_height_radius)
{
	m_Size.x = radius_height_radius.x;
	m_Size.y = radius_height_radius.y / 2.0f;
	m_Size.z = radius_height_radius.z;
	UpdateBoundingVolume();
}

Vector3 CylinderCollider::GetSupportPoint(const Vector3& direction) const
{
	glm::vec3 dir = { direction.x, direction.y, direction.z };

	Vector3 result = { 0.0f, 0.0f, 0.0f };
	glm::vec3 dir_xz = glm::normalize(glm::vec3(dir.x, 0, dir.z)) * m_Size.x;
	result = { dir_xz.x, 0.0f, dir_xz.z };

	float height = m_Size.y / 2.0f;
	result.y = (dir.y > 0) ? height : -height;

	DirectX::XMStoreFloat3(&result, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&result), DirectX::XMLoadFloat4x4(&m_AttachedEntity.GetWorldMatrix())));

	return Vector3(result.x, result.y, result.z);
}

void CylinderCollider::GetPoints(std::vector<Vector3>& points) const
{
	UNREFERENCED_PARAMETER(points);
}