#pragma once

class Entity;

class Collider
{
private:
	const Entity& m_AttachedEntity;
	Matrix4x4 m_OffsetMatrix;

protected:
	Collider(const Entity& entity);
	virtual ~Collider() = 0;

public:
	Matrix4x4& GetOffsetMatrix();
	const Entity& GetAttachedEntity() const;

	virtual void Render() = 0;
};

