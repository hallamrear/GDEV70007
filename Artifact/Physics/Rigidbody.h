#pragma once

class Entity;

class Rigidbody
{
private:
	static constexpr float c_MomentumDamping = 0.98f;
	static constexpr float c_DefaultMass = 100.0f;
	static constexpr float c_RestSpeed = 0.00000001f;
	static constexpr Vector3 c_Gravity = Vector3(0.0f, -9.81f, 0.0f);

	Entity& m_AttachedEntity;
	float m_Mass;
	float m_InverseMass;
	Vector3 m_ForceSum;

public:
	bool IsGravityEnabled;
	Vector3 AngularVelocity;
	Vector3 LinearVelocity;
	Vector3 Translation;
	Vector4 Rotation;

	Rigidbody(Entity& entity);
	~Rigidbody();

	void SetMass(const float& mass);
	const float& GetMass() const;

	void FixedUpdate();
	void Update(const float& deltaTime);
};