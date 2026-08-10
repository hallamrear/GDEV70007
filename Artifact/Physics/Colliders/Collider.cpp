#include "pch.h"
#include "Collider.h"
#include <World/Entity.h>
#include <System/AssetManagement.h>
#include <Rendering/Geometry/Mesh.h>

TextureRef Collider::m_NarrowPhaseTexture = nullptr;
TextureRef Collider::m_BroadPhaseTexture = nullptr;

#ifdef _DEBUG
COLLIDER_DRAW_LEVEL Collider::g_DrawColliders = COLLIDER_DRAW_LEVEL::COLLIDER_DRAW_ALL;
#else
COLLIDER_DRAW_LEVEL Collider::g_DrawColliders = COLLIDER_DRAW_LEVEL::COLLIDER_DRAW_NONE;
#endif

Collider::Collider(const COLLIDER_TYPE& colliderType, const Entity& entity) : m_AttachedEntity(entity), m_Type(colliderType)
{
	m_OffsetMatrix = IdentityMatrix;
	m_BoundingVolumeType = COLLIDER_TYPE_AABB;
	SetColliderModel(colliderType);
}

Matrix4x4 Collider::GetInverseTransformMatrix() const
{
	Matrix4x4 m = GetTransformMatrix();
	Matrix4x4 inv;
	DirectX::XMStoreFloat4x4(&inv, DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&m)));
	return inv;
}

Collider::~Collider()
{
	m_OffsetMatrix = Matrix4x4();
	
	if (m_NarrowPhaseTexture.use_count() == 1)
	{
		m_NarrowPhaseTexture = nullptr;
	}
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

	m_BoundingVolumeCollider = assetManager->GetModel("Colliders\\BoxCollider.glb");

	switch (colliderType)
	{
	case COLLIDER_TYPE_OBB:
	case COLLIDER_TYPE_AABB: 
		m_ColliderModel = assetManager->GetModel("Colliders\\BoxCollider.glb"); 
		break;

	case COLLIDER_TYPE_SPHERE: m_ColliderModel = assetManager->GetModel("Colliders\\SphereCollider.glb"); break;

	case COLLIDER_TYPE_CONVEX_HULL:
		m_ColliderModel = m_AttachedEntity.GetModel(); 
		break;

	case COLLIDER_TYPE_CAPSULE: m_ColliderModel = assetManager->GetModel("Colliders\\CylinderCollider.glb"); break;

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

	if (m_NarrowPhaseTexture == nullptr)
	{
		m_NarrowPhaseTexture = assetManager->GetTexture("NarrowPhaseCollider");
	}

	if (m_BroadPhaseTexture == nullptr)
	{
		m_BroadPhaseTexture = assetManager->GetTexture("BroadPhaseCollider");
	}

	m_ColliderModel->GetMeshes()[0]->GetTextures().resize(3);

	if (m_BroadPhaseTexture)
	{
		m_ColliderModel->GetMeshes()[0]->GetTextures()[1] = m_BroadPhaseTexture;
	}
	
	if (m_NarrowPhaseTexture)
	{
		m_ColliderModel->GetMeshes()[0]->GetTextures()[2] = m_NarrowPhaseTexture;
	}
}

const Vector3& Collider::GetSize() const
{
	return m_Size;
}

const COLLIDER_TYPE& Collider::GetType() const
{
	return m_Type;
}

const Vector3& Collider::GetBoundingVolumeExtents() const
{
	return m_BoundingVolumeHalfExtents;
}

Vector3 Collider::GetSupportPoint(const Vector3& direction) const
{
	UNREFERENCED_PARAMETER(direction);
	assert(true);
	return { FLT_MAX, FLT_MAX, FLT_MAX };
}

glm::vec3 Collider::GetSupportPoint(const glm::vec3& direction) const
{
	Vector3 furthest = GetSupportPoint(Vector3(direction.x, direction.y, direction.z));
	return { furthest.x, furthest.y, furthest.z };
}

void Collider::Render(Renderer& renderer)
{
	switch (g_DrawColliders)
	{

	case COLLIDER_DRAW_ALL:
	{
		RenderBroadBoundingVolume(renderer);
		__fallthrough;
	}
	case COLLIDER_DRAW_CONVEX_ONLY:
	{
		RenderCollisionModel(renderer);
	}
		break;

	case COLLIDER_DRAW_NONE:
	case COLLIDER_DRAW_LEVEL_COUNT:
	default:
		break;	
	}
}

void Collider::RenderCollisionModel(Renderer& renderer)
{
	if (m_ColliderModel == nullptr)
	{
		printf("Trying to draw a collider that doesn't have a model.\n");
		throw;
	}

	renderer.SetDebugDrawMode();
	renderer.GetPushConstants().Padding.m[2][2] = 1.0f;
	renderer.Render(m_ColliderModel, GetTransformMatrix());
	renderer.SetDefaultDrawMode();
}

void Collider::UpdateBoundingVolume()
{
	switch (m_Type)
	{

	case COLLIDER_TYPE_SPHERE:
	{
		m_BoundingVolumeHalfExtents.x = m_Size.x * 2.0f;
		m_BoundingVolumeHalfExtents.y = m_Size.y * 2.0f;
		m_BoundingVolumeHalfExtents.z = m_Size.z * 2.0f;
	}
		break;

	case COLLIDER_TYPE_AABB:
	case COLLIDER_TYPE_OBB:
	case COLLIDER_TYPE_CAPSULE:
	case COLLIDER_TYPE_CONVEX_HULL:
	{
		Matrix3x3 rotMatrix;
		m_AttachedEntity.GetRotationMatrix(rotMatrix);

		for (size_t i = 0; i < 9; i++)
		{
			rotMatrix.m[i % 3][i / 3] = fabsf(rotMatrix.m[i % 3][i / 3]);
		}

		m_BoundingVolumeHalfExtents.x = rotMatrix.m[0][0] * m_Size.x + rotMatrix.m[0][1] * m_Size.y + rotMatrix.m[0][2] * m_Size.z;
		m_BoundingVolumeHalfExtents.y = rotMatrix.m[1][0] * m_Size.x + rotMatrix.m[1][1] * m_Size.y + rotMatrix.m[1][2] * m_Size.z;
		m_BoundingVolumeHalfExtents.z = rotMatrix.m[2][0] * m_Size.x + rotMatrix.m[2][1] * m_Size.y + rotMatrix.m[2][2] * m_Size.z;

		m_BoundingVolumeHalfExtents.x = rotMatrix.m[0][0] * m_Size.x + rotMatrix.m[1][0] * m_Size.y + rotMatrix.m[2][0] * m_Size.z;
		m_BoundingVolumeHalfExtents.y = rotMatrix.m[0][1] * m_Size.x + rotMatrix.m[1][1] * m_Size.y + rotMatrix.m[2][1] * m_Size.z;
		m_BoundingVolumeHalfExtents.z = rotMatrix.m[0][2] * m_Size.x + rotMatrix.m[1][2] * m_Size.y + rotMatrix.m[2][2] * m_Size.z;
	}
		break;

	case COLLIDER_TYPE_COUNT:
		break;
	default:
		break;
	}
}

void Collider::RenderBroadBoundingVolume(Renderer& renderer)
{
	if (m_Type == m_BoundingVolumeType)
		return;

	if (m_BoundingVolumeCollider == nullptr)
	{
		printf("Trying to draw a collider that doesn't have a model.\n");
		throw;
	}

	Matrix4x4 matrix;
	DirectX::XMStoreFloat4x4(&matrix,
		DirectX::XMMatrixScaling(m_BoundingVolumeHalfExtents.x, m_BoundingVolumeHalfExtents.y, m_BoundingVolumeHalfExtents.z) *
		DirectX::XMMatrixTranslation(m_AttachedEntity.GetPosition().x, m_AttachedEntity.GetPosition().y, m_AttachedEntity.GetPosition().z) *
		DirectX::XMLoadFloat4x4(&m_OffsetMatrix));

	renderer.SetDebugDrawMode();
	renderer.GetPushConstants().Padding.m[2][2] = 0.0f;
	renderer.Render(m_BoundingVolumeCollider, matrix);
	renderer.SetDefaultDrawMode();
}