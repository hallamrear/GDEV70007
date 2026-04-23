#pragma once
#include <Physics/Rigidbody.h>
#include <Rendering/Geometry/Model.h>

class Collider;
enum COLLIDER_TYPE : int;

class Entity
{
private:
	bool m_IsAlive;
	bool m_IsPendingDestroy;
	std::string m_DisplayName;
	EntityID m_ID;
	std::string m_IDString;
	Rigidbody m_Rigidbody;
	ModelRef m_Model;
	Matrix4x4 m_WorldMatrix;

	Vector3 m_Scale;
	Vector3 m_Translation;
	Vector3 m_RotationEuler;

	Collider* m_Collider;

	void UpdateWorldMatrix();

public:
	Entity();
	~Entity();

	const EntityID GetID() const;
	const std::string& GetIDString() const;
	const std::string& GetDisplayName() const;
	void SetDisplayName(const std::string& displayName);
	Rigidbody& GetRigidbody();

	Collider* GetCollider() const;
	void SetCollider(const COLLIDER_TYPE& colliderType);
	void SetColliderFromModel(const COLLIDER_TYPE& colliderType);

	void SetPosition(const Vector3& position);
	void Translate(const Vector3& translation);
	const Vector3& GetPosition() const;

	void SetWorldMatrix(const Matrix4x4& worldMatrix);
	const Matrix4x4& GetWorldMatrix() const;

	void SetModel(ModelRef& model);
	const ModelRef& GetModel() const;

	void Update(const float& deltaTime);
	void PostUpdate(const float& deltaTime);
	void Render(Renderer& renderer);

	void SetAlive(const bool& state);
	const bool IsDead();
	void Kill();

	void Destroy();
	const bool& IsPendingDestroy();
};