#pragma once
#include <Rendering/Geometry/Model.h>
#include <System/ServiceLocator.h>
#include <Rendering/Renderer.h>
#include <glm/glm.hpp>

class Entity;

enum COLLIDER_TYPE : int
{
	COLLIDER_TYPE_SPHERE = 0,
	COLLIDER_TYPE_AABB = 1,
	COLLIDER_TYPE_OBB = 2,
	COLLIDER_TYPE_CAPSULE = 3,
	COLLIDER_TYPE_CONVEX_HULL = 4,
	COLLIDER_TYPE_COUNT = 5
};

static const std::string c_ColliderTypeNames[COLLIDER_TYPE::COLLIDER_TYPE_COUNT] =
{
	"Sphere Collider",
	"Axis Aligned Bounding Box",
	"Oriented Bounding Box",
	"Cylinder Collider",
	"Convex Hull Collider"
};

class Collider
{
public:

protected:
	COLLIDER_TYPE m_Type;
	const Entity& m_AttachedEntity;
	Matrix4x4 m_OffsetMatrix;
	ModelRef m_ColliderModel;
	static TextureRef m_ColliderTexture;

	void SetColliderModel(const COLLIDER_TYPE& colliderType);

	Collider* m_ChildCollider;

protected:
	Collider(const COLLIDER_TYPE& colliderType, const Entity& entity);
	Vector3 m_Size;
	virtual Matrix4x4 GetTransformMatrix() const = 0;
	Matrix4x4 GetInverseTransformMatrix() const;

public:
	virtual ~Collider() = 0;

	const Entity& GetAttachedEntity() const;

	virtual void SetSize(const Vector3& size) = 0;
	const Vector3& GetSize() const;

	const COLLIDER_TYPE& GetType() const;

	void AddChildCollider(Collider* collider);
	Collider* GetBottomCollider();
	Collider* GetChildCollider();

	virtual Vector3 GetSupportPoint(const Vector3& direction) const = 0;
	virtual glm::vec3 GetSupportPoint(const glm::vec3& direction) const;
	virtual void GetPoints(std::vector<Vector3>& points) const = 0;

	virtual void Render(Renderer& renderer);
};

