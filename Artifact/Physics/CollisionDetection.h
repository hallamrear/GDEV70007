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
	static bool SeperatingAxisTheorem(const Collider& colliderA, const Collider& colliderB);

	/*  Sphere vs Sphere */ static bool SphereSphereCollision(const SphereCollider& colliderA, const SphereCollider& colliderB);

	/*  AABB   vs Sphere */ static bool BoxSphereCollision(const AABBCollider& colliderA, const SphereCollider& colliderB);
	/*  AABB   vs AABB   */ static bool BoxBoxCollision(const AABBCollider& colliderA, const AABBCollider& colliderB);

	/*  Hull   vs Sphere */ static bool ConvexHullSphereCollision(const ConvexHullCollider& colliderA, const SphereCollider& colliderB);
	/*  Hull   vs AABB	 */ static bool ConvexHullBoxCollision(const ConvexHullCollider& colliderA, const AABBCollider& colliderB);
	/*  Hull   vs Hull	 */ static bool ConvexHullConvexHullCollision(const ConvexHullCollider& colliderA, const ConvexHullCollider& colliderB);

	/*  Mesh   vs Sphere */ static bool ComplexMeshSphereCollision(const ComplexMeshCollider& colliderA, const SphereCollider& colliderB);
	/*  Mesh   vs AABB   */ static bool ComplexMeshBoxCollision(const ComplexMeshCollider& colliderA, const AABBCollider& colliderB);
	/*  Mesh   vs Hull   */ static bool ComplexMeshConvexHullCollision(const ComplexMeshCollider& colliderA, const ConvexHullCollider& colliderB);
	/*  Mesh   vs Mesh   */ static bool ComplexMeshComplexMeshCollision(const ComplexMeshCollider& colliderA, const ComplexMeshCollider& colliderB);

public:

	static bool CheckCollision(const Collider& colliderA, const Collider& colliderB);
};

