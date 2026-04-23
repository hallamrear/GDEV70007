#include "pch.h"
#include "CollisionDetection.h"

const CollisionDetection::CollisionFunctionArray CollisionDetection::c_CollisionFunctionArray = 
{
	{ 
		{ { CollisionDetection::BoxBoxCollision, CollisionDetection::BoxBoxCollision, &CollisionDetection::BoxBoxCollision } },
		{ { CollisionDetection::BoxBoxCollision, &CollisionDetection::BoxBoxCollision, &CollisionDetection::BoxBoxCollision } },
		{ { CollisionDetection::BoxBoxCollision, &CollisionDetection::BoxBoxCollision, &CollisionDetection::BoxBoxCollision } }
	},
};

bool CollisionDetection::BoxBoxCollision(const BoxCollider& colliderA, const BoxCollider& colliderB)
{
	c_CollisionFunctionArray[colliderA->GetType()]

	return false;
}

bool CollisionDetection::CheckCollision(const Collider& colliderA, const Collider& colliderB)
{
	return false;
}
