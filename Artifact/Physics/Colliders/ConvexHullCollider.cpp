#include "pch.h"
#include "ConvexHullCollider.h"
#include <Rendering/Geometry/Mesh.h>
#include <Physics/Quickhull/Quickhull.h>
#include "../../World/Entity.h"

ConvexHullCollider::ConvexHullCollider(const Entity& entity, const ModelRef& modelRef) : Collider(COLLIDER_TYPE_CONVEX_HULL, entity)
{
	assert(modelRef);
	m_ConvexHull = modelRef->GetMeshes()[0]->GetConvexHull();
}

ConvexHullCollider::~ConvexHullCollider()
{

}

Vector3 ConvexHullCollider::GetFurthestPointInDirection(const Vector3& direction) const
{
	Matrix4x4 rotMatrix;
	GetAttachedEntity().GetRotationMatrix(rotMatrix);

	Matrix4x4 invRotMatrix;
	GetAttachedEntity().GetInverseRotationMatrix(invRotMatrix);

	Vector3 localDir = direction;
	DirectX::XMStoreFloat3(&localDir, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&direction), DirectX::XMLoadFloat4x4(&invRotMatrix)));

	int x = 0;
	glm::vec3 support = m_ConvexHull->FindSupportVertex({ localDir.x, localDir.y, localDir.z }, x);
	Vector3 wSupport = { support.x, support.y, support.z };

	DirectX::XMStoreFloat3(&wSupport, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&wSupport), DirectX::XMLoadFloat4x4(&rotMatrix)));
	
	wSupport.x += GetAttachedEntity().GetPosition().x;
	wSupport.y += GetAttachedEntity().GetPosition().y;
	wSupport.z += GetAttachedEntity().GetPosition().z;
	
	return wSupport;
}

void ConvexHullCollider::GetPoints(std::vector<Vector3>& points) const
{
	points.clear();
}

void ConvexHullCollider::SetSize(const Vector3& size)
{
	UNREFERENCED_PARAMETER(size);
}

Matrix4x4 ConvexHullCollider::GetTransformMatrix() const
{
	return m_AttachedEntity.GetWorldMatrix();
}
