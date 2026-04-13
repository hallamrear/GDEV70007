#include "pch.h"
#include "Collider.h"

Collider::Collider(const COLLIDER_TYPE& colliderType, const Entity& entity) : m_AttachedEntity(entity), m_Type(colliderType)
{
	m_OffsetMatrix = Matrix4x4();
}

Collider::~Collider()
{
	m_OffsetMatrix = Matrix4x4();
}

Matrix4x4& Collider::GetOffsetMatrix()
{
	return m_OffsetMatrix;
}

const Entity& Collider::GetAttachedEntity() const
{
	return m_AttachedEntity;
}

void Collider::Render()
{

}