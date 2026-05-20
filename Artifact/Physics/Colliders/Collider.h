#pragma once
#include <Rendering/Geometry/Model.h>
#include <System/ServiceLocator.h>
#include <Rendering/Renderer.h>

class Entity;

enum COLLIDER_TYPE : int
{
	COLLIDER_TYPE_SPHERE = 0,
	COLLIDER_TYPE_AABB = 1,
	COLLIDER_TYPE_CONVEX_HULL = 2,
	COLLIDER_TYPE_MESH = 3,
	COLLIDER_TYPE_COUNT = 4
};

static const std::string c_ColliderTypeNames[COLLIDER_TYPE::COLLIDER_TYPE_COUNT] =
{
	"Sphere Collider",
	"Axis Aligned Bounding Box",
	"Convex Hull Collider",
	"Mesh Collider",
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

public:
	virtual ~Collider() = 0;

	const Entity& GetAttachedEntity() const;

	virtual void SetSize(const Vector3& size) = 0;
	const Vector3& GetSize() const;

	const COLLIDER_TYPE& GetType() const;

	void AddChildCollider(Collider* collider);
	Collider* GetBottomCollider();
	Collider* GetChildCollider();

	virtual Vector3 GetFurthestPointInDirection(const Vector3& direction) = 0;

	virtual void Render(Renderer& renderer);
};

