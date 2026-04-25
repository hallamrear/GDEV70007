#include "pch.h"
#include "CollisionDetection.h"

bool CollisionDetection::SeperatingAxisTheorem(const Collider& colliderA, const Collider& colliderB)
{


	return false;
}


bool CollisionDetection::CheckCollision(const Collider& colliderA, const Collider& colliderB)
{
	switch (colliderA.GetType())
	{
	case COLLIDER_TYPE_AABB:
	{
		switch (colliderB.GetType())
		{
		case COLLIDER_TYPE_AABB: { BoxBoxCollision((const AABBCollider&)colliderA, (const AABBCollider&)colliderB); } break;
		case COLLIDER_TYPE_SPHERE: { BoxSphereCollision((const AABBCollider&)colliderA, (const SphereCollider&)colliderB); } break;
		case COLLIDER_TYPE_CONVEX_HULL: { ConvexHullBoxCollision((const ConvexHullCollider&)colliderA, (const AABBCollider&)colliderB); } break;
		case COLLIDER_TYPE_MESH: { ComplexMeshBoxCollision((const ComplexMeshCollider&)colliderA, (const AABBCollider&)colliderB); } break;
		default:
			assert(false);
			return false;
			break;
		}
	} 
	break;

	case COLLIDER_TYPE_SPHERE:
	{
		switch (colliderB.GetType())
		{
		case COLLIDER_TYPE_AABB: { BoxSphereCollision((const AABBCollider&)colliderB, (const SphereCollider&)colliderA); } break;
		case COLLIDER_TYPE_SPHERE: { SphereSphereCollision((const SphereCollider&)colliderA, (const SphereCollider&)colliderB); } break;
		case COLLIDER_TYPE_CONVEX_HULL: { ConvexHullSphereCollision((const ConvexHullCollider&)colliderB, (const SphereCollider&)colliderA); } break;
		case COLLIDER_TYPE_MESH: { ComplexMeshSphereCollision((const ComplexMeshCollider&)colliderA, (const AABBCollider&)colliderB); } break;
		default:
			assert(false);
			return false;
			break;
		}
	} 
	break;

	case COLLIDER_TYPE_CONVEX_HULL: 
	{

	} 
	break;





	default:
		assert(false);
		return false;
		break;
	}

	return false;
}
