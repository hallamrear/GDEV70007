#include "pch.h"
#include "CollisionDetection.h"
#include <World/Entity.h>
#include <Physics/Colliders/SphereCollider.h>
#include <Physics/Colliders/AABBCollider.h>
#include <Physics/Colliders/ConvexHullCollider.h>

CollisionDetection::CollisionFunction CollisionDetection::s_CollisionFunctionArray[COLLIDER_TYPE::COLLIDER_TYPE_COUNT][COLLIDER_TYPE::COLLIDER_TYPE_COUNT] = 
{
	/* 0, 0 */										/* 0, 1 */									 /* 0, 2 */											/* 0, 3 */
	CollisionDetection::SphereSphereCollision,		CollisionDetection::BoxSphereCollision,		  CollisionDetection::ConvexHullSphereCollision,		CollisionDetection::ComplexMeshSphereCollision,
	/* 1, 0 */										/* 1, 1 */									 /* 1, 2 */											/* 1, 3 */
	CollisionDetection::BoxSphereCollision,			CollisionDetection::BoxBoxCollision,		 CollisionDetection::ConvexHullBoxCollision,			CollisionDetection::ComplexMeshBoxCollision,
	/* 2, 0 */										/* 2, 1 */									 /* 2, 2 */											/* 2, 3 */
	CollisionDetection::ConvexHullSphereCollision,  CollisionDetection::ConvexHullBoxCollision,  CollisionDetection::ConvexHullConvexHullCollision,	CollisionDetection::ComplexMeshConvexHullCollision,
	/* 3, 0 */										/* 3, 1 */									 /* 3, 2 */											/* 3, 3 */
	CollisionDetection::ComplexMeshSphereCollision, CollisionDetection::ComplexMeshBoxCollision, CollisionDetection::ComplexMeshConvexHullCollision, CollisionDetection::ComplexMeshComplexMeshCollision
};

bool CollisionDetection::SeperatingAxisTheorem(const Collider* colliderA, const Collider* colliderB, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(colliderA);
	UNREFERENCED_PARAMETER(colliderB);
	UNREFERENCED_PARAMETER(manifold);

	assert(colliderA);
	assert(colliderB);

	return false;
}

bool CollisionDetection::SphereSphereCollision(const Collider* sphereColliderA, const Collider* sphereColliderB, CollisionManifold* manifold)
{
	assert(sphereColliderA);
	assert(sphereColliderB);

	const SphereCollider* colliderA = (const SphereCollider*)(sphereColliderA);
	const SphereCollider* colliderB = (const SphereCollider*)(sphereColliderB);
	assert(colliderA);
	assert(colliderB);

	float r2 = (colliderA->GetSize().x * colliderA->GetSize().x) + (colliderB->GetSize().x * colliderB->GetSize().x);
	
	Vector3 direction;
	DirectX::XMStoreFloat3(&direction,
		DirectX::XMVectorSubtract(XMLoadFloat3(&colliderB->GetAttachedEntity().GetPosition()), XMLoadFloat3(&colliderA->GetAttachedEntity().GetPosition())));

	Vector3 lengthSqr = Vector3(0.0f, 0.0f, 0.0f);
	DirectX::XMStoreFloat3(&lengthSqr, DirectX::XMVector3LengthSq(XMLoadFloat3(&direction)));

	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");

		/*
		float depth = (sqrtf(r2 - lengthSqr.x));
		Vector3 normal;
		DirectX::XMStoreFloat3(&normal, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&direction)));

		Vector3 hitPoint;
		hitPoint.x = sphereColliderA->GetAttachedEntity().GetPosition().x + (normal.x * depth);
		hitPoint.y = sphereColliderA->GetAttachedEntity().GetPosition().y + (normal.y * depth);
		hitPoint.z = sphereColliderA->GetAttachedEntity().GetPosition().z + (normal.z * depth);

		Contact contact(*sphereColliderA, *sphereColliderB, hitPoint, normal, depth);
		manifold->ContactPoints.push_back(contact);
		*/
	}

	return lengthSqr.x < r2;
}

bool CollisionDetection::BoxSphereCollision(const Collider* boxCollider, const Collider* sphereCollider, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(boxCollider);
	UNREFERENCED_PARAMETER(sphereCollider);
	
	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
	}

	return false;
}

bool CollisionDetection::BoxBoxCollision(const Collider* boxColliderA, const Collider* boxColliderB, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(boxColliderA);
	UNREFERENCED_PARAMETER(boxColliderB);


	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
	}

	return false;
}

bool CollisionDetection::ConvexHullSphereCollision(const Collider* convexHullCollider, const Collider* sphereCollider, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(convexHullCollider);
	UNREFERENCED_PARAMETER(sphereCollider);

	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
	}

	return false;
}

bool CollisionDetection::ConvexHullBoxCollision(const Collider* convexHullCollider, const Collider* boxCollider, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(convexHullCollider);
	UNREFERENCED_PARAMETER(boxCollider);

	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
	}

	return false;
}

bool CollisionDetection::ConvexHullConvexHullCollision(const Collider* convexHullColliderA, const Collider* convexHullColliderB, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(convexHullColliderA);
	UNREFERENCED_PARAMETER(convexHullColliderB);

	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
	}

	return false;
}

bool CollisionDetection::ComplexMeshSphereCollision(const Collider* complexMeshCollider, const Collider* sphereCollider, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(complexMeshCollider);
	UNREFERENCED_PARAMETER(sphereCollider);

	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
	}

	return false;
}

bool CollisionDetection::ComplexMeshBoxCollision(const Collider* complexMeshCollider, const Collider* boxCollider, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(complexMeshCollider);
	UNREFERENCED_PARAMETER(boxCollider);

	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
	}

	return false;
}

bool CollisionDetection::ComplexMeshConvexHullCollision(const Collider* complexMeshCollider, const Collider* convexHullCollider, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(complexMeshCollider);
	UNREFERENCED_PARAMETER(convexHullCollider);

	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
	}

	return false;
}

bool CollisionDetection::ComplexMeshComplexMeshCollision(const Collider* complexMeshColliderA, const Collider* complexMeshColliderB, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(complexMeshColliderA);
	UNREFERENCED_PARAMETER(complexMeshColliderB);

	if (manifold != nullptr)
	{
		printf("Collision manifold is valid but not being constructed.\n");
	}

	return false;
}

bool CollisionDetection::CheckCollision(const Collider* colliderA, const Collider* colliderB, CollisionManifold* manifold)
{
	assert(colliderA);
	assert(colliderB);
	assert(s_CollisionFunctionArray[colliderA->GetType()][colliderB->GetType()]);
	return s_CollisionFunctionArray[colliderA->GetType()][colliderB->GetType()](colliderA, colliderB, manifold);
}
