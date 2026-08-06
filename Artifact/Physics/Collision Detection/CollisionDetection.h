#pragma once
#include <Physics/Structures.h>
#include <Physics/Colliders/Collider.h>
#include <array>
#include <functional>

class AABBCollider;
class SphereCollider;
class ConvexHullCollider;
class CylinderCollider;

class CollisionDetection
{
	//Collision Detection Functions
private:
	typedef bool (*CollisionFunction)(const Collider*, const Collider*, CollisionManifold* manifold);
	static CollisionFunction s_CollisionFunctionArray[COLLIDER_TYPE::COLLIDER_TYPE_COUNT][COLLIDER_TYPE::COLLIDER_TYPE_COUNT];

	static bool DispatchSeperatingAxisTheorem(const Collider* colliderA, const Collider* colliderB, CollisionManifold* manifold = nullptr);
	static bool DispatchGilbertJohnsonKeethri(const Collider* colliderA, const Collider* colliderB, CollisionManifold* manifold = nullptr);

	/* Sphere  vs Sphere  */ static bool SphereSphereCollision(const Collider* sphereColliderA, const Collider* sphereColliderB, CollisionManifold* manifold = nullptr);
		       			  
	/* AABB    vs Sphere  */ static bool AABBSphereCollision(const Collider* aabbCollider, const Collider* sphereCollider, CollisionManifold* manifold = nullptr);
	/* AABB    vs AABB    */ static bool AABBAABBCollision(const Collider* aabbColliderA, const Collider* aabbColliderB, CollisionManifold* manifold = nullptr);
		       			  
 	/* OBB     vs Sphere  */ static bool OBBSphereCollision(const Collider* obbCollider, const Collider* sphereCollider, CollisionManifold* manifold = nullptr);
	/* OBB     vs AABB    */ static bool OBBAABBCollision(const Collider* obbCollider, const Collider* aabbCollider, CollisionManifold* manifold = nullptr);
	/* OBB     vs OBB     */ static bool OBBOBBCollision(const Collider* obbColliderA, const Collider* obbColliderB, CollisionManifold* manifold = nullptr);
						  
	/* Cylinder vs Sphere  */ static bool CylinderSphereCollision(const Collider* capsuleCollider, const Collider* sphereCollider, CollisionManifold* manifold = nullptr);
	/* Cylinder vs AABB    */ static bool CylinderAABBCollision(const Collider* capsuleCollider, const Collider* aabbCollider, CollisionManifold* manifold = nullptr);
	/* Cylinder vs OBB     */ static bool CylinderOBBCollision(const Collider* capsuleCollider, const Collider* obbCollider, CollisionManifold* manifold = nullptr);
	/* Cylinder vs Cylinder */ static bool CylinderCylinderCollision(const Collider* capsuleColliderA, const Collider* capsuleColliderB, CollisionManifold* manifold = nullptr);
						  
 	/* Hull    vs Sphere  */ static bool ConvexHullSphereCollision(const Collider* convexHullCollider, const Collider* sphereCollider, CollisionManifold* manifold = nullptr);
	/* Hull    vs AABB	  */ static bool ConvexHullAABBCollision(const Collider* convexHullCollider, const Collider* aabbCollider, CollisionManifold* manifold = nullptr);
	/* Hull    vs OBB	  */ static bool ConvexHullOBBCollision(const Collider* convexHullCollider, const Collider* obbCollider, CollisionManifold* manifold = nullptr);
	/* Hull    vs Cylinder */ static bool ConvexHullCylinderCollision(const Collider* convexHullCollider, const Collider* capsuleCollider, CollisionManifold* manifold = nullptr);
	/* Hull    vs Hull	  */ static bool ConvexHullConvexHullCollision(const Collider* convexHullColliderA, const Collider* convexHullColliderB, CollisionManifold* manifold = nullptr);
	
public:

	static bool Use_GJK;
	static bool Use_Dispatch_Table;
	static bool CheckCollision(const Collider* colliderA, const Collider* colliderB, CollisionManifold* manifold = nullptr);
};

