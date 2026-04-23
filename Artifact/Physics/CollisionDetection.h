#pragma once
#include <Physics/Colliders/Collider.h>
#include <array>
#include <functional>

class BoxCollider;
class SphereCollider;

class CollisionDetection
{
	//Collision Detection Functions
private:
	static bool BoxBoxCollision(const BoxCollider& colliderA, const BoxCollider& colliderB);

protected:
	typedef bool ;
	typedef std::array<std::array<std::function<bool(const Collider&, const Collider&)>, COLLIDER_TYPE_COUNT>, COLLIDER_TYPE_COUNT> CollisionFunctionArray;

	static const CollisionFunctionArray c_CollisionFunctionArray;

public:
	static bool CheckCollision(const Collider& colliderA, const Collider& colliderB);
};

