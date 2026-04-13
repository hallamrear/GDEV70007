#pragma once

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

protected:
	Collider(const COLLIDER_TYPE& colliderType, const Entity& entity);
	virtual ~Collider() = 0;

public:
	Matrix4x4& GetOffsetMatrix();
	const Entity& GetAttachedEntity() const;

	virtual void Render() = 0;
};

