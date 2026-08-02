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
	int x = 0;
	glm::vec3 support = m_ConvexHull->FindSupportVertex({ direction.x, direction.y, direction.z }, x);
	return { support.x, support.y, support.z };
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
