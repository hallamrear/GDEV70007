#include "pch.h"
#include "Collider.h"

Collider::Collider(const Entity& entity) : m_AttachedEntity(entity)
{

}

Collider::~Collider()
{

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