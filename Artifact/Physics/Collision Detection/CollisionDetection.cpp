#include "pch.h"
#include "CollisionDetection.h"
#include <World/Entity.h>
#include <Physics/Colliders/SphereCollider.h>
#include <Physics/Colliders/AABBCollider.h>
#include <Physics/Colliders/ConvexHullCollider.h>

#include <Physics/GJK/GJK.h>
#include <Physics/EPA/EPA.h>
#include <Physics/SAT/SeparatingAxisTheorem.h>
#include <glm/gtx/norm.hpp>

CollisionDetection::CollisionFunction CollisionDetection::s_CollisionFunctionArray[COLLIDER_TYPE::COLLIDER_TYPE_COUNT][COLLIDER_TYPE::COLLIDER_TYPE_COUNT] = 
{
	/*				Sphere											AABB										 OBB										 Cylinder										 Convex												*/
	/* Sphere  */  { CollisionDetection::SphereSphereCollision,		CollisionDetection::AABBSphereCollision,	 CollisionDetection::OBBSphereCollision,	 CollisionDetection::CylinderSphereCollision,	 CollisionDetection::ConvexHullSphereCollision		},
	/* AABB    */  { CollisionDetection::AABBSphereCollision,		CollisionDetection::AABBAABBCollision,		 CollisionDetection::OBBAABBCollision,		 CollisionDetection::CylinderAABBCollision,		 CollisionDetection::ConvexHullAABBCollision		},
	/* OBB     */  { CollisionDetection::OBBSphereCollision,		CollisionDetection::OBBAABBCollision,		 CollisionDetection::OBBOBBCollision,		 CollisionDetection::CylinderOBBCollision,		 CollisionDetection::ConvexHullOBBCollision			},
	/* Cylinder */ { CollisionDetection::CylinderSphereCollision,	CollisionDetection::CylinderAABBCollision,	 CollisionDetection::CylinderOBBCollision,	 CollisionDetection::CylinderCylinderCollision,	 CollisionDetection::ConvexHullCylinderCollision		},
	/* Convex  */  { CollisionDetection::ConvexHullSphereCollision,	CollisionDetection::ConvexHullAABBCollision, CollisionDetection::ConvexHullOBBCollision, CollisionDetection::ConvexHullCylinderCollision, CollisionDetection::ConvexHullConvexHullCollision	}
};

bool CollisionDetection::DispatchSeperatingAxisTheorem(const Collider* colliderA, const Collider* colliderB, CollisionManifold* manifold)
{
	assert(colliderA); assert(colliderB);
	return GJK::CheckCollision(*colliderA, *colliderB, manifold);
	//return SeparatingAxisTheorem::CheckCollision(*colliderA, *colliderB, manifold);
}

bool CollisionDetection::DispatchGilbertJohnsonKeethri(const Collider* colliderA, const Collider* colliderB, CollisionManifold* manifold)
{
	assert(colliderA); assert(colliderB);
	return GJK::CheckCollision(*colliderA, *colliderB, manifold);
}

bool CollisionDetection::SphereSphereCollision(const Collider* colliderA, const Collider* colliderB, CollisionManifold* manifold)
{
	assert(colliderA); assert(colliderB);
	float r2 = (colliderA->GetSize().x * colliderA->GetSize().x) + (colliderB->GetSize().x * colliderB->GetSize().x);
	
	glm::vec3 posA = { colliderA->GetAttachedEntity().GetPosition().x, colliderA->GetAttachedEntity().GetPosition().y, colliderA->GetAttachedEntity().GetPosition().z };
	glm::vec3 posB = { colliderB->GetAttachedEntity().GetPosition().x, colliderB->GetAttachedEntity().GetPosition().y, colliderB->GetAttachedEntity().GetPosition().z };

	glm::vec3 direction = posB - posA;
	float distance = glm::length2(direction);
	
	if (manifold != nullptr)
	{
		float depth = (sqrtf(r2 - distance));
		glm::vec3 normal = glm::normalize(direction);

		Vector3 hitPointA;
		hitPointA.x = colliderA->GetAttachedEntity().GetPosition().x + (normal.x * colliderA->GetSize().x);
		hitPointA.y = colliderA->GetAttachedEntity().GetPosition().y + (normal.y * colliderA->GetSize().x);
		hitPointA.z = colliderA->GetAttachedEntity().GetPosition().z + (normal.z * colliderA->GetSize().x);

		Vector3 hitPointB;
		hitPointB.x = colliderB->GetAttachedEntity().GetPosition().x + (normal.x * colliderB->GetSize().x);
		hitPointB.y = colliderB->GetAttachedEntity().GetPosition().y + (normal.y * colliderB->GetSize().x);
		hitPointB.z = colliderB->GetAttachedEntity().GetPosition().z + (normal.z * colliderB->GetSize().x);

		Contact contactA;
		contactA.Depth = depth;
		contactA.Normal = { normal.x, normal.y, normal.z };
		contactA.HitPoint = hitPointA;

		Contact contactB;
		contactB.Depth = depth;
		contactB.Normal = { normal.x, normal.y, normal.z };
		contactB.HitPoint = hitPointB;

		manifold->ContactPoints.push_back(contactA);
		manifold->ContactPoints.push_back(contactB);
	}

	return distance < r2;
}

bool CollisionDetection::AABBSphereCollision(const Collider* boxCollider, const Collider* sphereCollider, CollisionManifold* manifold)
{
	glm::vec3 spherePos = { sphereCollider->GetAttachedEntity().GetPosition().x, sphereCollider->GetAttachedEntity().GetPosition().y, sphereCollider->GetAttachedEntity().GetPosition().z };
	glm::vec3 boxPos = { boxCollider->GetAttachedEntity().GetPosition().x, boxCollider->GetAttachedEntity().GetPosition().y, boxCollider->GetAttachedEntity().GetPosition().z };

	glm::vec3 delta = boxPos - spherePos;

	glm::vec3 closestPointOnBox = { 
		std::clamp(delta.x, -boxCollider->GetSize().x, boxCollider->GetSize().x),
		std::clamp(delta.y, -boxCollider->GetSize().y, boxCollider->GetSize().y),
		std::clamp(delta.z, -boxCollider->GetSize().z, boxCollider->GetSize().z)
	};

	glm::vec3 localPoint = delta - closestPointOnBox;
	float distance = glm::length(localPoint);
	float radius = sphereCollider->GetSize().x;

	if (distance < radius)
	{
		glm::vec3 normal = glm::normalize(localPoint);
		float depth = radius - distance;

		glm::vec3 hitPointA = boxPos + (-normal * radius);
		glm::vec3 hitPointB = spherePos;

		Contact contactA;
		contactA.Normal = { normal.x, normal.y, normal.z };
		contactA.Depth = depth;
		contactA.HitPoint = { hitPointA.x, hitPointA.y, hitPointA.z };

		Contact contactB;
		contactB.Normal = { normal.x, normal.y, normal.z };
		contactB.Depth = depth;
		contactB.HitPoint = { hitPointB.x, hitPointB.y, hitPointB.z };

		manifold->ContactPoints.push_back(contactA);
		manifold->ContactPoints.push_back(contactB);

		return true;
	}

	return false;
}

bool CollisionDetection::AABBAABBCollision(const Collider* boxColliderA, const Collider* boxColliderB, CollisionManifold* manifold)
{
	assert(boxColliderA);
	assert(boxColliderB);
	return DispatchSeperatingAxisTheorem(boxColliderA, boxColliderB, manifold);
}

bool CollisionDetection::OBBSphereCollision(const Collider* obbCollider, const Collider* sphereCollider, CollisionManifold* manifold)
{
	assert(obbCollider);
	assert(sphereCollider);
	return DispatchGilbertJohnsonKeethri(obbCollider, sphereCollider, manifold);
}

bool CollisionDetection::OBBAABBCollision(const Collider* obbCollider, const Collider* aabbCollider, CollisionManifold* manifold)
{
	assert(aabbCollider);
	assert(obbCollider);
	return DispatchSeperatingAxisTheorem(obbCollider, aabbCollider, manifold);
}

bool CollisionDetection::OBBOBBCollision(const Collider* obbColliderA, const Collider* obbColliderB, CollisionManifold* manifold)
{
	assert(obbColliderA);
	assert(obbColliderB);
	return DispatchGilbertJohnsonKeethri(obbColliderA, obbColliderB, manifold);
}

bool CollisionDetection::CylinderSphereCollision(const Collider* capsuleCollider, const Collider* sphereCollider, CollisionManifold* manifold)
{
	assert(capsuleCollider);
	assert(sphereCollider);
	return DispatchGilbertJohnsonKeethri(capsuleCollider, sphereCollider, manifold);
}

bool CollisionDetection::CylinderAABBCollision(const Collider* capsuleCollider, const Collider* aabbCollider, CollisionManifold* manifold)
{
	assert(capsuleCollider);
	assert(aabbCollider);
	return DispatchGilbertJohnsonKeethri(capsuleCollider, aabbCollider, manifold);
}

bool CollisionDetection::CylinderOBBCollision(const Collider* capsuleCollider, const Collider* obbCollider, CollisionManifold* manifold)
{
	assert(capsuleCollider);
	assert(obbCollider);
	return DispatchGilbertJohnsonKeethri(capsuleCollider, obbCollider, manifold);
}

bool CollisionDetection::CylinderCylinderCollision(const Collider* capsuleColliderA, const Collider* capsuleColliderB, CollisionManifold* manifold)
{
	assert(capsuleColliderA);
	assert(capsuleColliderB);
	return DispatchGilbertJohnsonKeethri(capsuleColliderA, capsuleColliderB, manifold);
}

bool CollisionDetection::ConvexHullSphereCollision(const Collider* convexHullCollider, const Collider* sphereCollider, CollisionManifold* manifold)
{
	assert(convexHullCollider);
	assert(sphereCollider);
	return DispatchGilbertJohnsonKeethri(convexHullCollider, sphereCollider, manifold);
}

bool CollisionDetection::ConvexHullAABBCollision(const Collider* convexHullCollider, const Collider* aabbCollider, CollisionManifold* manifold)
{
	assert(convexHullCollider);
	assert(aabbCollider);
	return DispatchGilbertJohnsonKeethri(convexHullCollider, aabbCollider, manifold);
}

bool CollisionDetection::ConvexHullOBBCollision(const Collider* convexHullCollider, const Collider* obbCollider, CollisionManifold* manifold)
{
	assert(convexHullCollider);
	assert(obbCollider);
	return DispatchGilbertJohnsonKeethri(convexHullCollider, obbCollider, manifold);
}

bool CollisionDetection::ConvexHullCylinderCollision(const Collider* convexHullCollider, const Collider* capsuleCollider, CollisionManifold* manifold)
{
	assert(capsuleCollider);
	assert(convexHullCollider);
	return DispatchGilbertJohnsonKeethri(convexHullCollider, capsuleCollider, manifold);
}

bool CollisionDetection::ConvexHullConvexHullCollision(const Collider* convexHullColliderA, const Collider* convexHullColliderB, CollisionManifold* manifold)
{
	assert(convexHullColliderA);
	assert(convexHullColliderB);
	return DispatchGilbertJohnsonKeethri(convexHullColliderA, convexHullColliderB, manifold);
}

bool CollisionDetection::BroadPhaseCollision(const Collider* colliderA, const Collider* colliderB)
{
	glm::vec3 originA =  { colliderA->GetAttachedEntity().GetPosition().x, colliderA->GetAttachedEntity().GetPosition().y, colliderA->GetAttachedEntity().GetPosition().z };
	glm::vec3 extentsA = { 
		colliderA->GetAttachedEntity().GetCollider()->GetBoundingVolumeExtents().x / 2.0f,
		colliderA->GetAttachedEntity().GetCollider()->GetBoundingVolumeExtents().y / 2.0f,
		colliderA->GetAttachedEntity().GetCollider()->GetBoundingVolumeExtents().z / 2.0f };

	glm::vec3 maxA = { originA + extentsA };
	glm::vec3 minA = { originA - extentsA };

	glm::vec3 originB = { colliderB->GetAttachedEntity().GetPosition().x, colliderB->GetAttachedEntity().GetPosition().y, colliderB->GetAttachedEntity().GetPosition().z };
	glm::vec3 extentsB = {
		colliderB->GetAttachedEntity().GetCollider()->GetBoundingVolumeExtents().x / 2.0f,
		colliderB->GetAttachedEntity().GetCollider()->GetBoundingVolumeExtents().y / 2.0f,
		colliderB->GetAttachedEntity().GetCollider()->GetBoundingVolumeExtents().z / 2.0f };

	glm::vec3 maxB = { originB + extentsB };
	glm::vec3 minB = { originB - extentsB };

	bool collision = AABBAABBCollision(minA, maxA, minB, maxB);

 	return collision;
}

bool CollisionDetection::AABBAABBCollision(const glm::vec3& minA, const glm::vec3& maxA, const glm::vec3& minB, const glm::vec3& maxB)
{
	// Exit with no intersection if separated along an axis
	return (
		maxA[0] >= minB[0] && minA[0] <= maxB[0] &&
		maxA[1] >= minB[1] && minA[1] <= maxB[1] &&
		maxA[2] >= minB[2] && minA[2] <= maxB[2]);
}

bool CollisionDetection::CheckNarrowPhaseCollision(const Collider* colliderA, const Collider* colliderB, CollisionManifold* manifold)
{
	assert(colliderA);
	assert(colliderB);
	assert(s_CollisionFunctionArray[colliderA->GetType()][colliderB->GetType()] != nullptr);

	if (BroadPhaseCollision(colliderA, colliderB) == false)
	{
		if (manifold)
		{
			manifold->IsBroadPhaseColliding = false;
			manifold->IsNarrowPhaseColliding = false;
		}

		return false;
	}

	bool isNarrowPhaseColliding = false;

	if (colliderB->GetType() <= colliderA->GetType())
	{
		isNarrowPhaseColliding = s_CollisionFunctionArray[colliderA->GetType()][colliderB->GetType()](colliderA, colliderB, manifold);
	}
	else
	{
		isNarrowPhaseColliding = s_CollisionFunctionArray[colliderA->GetType()][colliderB->GetType()](colliderB, colliderA, manifold);
	}

	if (manifold)
	{
		manifold->IsBroadPhaseColliding = true;
		manifold->IsNarrowPhaseColliding = isNarrowPhaseColliding;
	}

	return isNarrowPhaseColliding;
}
