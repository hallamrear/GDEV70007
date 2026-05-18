#pragma once

class Entity;

class Rigidbody
{
private:
	static constexpr float c_DefaultMass = 100.0f;
	float m_Mass;
	float m_InverseMass;
	Vector3 m_CentreOfMass;

public:
	Vector3 InertiaTensor;
	Vector3 Forces;
	Vector3 Torques;
	bool IsSleeping;
	bool IsGravityEnabled;
	bool IsStatic;
	bool IsActive;
	Vector3 AngularVelocity;
	Vector3 LinearVelocity;
	Vector3 Translation;
	Vector4 Rotation;

	Rigidbody();
	~Rigidbody();

	void SetMass(const float& mass);
	const float& GetMass() const;

	float GetInverseMass();

	void StopMoving();

	void AddForce(const Vector3& force);
	void AddTorque(const Vector3& torque);
	Vector3 GetAngularVelocityAtPoint(const Vector3& point);
};

