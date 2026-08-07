#include "pch.h"
#include "ConvexHullCollider.h"
#include <Rendering/Geometry/Mesh.h>
#include <Physics/Quickhull/Quickhull.h>
#include "../../World/Entity.h"

ConvexHullCollider::ConvexHullCollider(const Entity& entity, const ModelRef& modelRef) : Collider(COLLIDER_TYPE_CONVEX_HULL, entity)
{
	assert(modelRef);
	m_ConvexHull = modelRef->GetMeshes()[0]->GetConvexHull();
	m_ColliderModel = modelRef;

	m_Size.x = (m_ColliderModel->GetMeshes()[0]->GetMaxVertexLocalSpace().x - m_ColliderModel->GetMeshes()[0]->GetMinVertexLocalSpace().x);
	m_Size.y = (m_ColliderModel->GetMeshes()[0]->GetMaxVertexLocalSpace().y - m_ColliderModel->GetMeshes()[0]->GetMinVertexLocalSpace().y);
	m_Size.z = (m_ColliderModel->GetMeshes()[0]->GetMaxVertexLocalSpace().z - m_ColliderModel->GetMeshes()[0]->GetMinVertexLocalSpace().z);
	UpdateBoundingVolume();
}

ConvexHullCollider::~ConvexHullCollider()
{
	m_ColliderModel = nullptr;
}

Vector3 ConvexHullCollider::GetSupportPoint(const Vector3& direction) const
{
	Vector3 localDir = direction;
	int x = 0;
	glm::vec3 support = m_ConvexHull->FindSupportVertex({ localDir.x, localDir.y, localDir.z }, x);
	Vector3 wSupport = { support.x, support.y, support.z };

	Matrix4x4 matrix = GetTransformMatrix();
	DirectX::XMStoreFloat3(&wSupport, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&wSupport), DirectX::XMLoadFloat4x4(&matrix)));
	
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

void ConvexHullCollider::RenderCollisionModel(Renderer& renderer)
{
    const ConvexHull* hull = m_ColliderModel->GetMeshes()[0]->GetConvexHull();
	
	if(hull != nullptr)
	{
		renderer.SetDebugDrawMode();
		renderer.Render(*hull, m_NarrowPhaseTexture, GetTransformMatrix());
		renderer.SetDefaultDrawMode();
	}
}