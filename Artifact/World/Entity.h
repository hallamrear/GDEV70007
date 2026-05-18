#pragma once
#include <Rendering/Geometry/Model.h>

class Collider;
enum COLLIDER_TYPE : int;
class Rigidbody;

class Entity
{
private:
	bool m_IsAlive;
	bool m_IsPendingDestroy;
	std::string m_DisplayName;
	EntityID m_ID;
	std::string m_IDString;
	Rigidbody* m_Rigidbody;
	ModelRef m_Model;
	Matrix4x4 m_WorldMatrix;
	Matrix4x4 m_RotationMatrix;
	Collider* m_Collider;

	Vector3 m_ForwardVector;
	Vector3 m_UpVector;
	Vector3 m_RightVector;

	void UpdateWorldMatrix();

public:
	Entity();
	~Entity();

	const EntityID GetID() const;
	const std::string& GetIDString() const;
	const std::string& GetDisplayName() const;
	void SetDisplayName(const std::string& displayName);
	Rigidbody& GetRigidbody();
	const Rigidbody& GetRigidbody() const;
	void SetRigidbody(Rigidbody& rigidbody);

	Collider* GetCollider() const;
	void AddCollider(const COLLIDER_TYPE& colliderType);
	void AddColliderFromModel(const COLLIDER_TYPE& colliderType);
	void RemoveCollider();

	void SetPosition(const Vector3& position);
	void Translate(const Vector3& translation);
	void Rotate(const Vector3& rotation);
	const Vector3& GetPosition() const;

	void SetWorldMatrix(const Matrix4x4& worldMatrix);
	const Matrix4x4& GetWorldMatrix() const;

	void SetModel(ModelRef& model);
	const ModelRef& GetModel() const;

	void FixedUpdate();
	void Update(const float& deltaTime);
	void Render(Renderer& renderer);

	void SetAlive(const bool& state);
	const bool IsDead();
	void Kill();

	void Destroy();
	const bool& IsPendingDestroy();

	const Vector3& GetRightVector() const;
	const Vector3& GetUpVector() const;
	const Vector3& GetForwardVector() const;
};