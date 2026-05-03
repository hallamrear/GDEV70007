#pragma once
#include <Physics/Colliders/Collider.h>
#include <array>
#include <functional>

class AABBCollider;
class SphereCollider;
class ConvexHullCollider;
class ComplexMeshCollider;

struct ContactPoint
{
	const Collider& Subject;
	const Vector3 HitPoint;
	const Vector3 Normal;
	const float Depth;

	ContactPoint(const Collider& subject, const Vector3 hitPoint, const Vector3 contactNormal, const float penetration) :
		Subject(subject), 
		HitPoint(hitPoint),
		Normal(contactNormal),
		Depth(penetration)
	{

	}
};

struct CollisionManifold
{
	std::vector<ContactPoint> ContactPoints;

	CollisionManifold()
	{
		ContactPoints = std::vector<ContactPoint>();
	}

	~CollisionManifold()
	{
		Reset();
	}

	void Reset()
	{
		ContactPoints.clear();
	}
};

class CollisionDetection
{
	//Collision Detection Functions
private:
	typedef bool (*CollisionFunction)(const Collider*, const Collider*, const CollisionManifold* manifold);
	static CollisionFunction s_CollisionFunctionArray[COLLIDER_TYPE::COLLIDER_TYPE_COUNT][COLLIDER_TYPE::COLLIDER_TYPE_COUNT];

	static bool SeperatingAxisTheorem(const Collider* colliderA, const Collider* colliderB);

	/*  Sphere vs Sphere */ static bool SphereSphereCollision(const Collider* sphereColliderA, const Collider* sphereColliderB);

	/*  AABB   vs Sphere */ static bool BoxSphereCollision(const Collider* boxCollider, const Collider* sphereCollider);
	/*  AABB   vs AABB   */ static bool BoxBoxCollision(const Collider* boxColliderA, const Collider* boxColliderB);

	/*  Hull   vs Sphere */ static bool ConvexHullSphereCollision(const Collider* convexHullCollider, const Collider* sphereCollider);
	/*  Hull   vs AABB	 */ static bool ConvexHullBoxCollision(const Collider* convexHullCollider, const Collider* boxCollider);
	/*  Hull   vs Hull	 */ static bool ConvexHullConvexHullCollision(const Collider* convexHullColliderA, const Collider* convexHullColliderB);

	/*  Mesh   vs Sphere */ static bool ComplexMeshSphereCollision(const Collider* complexMeshCollider, const Collider* sphereCollider);
	/*  Mesh   vs AABB   */ static bool ComplexMeshBoxCollision(const Collider* complexMeshCollider, const Collider* boxCollider);
	/*  Mesh   vs Hull   */ static bool ComplexMeshConvexHullCollision(const Collider* complexMeshCollider, const Collider* convexHullCollider);
	/*  Mesh   vs Mesh   */ static bool ComplexMeshComplexMeshCollision(const Collider* complexMeshColliderA, const Collider* complexMeshColliderB);

public:

	static bool CheckCollision(const Collider* colliderA, const Collider* colliderB, const CollisionManifold* manifold = nullptr);
};

