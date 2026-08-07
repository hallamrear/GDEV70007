#include "pch.h"
#include "AABBCollider.h"
#include <World/Entity.h>
#include <System/Maths.h>

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
	UpdateBoundingVolume();
}

Vector3 AABBCollider::GetMaxCornerWorldSpace() const
{
	Vector3 bounds = GetBoundingVolumeExtents();
	Vector3 max = { m_AttachedEntity.GetPosition().x + bounds.x, m_AttachedEntity.GetPosition().y + bounds.y, m_AttachedEntity.GetPosition().z + bounds.z};
	return max;
}

Matrix4x4 AABBCollider::GetTransformMatrix() const
{
	Vector3 bounds = GetBoundingVolumeExtents();
	Matrix4x4 transformMatrix = IdentityMatrix;
	DirectX::XMStoreFloat4x4(&transformMatrix,
		DirectX::XMMatrixScaling(bounds.x, bounds.y, bounds.z) *
		DirectX::XMMatrixTranslation(m_AttachedEntity.GetPosition().x, m_AttachedEntity.GetPosition().y, m_AttachedEntity.GetPosition().z) *
		DirectX::XMLoadFloat4x4(&m_OffsetMatrix));
	return transformMatrix;
}

Vector3 AABBCollider::GetMinCornerWorldSpace() const
{
	Vector3 bounds = GetBoundingVolumeExtents();
	Vector3 min = { m_AttachedEntity.GetPosition().x - bounds.x, m_AttachedEntity.GetPosition().y - bounds.y, m_AttachedEntity.GetPosition().z - bounds.z };
	return min;
}

Vector3 AABBCollider::GetMaxCornerLocalSpace() const
{
	return GetBoundingVolumeExtents();
}

Vector3 AABBCollider::GetMinCornerLocalSpace() const
{
	Vector3 bounds = GetBoundingVolumeExtents();
	return Vector3(bounds.x * -1.0f, bounds.y * -1.0f, bounds.z * -1.0f);
}

Vector3 AABBCollider::GetSupportPoint(const Vector3& direction) const
{
	Vector3 bounds = GetBoundingVolumeExtents();
	Vector3 maxPoint = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3 corner = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3 scaledPoint = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	float maxDistance = -FLT_MAX;
	float distance = -FLT_MAX;
	Matrix4x4 matrix = GetTransformMatrix();

	for (int i = 0; i < 8; i++)
	{
		scaledPoint = Vector3(Points[i].x * bounds.x, Points[i].y * bounds.y, Points[i].z * bounds.z);

		corner.x = scaledPoint.x + m_AttachedEntity.GetPosition().x;
		corner.y = scaledPoint.y + m_AttachedEntity.GetPosition().y;
		corner.z = scaledPoint.z + m_AttachedEntity.GetPosition().z;

		DirectX::XMStoreFloat(&distance, DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&corner), DirectX::XMLoadFloat3(&direction)));

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

glm::vec3 AABBCollider::ClosestPointOnColliderToPoint(const glm::vec3& point) const
{
	UNREFERENCED_PARAMETER(point);
	//https://www.youtube.com/watch?v=VmtNPguCTjQ
	//17:40

	Vector3 temp_max = GetMaxCornerWorldSpace();
	Vector3 temp_min = GetMinCornerWorldSpace();
	glm::vec3 max = { temp_max.x, temp_max.y, temp_max.z };
	glm::vec3 min = { temp_min.x, temp_min.y, temp_min.z };

	glm::vec3 closest;
	// For each coordinate axis, if the point coordinate value is
	// outside box, clamp it to the box, else keep it as is
	for (int i = 0; i < 3; i++) {
		float v = closest[i];
		if (v < min[i]) v = min[i]; // v = max( v, b.min[i] )
		if (v > max[i]) v = max[i]; // v = min( v, b.max[i] )
		closest[i] = v;
	}

	return closest;
}

float AABBCollider::SqrDistanceToAABB(const glm::vec3& point) const
{
	Vector3 temp_max = GetMaxCornerWorldSpace();
	Vector3 temp_min = GetMinCornerWorldSpace();
	glm::vec3 max = { temp_max.x, temp_max.y, temp_max.z };
	glm::vec3 min = { temp_min.x, temp_min.y, temp_min.z };

	float sqDist = 0.0f;
	
	for (int i = 0; i < 3; i++) {
		// for each axis count any excess distance outside box extents
		float v = point[i];
		if (v < min[i]) sqDist += (min[i] - v) * (min[i] - v);
		if (v > max[i]) sqDist += (v - max[i]) * (v - max[i]);
	}

	return sqDist;
}
