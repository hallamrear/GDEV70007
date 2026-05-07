#pragma once
#include <Physics/Colliders/Collider.h>
#include <array>
#include <functional>

class AABBCollider;
class SphereCollider;
class ConvexHullCollider;
class ComplexMeshCollider;

struct Contact
{
	const Collider& SubjectA;
	const Collider& SubjectB;
	/// <summary>
	/// Hit point is always relative to the position of subject A.
	/// </summary>
	const Vector3 HitPoint;
	const Vector3 Normal;
	const float Depth;

	Contact(const Collider& subjectA, const Collider& subjectB, const Vector3 hitPoint, const Vector3 contactNormal, const float penetration) :
		SubjectA(subjectA),
		SubjectB(subjectB),
		HitPoint(hitPoint),
		Normal(contactNormal),
		Depth(penetration)
	{

	}
};

struct CollisionManifold
{
	std::vector<Contact> ContactPoints;

	CollisionManifold()
	{
		ContactPoints = std::vector<Contact>();
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
	typedef bool (*CollisionFunction)(const Collider*, const Collider*, CollisionManifold* manifold);
	static CollisionFunction s_CollisionFunctionArray[COLLIDER_TYPE::COLLIDER_TYPE_COUNT][COLLIDER_TYPE::COLLIDER_TYPE_COUNT];

	static bool SeperatingAxisTheorem(const Collider* colliderA, const Collider* colliderB, CollisionManifold* manifold = nullptr);

	/*  Sphere vs Sphere */ static bool SphereSphereCollision(const Collider* sphereColliderA, const Collider* sphereColliderB, CollisionManifold* manifold = nullptr);

	/*  AABB   vs Sphere */ static bool BoxSphereCollision(const Collider* boxCollider, const Collider* sphereCollider, CollisionManifold* manifold = nullptr);
	/*  AABB   vs AABB   */ static bool BoxBoxCollision(const Collider* boxColliderA, const Collider* boxColliderB, CollisionManifold* manifold = nullptr);

	/*  Hull   vs Sphere */ static bool ConvexHullSphereCollision(const Collider* convexHullCollider, const Collider* sphereCollider, CollisionManifold* manifold = nullptr);
	/*  Hull   vs AABB	 */ static bool ConvexHullBoxCollision(const Collider* convexHullCollider, const Collider* boxCollider, CollisionManifold* manifold = nullptr);
	/*  Hull   vs Hull	 */ static bool ConvexHullConvexHullCollision(const Collider* convexHullColliderA, const Collider* convexHullColliderB, CollisionManifold* manifold = nullptr);

	/*  Mesh   vs Sphere */ static bool ComplexMeshSphereCollision(const Collider* complexMeshCollider, const Collider* sphereCollider, CollisionManifold* manifold = nullptr);
	/*  Mesh   vs AABB   */ static bool ComplexMeshBoxCollision(const Collider* complexMeshCollider, const Collider* boxCollider, CollisionManifold* manifold = nullptr);
	/*  Mesh   vs Hull   */ static bool ComplexMeshConvexHullCollision(const Collider* complexMeshCollider, const Collider* convexHullCollider, CollisionManifold* manifold = nullptr);
	/*  Mesh   vs Mesh   */ static bool ComplexMeshComplexMeshCollision(const Collider* complexMeshColliderA, const Collider* complexMeshColliderB, CollisionManifold* manifold = nullptr);

public:

	static bool CheckCollision(const Collider* colliderA, const Collider* colliderB, CollisionManifold* manifold = nullptr);
};

