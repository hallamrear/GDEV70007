#include "pch.h"
#include "Collider.h"
#include <World/Entity.h>
#include <System/AssetManagement.h>

TextureRef Collider::m_ColliderTexture = nullptr;

Collider::Collider(const COLLIDER_TYPE& colliderType, const Entity& entity) : m_AttachedEntity(entity), m_Type(colliderType)
{
	m_OffsetMatrix = IdentityMatrix;
	SetColliderModel(colliderType);
}

Collider::~Collider()
{
	m_OffsetMatrix = Matrix4x4();
	
	if (m_ColliderTexture.use_count() == 1)
	{
		m_ColliderTexture = nullptr;
	}
}

Matrix4x4& Collider::GetOffsetMatrix()
{
	return m_OffsetMatrix;
}

const Entity& Collider::GetAttachedEntity() const
{
	return m_AttachedEntity;
}

void Collider::SetColliderModel(const COLLIDER_TYPE& colliderType)
{
	AssetManager* assetManager = ServiceLocator::Locate<AssetManager>();

	if (assetManager == nullptr)
	{
		throw std::exception("Cannot source asset manager when setting collider models.\n");
		return;
	}

	m_ColliderTexture = assetManager->GetTexture("Content\\ColliderTexture.png");

	switch (colliderType)
	{
	case COLLIDER_TYPE_AABB:
	{
		m_ColliderModel = assetManager->GetModel("Content\\Colliders\\BoxCollider.glb");
	}
	break;

	case COLLIDER_TYPE_SPHERE:
	{
		m_ColliderModel = assetManager->GetModel("Content\\Colliders\\SphereCollider.glb");
	}
	break;

	case COLLIDER_TYPE_MESH:
	{
		m_ColliderModel = m_AttachedEntity.GetModel();
	}
	break;

	default:
		throw std::exception("Invalid collider type.\n");
		break;
	}

	if (m_ColliderModel == nullptr)
	{
		printf("Failed to set collider type to the required mesh.\n");
		throw std::exception("Invalid collider type.\n");
		return;
	}

	m_Type = colliderType;
}

const Vector3& Collider::GetSize() const
{
	return m_Size;
}

const COLLIDER_TYPE& Collider::GetType() const
{
	return m_Type;
}

void Collider::Render(Renderer& renderer)
{
	if (m_ColliderModel == nullptr)
	{
		printf("Trying to draw a collider that doesn't have a model.\n");
		throw;
	}

	renderer.SetDebugDrawMode();
	Matrix4x4 worldMatrix = IdentityMatrix;
	DirectX::XMStoreFloat4x4(&worldMatrix, 
		DirectX::XMMatrixScaling(m_Size.x, m_Size.y, m_Size.z) *
		DirectX::XMLoadFloat4x4(&m_AttachedEntity.GetWorldMatrix()) * 
		DirectX::XMLoadFloat4x4(&m_OffsetMatrix));
	renderer.Render(m_ColliderModel, worldMatrix);
	renderer.SetDefaultDrawMode();
}