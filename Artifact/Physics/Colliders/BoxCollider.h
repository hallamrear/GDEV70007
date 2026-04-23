#pragma once
#include <Physics/Colliders/Collider.h>

class BoxCollider : public Collider
{
private:

public:
	BoxCollider(const Entity& entity, const Vector3& halfWidth = Vector3(0.5f, 0.5f, 0.5f));
	BoxCollider(const Entity& entity, const Vector3& max, const Vector3& min);
	~BoxCollider();

	/// <summary>
	/// Sets the size of the box collider.
	/// </summary>
	/// <param name="size">A vector3 dictating the half width of the cube.</param>
	void SetSize(const Vector3& size);
};