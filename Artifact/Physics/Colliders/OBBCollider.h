#pragma once
#include <Physics/Colliders/Collider.h>

class OBBCollider : public Collider
{
private:
	static Vector3 Points[8];

protected:
	Matrix4x4 GetTransformMatrix() const override;

public:
	OBBCollider(const Entity& entity, const Vector3& halfWidth = Vector3(0.5f, 0.5f, 0.5f));
	OBBCollider(const Entity& entity, const Vector3& max, const Vector3& min);
	~OBBCollider();

	/// <summary>
	/// Sets the size of the box collider.
	/// </summary>
	/// <param name="size">A vector3 dictating the half width of the cube.</param>
	void SetSize(const Vector3& size);

	Vector3 GetMaxCornerWorldSpace() const;
	Vector3 GetMinCornerWorldSpace() const;
	Vector3 GetMaxCornerLocalSpace() const;
	Vector3 GetMinCornerLocalSpace() const;

	void GetPoints(std::vector<Vector3>& points) const override;
	Vector3 GetFurthestPointInDirection(const Vector3& direction) const override;
};