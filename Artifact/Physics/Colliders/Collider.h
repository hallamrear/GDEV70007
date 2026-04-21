#pragma once
#include <Rendering/Geometry/Model.h>
#include <System/ServiceLocator.h>
#include <Rendering/Renderer.h>

class Entity;

class Collider
{
protected:
	enum COLLIDER_TYPE
	{
		COLLIDER_TYPE_SPHERE,
		COLLIDER_TYPE_AABB,
		COLLIDER_TYPE_MESH,
		COLLIDER_TYPE_COUNT
	};

private:
	COLLIDER_TYPE m_Type;
	const Entity& m_AttachedEntity;
	Matrix4x4 m_OffsetMatrix;
	ModelRef m_ColliderModel;

protected:
	Collider(const COLLIDER_TYPE& colliderType, const Entity& entity);
	virtual ~Collider() = 0;

public:
	Matrix4x4& GetOffsetMatrix();
	const Entity& GetAttachedEntity() const;

	void SetColliderModel(const COLLIDER_TYPE& colliderType);

	void Render(Renderer& renderer);
};

