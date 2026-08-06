#pragma once
#include "Collider.h"

class CylinderCollider : public Collider
{
private:
    Matrix4x4 GetTransformMatrix() const override;

public:
    CylinderCollider(const Entity& entity, const float& radius, const float& height);
    ~CylinderCollider();

    void SetSize(const Vector3& radius_height_radius);
    Vector3 GetSupportPoint(const Vector3& direction) const;
    void GetPoints(std::vector<Vector3>& points) const;
};