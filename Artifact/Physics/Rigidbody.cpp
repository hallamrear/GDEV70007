#include "pch.h"
#include "Rigidbody.h"

Rigidbody::Rigidbody() : m_Entity(nullptr)
{
	m_Mass = 0.0f;
	m_InverseMass = 0.0f;
	m_CentreOfMass = Vector3(0.0f, 0.0f, 0.0f);
	InverseInertiaTensor = glm::vec3();
	WorldInverseInertiaTensor = glm::mat3x3();
	Translation = glm::vec3(0.0f, 0.0f, 0.0f);
	Rotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	LinearVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
	AngularVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
	Forces = glm::vec3(0.0f, 0.0f, 0.0f);
	Torques = glm::vec3(0.0f, 0.0f, 0.0f);
	IsGravityEnabled = false;
	IsActive = false;
	IsSleeping = false;
	IsStatic = false;
	SetMass(c_DefaultMass);
}

Rigidbody::~Rigidbody()
{
	m_Entity = nullptr;
	m_Mass = 0.0f;
	m_InverseMass = 0.0f;
	m_CentreOfMass = Vector3(0.0f, 0.0f, 0.0f);
	InverseInertiaTensor = glm::vec3();
	WorldInverseInertiaTensor = glm::mat3x3();
	Translation = glm::vec3(0.0f, 0.0f, 0.0f);
	Rotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	LinearVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
	AngularVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
	Forces = glm::vec3(0.0f, 0.0f, 0.0f);
	Torques = glm::vec3(0.0f, 0.0f, 0.0f);
	IsGravityEnabled = false;
	IsActive = false;
	IsSleeping = false;
	IsStatic = false;
	SetMass(0.0f);
}

void Rigidbody::SetMass(const float& mass)
{
	m_Mass = std::max(mass, 0.0f);

	if (m_Mass < FLT_EPSILON)
	{
		m_InverseMass = 0.0f;
	}
	else
	{
		m_InverseMass = 1.0f / m_Mass;
	}

	float w = 10.0f;
	float h = w;
	float d = w;

	InverseInertiaTensor =
	{
		//(m_Mass / 12.0f) * (h * h + d * d), 0.0f, 0.0f,
		//0.0f, (m_Mass / 12.0f) * (w * w + d * d), 0.0f,
		//0.0f, 0.0f, (m_Mass / 12.0f) * (w * w + h * h)

		(m_InverseMass * 12.0f) * (h * h + d * d),
		(m_InverseMass * 12.0f) * (w * w + d * d),
		(m_InverseMass * 12.0f) * (w * w + h * h)
	};
}

const float& Rigidbody::GetMass() const
{
	return m_Mass;
}

float Rigidbody::GetInverseMass()
{
	return m_InverseMass;
}

void Rigidbody::SetEntity(Entity* entity)
{
	m_Entity = entity;
}

const Entity& Rigidbody::GetEntity() const
{
	return *m_Entity;
}

Entity* Rigidbody::GetEntity()
{
	return m_Entity;
}

void Rigidbody::StopMoving()
{
	Forces = { 0.0f, 0.0f, 0.0f };
	Torques = { 0.0f, 0.0f, 0.0f };
	LinearVelocity = { 0.0f, 0.0f, 0.0f };
	AngularVelocity = { 0.0f, 0.0f, 0.0f };
}

void Rigidbody::ApplyAngularInpulse(const glm::vec3& force)
{
	AngularVelocity += WorldInverseInertiaTensor * force;
}

void Rigidbody::ApplyLinearImpulse(const glm::vec3& force)
{
	LinearVelocity += force * m_InverseMass;
}

void Rigidbody::AddForce(const glm::vec3& force)
{
	Forces += force;
}

//Position is in world space. 
void Rigidbody::AddForceAtPosition(const glm::vec3& force, const glm::vec3& position)
{
	glm::vec3 localPosition = position - Translation;
	AddForce(force);
	AddTorque(glm::cross(localPosition, force));
}

void Rigidbody::AddTorque(const glm::vec3& torque)
{
	Torques += torque;
}

void Rigidbody::ClearForces()
{
	Forces = glm::vec3(0.0f, 0.0f, 0.0f);
	Torques = glm::vec3(0.0f, 0.0f, 0.0f);
}

Vector3 Rigidbody::GetAngularVelocityAtPoint(const Vector3& point)
{
	glm::vec3 diff = { point.x - Translation.x, point.y - Translation.y, point.z - Translation.z };
	glm::vec3 cross = glm::cross(AngularVelocity, diff);
	return { cross.x, cross.y, cross.z };
}

void Rigidbody::UpdateInertiaTensor()
{
	glm::mat3 orientation = glm::mat3_cast(Rotation);
	glm::mat3 inverseOrientation = glm::mat3_cast(glm::inverse(Rotation));
	WorldInverseInertiaTensor = orientation * glm::mat3(glm::scale(InverseInertiaTensor)) * inverseOrientation;
}
