#include "pch.h"
#include "ConvexHullCollider.h"
#include <Rendering/Geometry/Mesh.h>

ConvexHullCollider::ConvexHullCollider(const Entity& entity, const ModelRef& modelRef) : Collider(COLLIDER_TYPE_CONVEX_HULL, entity)
{
	assert(modelRef);
	m_ConvexHull = modelRef->GetMeshes()[0]->GetConvexHull();
}

ConvexHullCollider::~ConvexHullCollider()
{
}

void ConvexHullCollider::SetSize(const Vector3& size)
{
	UNREFERENCED_PARAMETER(size);
}

Matrix4x4 ConvexHullCollider::GetTransformMatrix() const
{
	return IdentityMatrix;
}
