#include "pch.h"
#include "ConvexHullCollider.h"

ConvexHullCollider::ConvexHullCollider(const Entity& entity) : Collider(COLLIDER_TYPE_CONVEX_HULL, entity)
{

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
