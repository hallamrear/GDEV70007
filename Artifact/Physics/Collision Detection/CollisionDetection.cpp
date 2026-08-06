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

		Vector3 hitPoint;
		hitPoint.x = colliderA->GetAttachedEntity().GetPosition().x + (normal.x * depth);
		hitPoint.y = colliderA->GetAttachedEntity().GetPosition().y + (normal.y * depth);
		hitPoint.z = colliderA->GetAttachedEntity().GetPosition().z + (normal.z * depth);

		Contact contact;
		contact.Depth = depth; 
		contact.Normal = { normal.x, normal.y, normal.z };
		contact.HitPoint = hitPoint;
		
		manifold->ContactPoints.push_back(contact);
	}

	return distance < r2;
}

bool CollisionDetection::AABBSphereCollision(const Collider* boxCollider, const Collider* sphereCollider, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(boxCollider);
	UNREFERENCED_PARAMETER(sphereCollider);

	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
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

bool CollisionDetection::CheckCollision(const Collider* colliderA, const Collider* colliderB, CollisionManifold* manifold)
{
	assert(colliderA);
	assert(colliderB);
	assert(s_CollisionFunctionArray[colliderA->GetType()][colliderB->GetType()] != nullptr);

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
