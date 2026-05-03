#include "pch.h"
#include "CollisionDetection.h"

CollisionDetection::CollisionFunction CollisionDetection::s_CollisionFunctionArray[COLLIDER_TYPE::COLLIDER_TYPE_COUNT][COLLIDER_TYPE::COLLIDER_TYPE_COUNT] = 
{ nullptr };

bool CollisionDetection::SeperatingAxisTheorem(const Collider* colliderA, const Collider* colliderB)
{
	assert(colliderA);
	assert(colliderB);

	return false;
}


bool CollisionDetection::CheckCollision(const Collider* colliderA, const Collider* colliderB, const CollisionManifold* manifold)
{
	assert(colliderA);
	assert(colliderB);
	assert(s_CollisionFunctionArray[colliderA->GetType()][colliderB->GetType()]);
	return s_CollisionFunctionArray[colliderA->GetType()][colliderB->GetType()](colliderA, colliderB, manifold);
}
