#pragma once
#include <Physics/Colliders/Collider.h>

class SphereCollider : public Collider
{
private:

public:
	SphereCollider(const Entity& entity, const float& radius);
	SphereCollider(const Entity& entity, const Vector3& size);
	~SphereCollider();

	/// <summary>
	/// Sets the radius of the sphere collider to the largest of the component absolutes of the vector.
	/// </summary>
	/// <param name="size">A vector3, of which the largest (absolute) component will be set as the radius</param>
	void SetSize(const Vector3& size);
};

