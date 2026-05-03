#include "pch.h"
#include "Entity.h"
#include <Rendering/Renderer.h>
#include <Physics/Colliders/SphereCollider.h>
#include <Physics/Colliders/AABBCollider.h>

#include <Rendering/Geometry/Mesh.h>

Entity::Entity()
{
	m_Model = nullptr;
	m_ID = EntityID();
	CoCreateGuid(&m_ID);
	RPC_CSTR str;
	UuidToString(&m_ID, &str);
	m_IDString = std::string(reinterpret_cast<char*>(str));
	m_DisplayName = "Unnamed";
	m_WorldMatrix = IdentityMatrix;
	m_Scale = Vector3(1.0f, 1.0f, 1.0f);
	m_Translation = Vector3(0.0f, 0.0f, 0.0f);
	m_RotationEuler = Vector3(0.0f, 0.0f, 0.0f);
	m_IsAlive = true;
	m_Collider = nullptr;
	m_IsPendingDestroy = false;
}

Entity::~Entity()
{
	m_ID = EntityID();
	m_Model = nullptr;
	m_IDString = "";
	m_DisplayName = "";
	m_WorldMatrix = IdentityMatrix;
	m_Scale = Vector3(0.0f, 0.0f, 0.0f);
	m_Translation = Vector3(0.0f, 0.0f, 0.0f);
	m_RotationEuler = Vector3(0.0f, 0.0f, 0.0f);
	m_IsAlive = false;
	m_IsPendingDestroy = true;
}

const EntityID Entity::GetID() const
{
	return m_ID;
}

const std::string& Entity::GetIDString() const
{
	return m_IDString;
}

const std::string& Entity::GetDisplayName() const
{
	return m_DisplayName;
}

void Entity::SetDisplayName(const std::string& displayName)
{
	m_DisplayName = displayName;
}

Rigidbody& Entity::GetRigidbody()
{
	return m_Rigidbody;
}

Collider* Entity::GetCollider() const
{
	if (m_Collider != nullptr)
	{
		return m_Collider;
	}

	return nullptr;
}

void Entity::SetCollider(const COLLIDER_TYPE& colliderType)
{
	Vector3 size = Vector3(0.5f, 0.5f, 0.5f);

	if (GetCollider() != nullptr)
	{
		if (GetCollider()->GetType() == colliderType)
		{
			return;
		}

		size = GetCollider()->GetSize();

		delete m_Collider;
		m_Collider = nullptr;
	}

	switch (colliderType)
	{
	case COLLIDER_TYPE_SPHERE:
		m_Collider = new SphereCollider(*this, size.x);
		break;
	case COLLIDER_TYPE_AABB:
		m_Collider = new AABBCollider(*this, size);
		break;
	case COLLIDER_TYPE_COUNT:
	default:
		printf("Failed to create a new collider. SetCollider was passed an invalid collider type.\n");
		break;
	}
}

void Entity::SetColliderFromModel(const COLLIDER_TYPE& colliderType)
{
	if (GetModel() == nullptr)
	{
		printf("Failed to create a collider from the model. Model reference is not set or invalid.\n");
		return;
	}

	SetCollider(colliderType);

	if (GetCollider() != nullptr)
	{
		Vector3 newSize = Vector3(
			std::max(GetModel()->GetMeshes()[0]->GetMaxVertexLocalSpace().x, std::abs(GetModel()->GetMeshes()[0]->GetMinVertexLocalSpace().x)) * 2.0f,
			std::max(GetModel()->GetMeshes()[0]->GetMaxVertexLocalSpace().y, std::abs(GetModel()->GetMeshes()[0]->GetMinVertexLocalSpace().y)) * 2.0f,
			std::max(GetModel()->GetMeshes()[0]->GetMaxVertexLocalSpace().z, std::abs(GetModel()->GetMeshes()[0]->GetMinVertexLocalSpace().z)) * 2.0f
		);

		//Spheres need radii rather than extents.
		if (colliderType == COLLIDER_TYPE_SPHERE)
		{
			newSize.x *= 0.5f;
			newSize.y *= 0.5f;
			newSize.z *= 0.5f;
		}

		GetCollider()->SetSize(newSize);
	}
}

void Entity::RemoveCollider()
{
	if (m_Collider != nullptr)
	{
		delete m_Collider;
		m_Collider = nullptr;
	}
}

void Entity::SetPosition(const Vector3& translation)
{
	m_Translation = translation;
}

void Entity::Translate(const Vector3& translation)
{
	m_Translation.x += translation.x;
	m_Translation.y += translation.y;
	m_Translation.z += translation.z;
}

const Vector3& Entity::GetPosition() const
{
	return m_Translation;
}

void Entity::SetWorldMatrix(const Matrix4x4& worldMatrix)
{
	m_WorldMatrix = worldMatrix;
}

const Matrix4x4& Entity::GetWorldMatrix() const
{
	return m_WorldMatrix;
}

void Entity::UpdateWorldMatrix()
{
	DirectX::XMStoreFloat4x4(&m_WorldMatrix,
		DirectX::XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) *
		DirectX::XMMatrixRotationRollPitchYaw(m_RotationEuler.x, m_RotationEuler.y, m_RotationEuler.z) *
		DirectX::XMMatrixTranslation(m_Translation.x, m_Translation.y, m_Translation.z));
}

void Entity::SetModel(ModelRef& model)
{
	m_Model = model;
}

const ModelRef& Entity::GetModel() const
{
	return m_Model;
}

void Entity::Update(const float& deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);
}

void Entity::PostUpdate(const float& deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);
}

void Entity::Render(Renderer& renderer)
{
	if (m_Model != nullptr)
	{
		renderer.Render(m_Model, m_WorldMatrix);
	}

	if (m_Collider != nullptr)
	{
		m_Collider->Render(renderer);
	}
}

void Entity::SetAlive(const bool& state)
{
	m_IsAlive = state;
}

const bool Entity::IsDead()
{
	return (m_IsAlive == false);
}

void Entity::Kill()
{
	SetAlive(false);
}

const bool& Entity::IsPendingDestroy()
{
	return m_IsPendingDestroy;
}

void Entity::Destroy()
{
	m_IsPendingDestroy = true;
}
