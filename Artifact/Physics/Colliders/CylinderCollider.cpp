#include "pch.h"
#include "CylinderCollider.h"
#include <World/Entity.h>
#include <System/AssetManagement.h>

Matrix4x4 CylinderCollider::GetTransformMatrix() const
{
	Matrix4x4 transformMatrix = IdentityMatrix;
	DirectX::XMStoreFloat4x4(&transformMatrix,
		DirectX::XMMatrixScaling(m_Size.x, m_Size.y, m_Size.z) *
		DirectX::XMMatrixTranslation(m_AttachedEntity.GetPosition().x, m_AttachedEntity.GetPosition().y, m_AttachedEntity.GetPosition().z) *
		DirectX::XMLoadFloat4x4(&m_OffsetMatrix));
	return transformMatrix;
}

CylinderCollider::CylinderCollider(const Entity& entity, const float& radius, const float& height) : Collider(COLLIDER_TYPE::COLLIDER_TYPE_CAPSULE, entity)
{
	SetSize(Vector3(radius, height, radius));
}

CylinderCollider::~CylinderCollider()
{
	SetSize(Vector3(0.0f, 0.0f, 0.0f));
}

void CylinderCollider::SetSize(const Vector3& radius_height_radius)
{
	m_Size.x = radius_height_radius.x;
	m_Size.y = radius_height_radius.y / 2.0f;
	m_Size.z = radius_height_radius.z;
}

Vector3 CylinderCollider::GetSupportPoint(const Vector3& direction) const
{
	glm::vec3 dir = { direction.x, direction.y, direction.z };
	glm::vec3 result = { 0.0f, 0.0f, 0.0f };

	//glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	//float udotd = glm::dot(up, dir);
	//glm::vec3 w = dir - (udotd * up);
	//
	//float sign = (signbit(udotd) ? 1.0f : -1.0f);
	//glm::vec3 centre = glm::vec3(0.0f, 0.0f, 0.0f);
	//
	//if (w.y < FLT_EPSILON)
	//{
	//	result = centre + (sign * m_Size.y * up);
	//}
	//else
	//{
	//	glm::vec3 norm = glm::normalize(w);
	//	result = centre + (sign * m_Size.y * up) + (m_Size.x * (w / glm::length(w)));
	//}




	glm::vec3 dir_xz = glm::vec3(dir.x, 0, dir.z);
	result = glm::normalize(dir_xz) * m_Size.x;

	float height = m_Size.y;
	result.y = (dir.y > 0) ? height : -height;

	result.x += m_AttachedEntity.GetPosition().x;
	result.y += m_AttachedEntity.GetPosition().y;
	result.z += m_AttachedEntity.GetPosition().z;

	return Vector3(result.x, result.y, result.z);
}

void CylinderCollider::GetPoints(std::vector<Vector3>& points) const
{
	UNREFERENCED_PARAMETER(points);
}