#include "pch.h"
#include "Entity.h"
#include <Rendering/Renderer.h>
#include <Physics/Colliders/SphereCollider.h>
#include <Physics/Colliders/AABBCollider.h>
#include <Physics/Colliders/OBBCollider.h>
#include <Physics/Colliders/CylinderCollider.h>
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
	m_RotationMatrix = IdentityMatrix;
	m_IsAlive = true;
	m_Collider = nullptr;
	m_IsPendingDestroy = false;

	m_RotationDir = Vector3(
		(float)(rand() % 100) / 100.0f,
		(float)(rand() % 100) / 100.0f, 
		(float)(rand() % 100) / 100.0f);

	m_ForwardVector = Vector3(0.0f, 0.0f, 0.0f);
	m_RightVector = Vector3(0.0f, 0.0f, 0.0f);
	m_UpVector = Vector3(0.0f, 0.0f, 0.0f);
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
	m_Rigidbody->SetEntity(this);
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
	case COLLIDER_TYPE_OBB:
		m_Collider = new OBBCollider(*this, size);
		break;
	case COLLIDER_TYPE_CAPSULE:
		m_Collider = new CylinderCollider(*this, 0.5f, 2.0f);
		break;
	case COLLIDER_TYPE_CONVEX_HULL:
	{
		if (m_Model != nullptr)
		{
			m_Collider = new ConvexHullCollider(*this, m_Model);
		}
	}
	break;

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
		Vector3 newSize = { 0.0f, 0.0f, 0.0f };

		switch (colliderType)
		{
		case COLLIDER_TYPE_SPHERE:
		{
			glm::vec3 max = { GetModel()->GetMeshes()[0]->GetMaxVertexLocalSpace().x ,GetModel()->GetMeshes()[0]->GetMaxVertexLocalSpace().y, GetModel()->GetMeshes()[0]->GetMaxVertexLocalSpace().z };
			glm::vec3 min = { GetModel()->GetMeshes()[0]->GetMinVertexLocalSpace().x, GetModel()->GetMeshes()[0]->GetMinVertexLocalSpace().y, GetModel()->GetMeshes()[0]->GetMinVertexLocalSpace().z };

			glm::vec3 furthest = (glm::length2(max) > glm::length2(min)) ? max : min;

			float radius = glm::dot(furthest, furthest);
			radius = sqrtf(radius);
			newSize = { radius, radius, radius };
		}
		break;

		case COLLIDER_TYPE_AABB:
		case COLLIDER_TYPE_OBB:
		{
			newSize = Vector3(
				std::max(GetModel()->GetMeshes()[0]->GetMaxVertexLocalSpace().x, std::abs(GetModel()->GetMeshes()[0]->GetMinVertexLocalSpace().x)) * 2.0f,
				std::max(GetModel()->GetMeshes()[0]->GetMaxVertexLocalSpace().y, std::abs(GetModel()->GetMeshes()[0]->GetMinVertexLocalSpace().y)) * 2.0f,
				std::max(GetModel()->GetMeshes()[0]->GetMaxVertexLocalSpace().z, std::abs(GetModel()->GetMeshes()[0]->GetMinVertexLocalSpace().z)) * 2.0f
			);
		}
			break;

		case COLLIDER_TYPE_CAPSULE:
		{
			glm::vec3 size;
			size.x = fabsf(GetModel()->GetMeshes()[0]->GetMaxVertexLocalSpace().x - GetModel()->GetMeshes()[0]->GetMinVertexLocalSpace().x);
			size.y = fabsf(GetModel()->GetMeshes()[0]->GetMaxVertexLocalSpace().y - GetModel()->GetMeshes()[0]->GetMinVertexLocalSpace().y);
			size.z = fabsf(GetModel()->GetMeshes()[0]->GetMaxVertexLocalSpace().z - GetModel()->GetMeshes()[0]->GetMinVertexLocalSpace().z);

			float r = std::max(size.x, size.z);
			float h = size.y * 2.0f;
			newSize = { r, h, r };			
		}
			break;

		case COLLIDER_TYPE_CONVEX_HULL:
		case COLLIDER_TYPE_COUNT:
		default:
			return;
			break;
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
	m_Rigidbody->Translation.x = translation.x;
	m_Rigidbody->Translation.y = translation.y;
	m_Rigidbody->Translation.z = translation.z;
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
	glm::vec3 angles = { rotation.x, rotation.y, rotation.z };
	m_Rigidbody->Rotation *= glm::toQuat(glm::orientate3(angles));
	m_Rigidbody->Rotation = glm::normalize(m_Rigidbody->Rotation);
	UpdateWorldMatrix();
}

const Vector3 Entity::GetPosition() const
{
	return { m_Rigidbody->Translation.x, m_Rigidbody->Translation.y, m_Rigidbody->Translation.z };
}

const Vector4 Entity::GetRotation() const
{
	return { m_Rigidbody->Rotation.x, m_Rigidbody->Rotation.y, m_Rigidbody->Rotation.z, m_Rigidbody->Rotation.w };
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

const void Entity::GetRotationMatrix(Matrix4x4& matrix) const
{
	matrix = m_RotationMatrix;
}

const void Entity::GetRotationMatrix(Matrix3x3& matrix) const
{
	DirectX::XMStoreFloat3x3(&matrix, DirectX::XMLoadFloat4x4(&m_RotationMatrix));
}

const void Entity::GetInverseRotationMatrix(Matrix4x4& matrix) const
{
	DirectX::XMStoreFloat4x4(&matrix, DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&m_RotationMatrix)));
}

void Entity::UpdateWorldMatrix()
{
	DirectX::XMFLOAT4 rotation = { m_Rigidbody->Rotation.x, m_Rigidbody->Rotation.y, m_Rigidbody->Rotation.z, m_Rigidbody->Rotation.w };

	DirectX::XMStoreFloat4x4(&m_RotationMatrix,
		DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation)));

	DirectX::XMStoreFloat4x4(&m_WorldMatrix,
		DirectX::XMLoadFloat4x4(&m_RotationMatrix) *
		DirectX::XMMatrixTranslation(m_Rigidbody->Translation.x, m_Rigidbody->Translation.y, m_Rigidbody->Translation.z));

	DirectX::XMStoreFloat3(&m_RightVector, DirectX::XMVector3Normalize(DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&BASIS_RIGHT_VECTOR), DirectX::XMLoadFloat4x4(&m_RotationMatrix))));
	DirectX::XMStoreFloat3(&m_UpVector, DirectX::XMVector3Normalize(DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&BASIS_UP_VECTOR), DirectX::XMLoadFloat4x4(&m_RotationMatrix))));
	DirectX::XMStoreFloat3(&m_ForwardVector, DirectX::XMVector3Normalize(DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&BASIS_FORWARD_VECTOR), DirectX::XMLoadFloat4x4(&m_RotationMatrix))));

	if(m_Collider != nullptr)
	{
		m_Collider->UpdateBoundingVolume();
	}
}

void Entity::SetModel(ModelRef& model)
{
	m_Model = model;
}

const bool Entity::HasModel() const
{
	return (m_Model != nullptr);
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

	static float rotSpeed = 1.0f;
	Rotate({ deltaTime * m_RotationDir.x * rotSpeed, deltaTime * m_RotationDir.y * rotSpeed, deltaTime * m_RotationDir.z * rotSpeed });
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