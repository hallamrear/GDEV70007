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

bool CollisionDetection::Use_GJK = true;
bool CollisionDetection::Use_Dispatch_Table = true;

CollisionDetection::CollisionFunction CollisionDetection::s_CollisionFunctionArray[COLLIDER_TYPE::COLLIDER_TYPE_COUNT][COLLIDER_TYPE::COLLIDER_TYPE_COUNT] = 
{
	/*				Sphere											AABB										 OBB										 Cylinder										 Convex												*/
	/* Sphere  */ { CollisionDetection::SphereSphereCollision,		CollisionDetection::AABBSphereCollision,	 CollisionDetection::OBBSphereCollision,	 CollisionDetection::CylinderSphereCollision,	 CollisionDetection::ConvexHullSphereCollision		},
	/* AABB    */ { CollisionDetection::AABBSphereCollision,		CollisionDetection::AABBAABBCollision,		 CollisionDetection::OBBAABBCollision,		 CollisionDetection::CylinderAABBCollision,		 CollisionDetection::ConvexHullAABBCollision		},
	/* OBB     */ { CollisionDetection::OBBSphereCollision,			CollisionDetection::OBBAABBCollision,		 CollisionDetection::OBBOBBCollision,		 CollisionDetection::CylinderOBBCollision,		 CollisionDetection::ConvexHullOBBCollision			},
	/* Cylinder */ { CollisionDetection::CylinderSphereCollision,		CollisionDetection::CylinderAABBCollision,	 CollisionDetection::CylinderOBBCollision,	 CollisionDetection::CylinderCylinderCollision,	 CollisionDetection::ConvexHullCylinderCollision		},
	/* Convex  */ { CollisionDetection::ConvexHullSphereCollision,	CollisionDetection::ConvexHullAABBCollision, CollisionDetection::ConvexHullOBBCollision, CollisionDetection::ConvexHullCylinderCollision, CollisionDetection::ConvexHullConvexHullCollision	}
};

bool CollisionDetection::DispatchSeperatingAxisTheorem(const Collider* colliderA, const Collider* colliderB, CollisionManifold* manifold)
{
	assert(colliderA); assert(colliderB);
	return SeparatingAxisTheorem::CheckCollision(*colliderA, *colliderB, manifold);
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

	//const AABBCollider* aabb = dynamic_cast<const AABBCollider*>(boxCollider);
	//
	//assert(aabb);
	//
	//glm::vec3 spherePos = { sphereCollider->GetAttachedEntity().GetPosition().x, sphereCollider->GetAttachedEntity().GetPosition().y, sphereCollider->GetAttachedEntity().GetPosition().z };
	//
	//float sqrDist = aabb->SqrDistanceToAABB(spherePos);
	//float sqrRadius = sphereCollider->GetSize().x * sphereCollider->GetSize().x;
	//
	//bool collision = (sqrDist <= sqrRadius);
	//
	//if (collision && manifold != nullptr)
	//{
	//	//TODO : Make contact points.
	//	printf("AABBSphere manifold generation not finished.\n");
	//
	//	Contact contact;
	//	contact.Depth = (sqrRadius - sqrDist);
	//	glm::vec3 closest = aabb->ClosestPointOnColliderToPoint(spherePos);
	//	contact.HitPoint = { closest.x, closest.y, closest.z };
	//
	//	manifold->ContactPoints.push_back(contact);
	//}
	//
	//return collision;
}

bool CollisionDetection::AABBAABBCollision(const Collider* boxColliderA, const Collider* boxColliderB, CollisionManifold* manifold)
{
	assert(boxColliderA);
	assert(boxColliderB);
	return DispatchSeperatingAxisTheorem(boxColliderA, boxColliderB, manifold);
}

bool CollisionDetection::OBBSphereCollision(const Collider* obbCollider, const Collider* sphereCollider, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(obbCollider);
	UNREFERENCED_PARAMETER(sphereCollider);

	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
	}

	return false;
}

bool CollisionDetection::OBBAABBCollision(const Collider* obbCollider, const Collider* aabbCollider, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(obbCollider);
	UNREFERENCED_PARAMETER(aabbCollider);

	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
	}

	return false;
}

bool CollisionDetection::OBBOBBCollision(const Collider* obbColliderA, const Collider* obbColliderB, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(obbColliderA);
	UNREFERENCED_PARAMETER(obbColliderB);

	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
	}

	return false;
}

bool CollisionDetection::CylinderSphereCollision(const Collider* capsuleCollider, const Collider* sphereCollider, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(capsuleCollider);
	UNREFERENCED_PARAMETER(sphereCollider);

	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
	}

	return false;
}

bool CollisionDetection::CylinderAABBCollision(const Collider* capsuleCollider, const Collider* aabbCollider, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(capsuleCollider);
	UNREFERENCED_PARAMETER(aabbCollider);

	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
	}

	return DispatchGilbertJohnsonKeethri(capsuleCollider, aabbCollider, manifold);
}

bool CollisionDetection::CylinderOBBCollision(const Collider* capsuleCollider, const Collider* obbCollider, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(capsuleCollider);
	UNREFERENCED_PARAMETER(obbCollider);

	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
	}

	return false;
}

bool CollisionDetection::CylinderCylinderCollision(const Collider* capsuleColliderA, const Collider* capsuleColliderB, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(capsuleColliderA);
	UNREFERENCED_PARAMETER(capsuleColliderB);

	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
	}

	return false;
}

bool CollisionDetection::ConvexHullSphereCollision(const Collider* convexHullCollider, const Collider* sphereCollider, CollisionManifold* manifold)
{
	assert(convexHullCollider);
	assert(sphereCollider);

	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
	}

	return false;
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
	UNREFERENCED_PARAMETER(capsuleCollider);
	UNREFERENCED_PARAMETER(convexHullCollider);

	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
	}

	return false;
}

bool CollisionDetection::ConvexHullConvexHullCollision(const Collider* convexHullColliderA, const Collider* convexHullColliderB, CollisionManifold* manifold)
{
	assert(convexHullColliderA);
	assert(convexHullColliderB);
	return DispatchGilbertJohnsonKeethri(convexHullColliderA, convexHullColliderB, manifold);
}

bool CollisionDetection::BroadPhaseCollision(const Collider* colliderA, const Collider* colliderB)
{
	Vector3 colliderAPos = colliderA->GetAttachedEntity().GetPosition();
	Vector3 colliderAExtents = colliderA->GetBoundingVolumeExtents();
	Vector3 colliderBPos = colliderB->GetAttachedEntity().GetPosition();
	Vector3 colliderBExtents = colliderB->GetBoundingVolumeExtents();

	glm::vec3 max_a = { colliderAPos.x + colliderAExtents.x, colliderAPos.y + colliderAExtents.y, colliderAPos.z + colliderAExtents.z };
	glm::vec3 min_a = { colliderAPos.x - colliderAExtents.x, colliderAPos.y - colliderAExtents.y, colliderAPos.z - colliderAExtents.z };

	glm::vec3 max_b = { colliderBPos.x + colliderBExtents.x, colliderBPos.y + colliderBExtents.y, colliderBPos.z + colliderBExtents.z };
	glm::vec3 min_b = { colliderBPos.x - colliderBExtents.x, colliderBPos.y - colliderBExtents.y, colliderBPos.z - colliderBExtents.z };

	// Exit with no intersection if separated along an axis
	if (max_a[0] < min_b[0] || min_a[0] > max_b[0]) return false;
	if (max_a[1] < min_b[1] || min_a[1] > max_b[1]) return false;
	if (max_a[2] < min_b[2] || min_a[2] > max_b[2]) return false;
	
	// Overlapping on all axes means AABBs are intersecting
	return true;
}

bool CollisionDetection::AABBAABBCollision(const glm::vec3& minA, const glm::vec3& maxA, const glm::vec3& minB, const glm::vec3& maxB)
{
	// Exit with no intersection if separated along an axis
	if (maxA[0] < minB[0] || minA[0] > maxB[0]) return false;
	if (maxA[1] < minB[1] || minA[1] > maxB[1]) return false;
	if (maxA[2] < minB[2] || minA[2] > maxB[2]) return false;

	// Overlapping on all axes means AABBs are intersecting
	return true;
}

bool CollisionDetection::CheckNarrowPhaseCollision(const Collider* colliderA, const Collider* colliderB, CollisionManifold* manifold)
{
	assert(colliderA);
	assert(colliderB);
	assert(s_CollisionFunctionArray[colliderA->GetType()][colliderB->GetType()] != nullptr);

	if (BroadPhaseCollision(colliderA, colliderB) == false)
	{
		return false;
	}

	if (Use_Dispatch_Table)
	{
		if (colliderB->GetType() <= colliderA->GetType())
		{
			return s_CollisionFunctionArray[colliderA->GetType()][colliderB->GetType()](colliderA, colliderB, manifold);
		}
		else
		{
			return s_CollisionFunctionArray[colliderA->GetType()][colliderB->GetType()](colliderB, colliderA, manifold);
		}
	}
	else
	{
	return (Use_GJK ? GJK::CheckCollision(*colliderA, *colliderB, manifold) : DispatchSeperatingAxisTheorem(colliderA, colliderB, manifold));
	}
}
