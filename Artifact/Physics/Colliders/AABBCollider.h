#pragma once
#include <Physics/Colliders/Collider.h>

class AABBCollider : public Collider
{
private:

public:
	AABBCollider(const Entity& entity, const Vector3& halfWidth = Vector3(0.5f, 0.5f, 0.5f));
	AABBCollider(const Entity& entity, const Vector3& max, const Vector3& min);
	~AABBCollider();

	/// <summary>
	/// Sets the size of the box collider.
	/// </summary>
	/// <param name="size">A vector3 dictating the half width of the cube.</param>
	void SetSize(const Vector3& size);

	Vector3 GetMaxCornerWorldSpace() const;
	Vector3 GetMinCornerWorldSpace() const;
	Vector3 GetMaxCornerLocalSpace() const;
	Vector3 GetMinCornerLocalSpace() const;

	void Render(Renderer& renderer);
};