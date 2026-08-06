#pragma once
#include <Physics/Colliders/Collider.h>

class ConvexHull;

class ConvexHullCollider : public Collider
{
private:
	/// <summary>
	/// Sets the radius of the sphere collider to the largest of the component absolutes of the vector.
	/// </summary>
	/// <param name="size">A vector3, of which the largest (absolute) component will be set as the radius</param>
	void SetSize(const Vector3& size);
protected:
	Matrix4x4 GetTransformMatrix() const override;
	const ConvexHull* m_ConvexHull;

public:
	ConvexHullCollider(const Entity& entity, const ModelRef& modelRef);
	~ConvexHullCollider();

	Vector3 GetSupportPoint(const Vector3& direction) const override;
	void GetPoints(std::vector<Vector3>& points) const override;
};

