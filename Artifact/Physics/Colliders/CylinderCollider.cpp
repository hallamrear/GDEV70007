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

// Source - https://stackoverflow.com/a/4609795
// Posted by user79758, modified by community. See post 'Timeline' for change history
// Retrieved 2026-08-21, License - CC BY-SA 4.0

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

Vector3 CylinderCollider::GetSupportPoint(const Vector3& direction) const
{
	glm::vec3 dir = { direction.x, direction.y, direction.z };
	dir = glm::normalize(dir);

	glm::vec3 centreAxis = { m_AttachedEntity.GetUpVector().x, m_AttachedEntity.GetUpVector().y, m_AttachedEntity.GetUpVector().z };
	centreAxis = glm::normalize(centreAxis);
	
	glm::vec3 centre = { m_AttachedEntity.GetPosition().x, m_AttachedEntity.GetPosition().y, m_AttachedEntity.GetPosition().z };

	//glm::vec3 w = dir - glm::dot(centreAxis, dir) * centreAxis;
	//glm::vec3 supp = centre + (sgn(glm::dot(centreAxis, dir)) * m_Size.y * centreAxis) + (m_Size.x * glm::normalize(w));


	glm::vec3 circleDir = glm::normalize(glm::vec3(dir.x, 0.0f, dir.z));
	float height = (dir.y > 0) ? m_Size.y : -m_Size.y;
	glm::vec3 vertical = height * centreAxis;
	glm::vec3 supp = centre + vertical + (circleDir * (m_Size.x / 2.0f));

	Vector3 result = { 0.0f, 0.0f, 0.0f };

	result = { supp.x, supp.y, supp.z };

	//glm::vec3 dir_xz = glm::normalize(glm::vec3(dir.x, 0, dir.z)) * m_Size.x;
	//result = { dir_xz.x, 0.0f, dir_xz.z };

	//float height = m_Size.y / 2.0f;
	//result.y = (dir.y > 0) ? height : -height;

	//DirectX::XMStoreFloat3(&result, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&result), DirectX::XMLoadFloat4x4(&m_AttachedEntity.GetWorldMatrix())));

	return Vector3(result.x, result.y, result.z);
}

void CylinderCollider::GetPoints(std::vector<Vector3>& points) const
{
	UNREFERENCED_PARAMETER(points);
}