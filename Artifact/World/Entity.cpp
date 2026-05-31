#include "pch.h"
#include "Entity.h"
#include <Rendering/Renderer.h>
#include <Physics/Colliders/SphereCollider.h>
#include <Physics/Colliders/AABBCollider.h>
#include <Physics/Colliders/ConvexHullCollider.h>
#include <Rendering/Geometry/Mesh.h>
#include <Physics/Rigidbody.h>

Entity::Entity() : m_Rigidbody(nullptr)
{
	m_Model = nullptr;
	m_ID = EntityID();
	CoCreateGuid(&m_ID);
	RPC_CSTR str;
	UuidToString(&m_ID, &str);
	m_IDString = std::string(reinterpret_cast<char*>(str));
	m_DisplayName = "Unnamed";
	m_WorldMatrix = IdentityMatrix;
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
	assert(m_Rigidbody);
	return *m_Rigidbody;
}

const Rigidbody& Entity::GetRigidbody() const
{
	assert(m_Rigidbody);
	return *m_Rigidbody;
}

void Entity::SetRigidbody(Rigidbody& rigidbody)
{
	m_Rigidbody = &rigidbody;
}

Collider* Entity::GetCollider() const
{
	if (m_Collider != nullptr)
	{
		return m_Collider;
	}

	return nullptr;
}

void Entity::AddCollider(const COLLIDER_TYPE& colliderType)
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
	case COLLIDER_TYPE_CONVEX_HULL:
	{
		if (m_Model != nullptr)
		{
			//m_Collider = new ConvexHullCollider(*this, m_Model);
		}
	}

	case COLLIDER_TYPE_COUNT:
	default:
		printf("Failed to create a new collider. SetCollider was passed an invalid collider type.\n");
		break;
	}
}

void Entity::AddColliderFromModel(const COLLIDER_TYPE& colliderType)
{
	if (GetModel() == nullptr)
	{
		printf("Failed to create a collider from the model. Model reference is not set or invalid.\n");
		return;
	}

	AddCollider(colliderType);

	if (GetCollider() != nullptr)
	{
		Vector3 newSize = Vector3(
			std::max(GetModel()->GetMeshes()[0]->GetMaxVertexLocalSpace().x, std::abs(GetModel()->GetMeshes()[0]->GetMinVertexLocalSpace().x)) * 2.0f,
			std::max(GetModel()->GetMeshes()[0]->GetMaxVertexLocalSpace().y, std::abs(GetModel()->GetMeshes()[0]->GetMinVertexLocalSpace().y)) * 2.0f,
			std::max(GetModel()->GetMeshes()[0]->GetMaxVertexLocalSpace().z, std::abs(GetModel()->GetMeshes()[0]->GetMinVertexLocalSpace().z)) * 2.0f
		);

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
	m_Rigidbody->Translation = translation;	
	UpdateWorldMatrix();
}

void Entity::Translate(const Vector3& translation)
{
	m_Rigidbody->Translation.x += translation.x;
	m_Rigidbody->Translation.y += translation.y;
	m_Rigidbody->Translation.z += translation.z;
	UpdateWorldMatrix();
}

void Entity::Rotate(const Vector3& rotation)
{
	DirectX::XMStoreFloat4(&m_Rigidbody->Rotation,
		DirectX::XMQuaternionMultiply(XMLoadFloat4(&m_Rigidbody->Rotation),
		DirectX::XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)));

	DirectX::XMStoreFloat4(&m_Rigidbody->Rotation, DirectX::XMQuaternionNormalize(XMLoadFloat4(&m_Rigidbody->Rotation)));

	UpdateWorldMatrix();
}

const Vector3& Entity::GetPosition() const
{
	return m_Rigidbody->Translation;
}

void Entity::SetWorldMatrix(const Matrix4x4& worldMatrix)
{
	m_WorldMatrix = worldMatrix;
	UpdateWorldMatrix();
}

const Matrix4x4& Entity::GetWorldMatrix() const
{
	return m_WorldMatrix;
}

void Entity::UpdateWorldMatrix()
{
	DirectX::XMStoreFloat4x4(&m_RotationMatrix,
		DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&m_Rigidbody->Rotation)));

	DirectX::XMStoreFloat4x4(&m_WorldMatrix,
		DirectX::XMLoadFloat4x4(&m_RotationMatrix) *
		DirectX::XMMatrixTranslation(m_Rigidbody->Translation.x, m_Rigidbody->Translation.y, m_Rigidbody->Translation.z));

	DirectX::XMStoreFloat3(&m_RightVector, DirectX::XMVector3Normalize(DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&BASIS_RIGHT_VECTOR), DirectX::XMLoadFloat4x4(&m_RotationMatrix))));
	DirectX::XMStoreFloat3(&m_UpVector, DirectX::XMVector3Normalize(DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&BASIS_UP_VECTOR), DirectX::XMLoadFloat4x4(&m_RotationMatrix))));
	DirectX::XMStoreFloat3(&m_ForwardVector, DirectX::XMVector3Normalize(DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&BASIS_FORWARD_VECTOR), DirectX::XMLoadFloat4x4(&m_RotationMatrix))));
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

	if (m_IsPendingDestroy)
	{
		return;
	}

	if (m_DisplayName == "Test Room")
	{
		return;
	}

	//Rotate({ deltaTime * (rand() % 30), deltaTime * (rand() % 30), deltaTime * (rand() % 30) });
	//Rotate({ 0.0f, deltaTime * (rand() % 30), 0.0f });
}

void Entity::FixedUpdate()
{
	if (m_IsPendingDestroy)
	{
		return;
	}

	UpdateWorldMatrix();
}

void Entity::Render(Renderer& renderer)
{
	renderer.SetDefaultDrawMode();
	renderer.SetDebugDrawMode();

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

const Vector3& Entity::GetForwardVector() const
{
	return m_ForwardVector;
}

const Vector3& Entity::GetRightVector() const
{
	return m_RightVector;
}

const Vector3& Entity::GetUpVector() const
{
	return m_UpVector;
}