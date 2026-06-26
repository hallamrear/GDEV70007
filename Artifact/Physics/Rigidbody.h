#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Entity;

class Rigidbody
{
private:
	Entity* m_Entity;
	static constexpr float c_DefaultMass = 100.0f;
	float m_Mass;
	float m_InverseMass;
	Vector3 m_CentreOfMass;

public:
	glm::mat3x3 InertiaTensor;
	glm::mat3x3 WorldInverseInertiaTensor;
	glm::vec3 Forces;
	glm::vec3 Torques;
	bool IsSleeping;
	bool IsGravityEnabled;
	bool IsStatic;
	bool IsActive;
	glm::vec3 AngularVelocity;
	glm::vec3 LinearVelocity;
	glm::vec3 Translation;
	glm::quat Rotation;

	Rigidbody();
	~Rigidbody();

	void SetMass(const float& mass);
	const float& GetMass() const;

	float GetInverseMass();

	void SetEntity(Entity* entity);
	const Entity& GetEntity() const;
	Entity* GetEntity();

	void StopMoving();

	void AddForce(const Vector3& force);
	void AddTorque(const Vector3& torque);
	Vector3 GetAngularVelocityAtPoint(const Vector3& point);

	void UpdateInertiaTensor();
};

