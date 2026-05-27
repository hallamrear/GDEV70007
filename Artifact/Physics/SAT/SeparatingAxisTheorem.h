#pragma once

class Collider;
struct CollisionManifold;

struct Interval
{
	float max = -INFINITY;
	float min = INFINITY;
	Vector3 axis = Vector3(0.0f, 0.0f, 0.0f);
	float delta = INFINITY;
};

class SeparatingAxisTheorem
{
private:
	static Interval GetInterval(const Collider& collider, const Vector3& axis);
	static bool OverlapOnAxis(const Collider& colliderA, const Collider& colliderB, const Vector3& testAxis);

public:
	static bool CheckCollision(const Collider& colliderA, const Collider& colliderB, CollisionManifold* manifold = nullptr);
};

