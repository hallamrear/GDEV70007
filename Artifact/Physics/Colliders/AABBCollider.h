#pragma once
#include <Physics/Colliders/Collider.h>

class AABBCollider : public Collider
{
private:
	static Vector3 Points[8];

protected:
	Matrix4x4 GetTransformMatrix() const override;

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

	Vector3 GetSupportPoint(const Vector3& direction) const override;
	void GetPoints(std::vector<Vector3>& points) const override;

	glm::vec3 ClosestPointOnColliderToPoint(const glm::vec3& point) const;
	
	// Returns the squared distance between a point p and an AABB b
	float SqrDistanceToAABB(const glm::vec3& point) const;
};