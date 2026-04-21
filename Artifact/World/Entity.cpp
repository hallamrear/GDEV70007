#include "pch.h"
#include "Entity.h"
#include <Rendering/Renderer.h>
#include <Physics/Colliders/SphereCollider.h>

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
	m_Collider = new SphereCollider(*this, 0.5f);
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