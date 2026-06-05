#include "pch.h"
#include "Rigidbody.h"

Rigidbody::Rigidbody() : m_Entity(nullptr)
{
	m_Mass = 0.0f;
	m_InverseMass = 0.0f;
	m_CentreOfMass = Vector3(0.0f, 0.0f, 0.0f);
	InertiaTensor = Vector3(0.0f, 0.0f, 0.0f);
	Translation = Vector3(0.0f, 0.0f, 0.0f);
	Rotation = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	LinearVelocity = Vector3(0.0f, 0.0f, 0.0f);
	AngularVelocity = Vector3(0.0f, 0.0f, 0.0f);
	Forces = Vector3(0.0f, 0.0f, 0.0f);
	Torques = Vector3(0.0f, 0.0f, 0.0f);
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
	InertiaTensor = Vector3(0.0f, 0.0f, 0.0f);
	Translation = Vector3(0.0f, 0.0f, 0.0f);
	Rotation = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	LinearVelocity = Vector3(0.0f, 0.0f, 0.0f);
	AngularVelocity = Vector3(0.0f, 0.0f, 0.0f);
	Forces = Vector3(0.0f, 0.0f, 0.0f);
	Torques = Vector3(0.0f, 0.0f, 0.0f);
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

	InertiaTensor =
	{
		(m_Mass / 12.0f) * (h * h + d * d),
		(m_Mass / 12.0f) * (w * w + d * d),
		(m_Mass / 12.0f) * (w * w + h * h)
	};
}

const float& Rigidbody::GetMass() const
{
	return m_Mass;
}

//void Rigidbody::FixedUpdate()
//{
//	if (m_InverseMass <= FLT_EPSILON)
//	{
//		return;
//	}
//
//	if (IsGravityEnabled)
//	{
//		//Adding Weight
//		m_ForceSum.x += c_Gravity.x * m_InverseMass;
//		m_ForceSum.y += c_Gravity.y * m_InverseMass;
//		m_ForceSum.z += c_Gravity.z * m_InverseMass;
//	}
//
//	Vector3 length = Vector3();
//	DirectX::XMStoreFloat3(&length, DirectX::XMVector3LengthSq(DirectX::XMLoadFloat3(&m_ForceSum)));
//	if (length.x > FLT_EPSILON)
//	{
//		//Acceleration
//		LinearVelocity.x += (m_ForceSum.x * m_InverseMass);
//		LinearVelocity.y += (m_ForceSum.y * m_InverseMass);
//		LinearVelocity.z += (m_ForceSum.z * m_InverseMass);
//	}
//
//	DirectX::XMStoreFloat3(&length, DirectX::XMVector3LengthSq(DirectX::XMLoadFloat3(&LinearVelocity)));
//	if (length.x < c_RestSpeed)
//	{
//		LinearVelocity = Vector3(0.0f, 0.0f, 0.0f);
//	}
//
//	//Velocity
//	Translation.x += LinearVelocity.x * FIXED_TIMESTEP;
//	Translation.y += LinearVelocity.y * FIXED_TIMESTEP;
//	Translation.z += LinearVelocity.z * FIXED_TIMESTEP;
//
//	m_ForceSum = Vector3(0.0f, 0.0f, 0.0f);
//}
//
//void Rigidbody::Update(const float& deltaTime)
//{
//	UNREFERENCED_PARAMETER(deltaTime);
//}

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
	Forces = Vector3(0.0f, 0.0f, 0.0f);
	Torques = Vector3(0.0f, 0.0f, 0.0f);
	LinearVelocity = Vector3(0.0f, 0.0f, 0.0f);
	AngularVelocity = Vector3(0.0f, 0.0f, 0.0f);
}

void Rigidbody::AddForce(const Vector3& force)
{
	Forces.x += force.x;
	Forces.y += force.y;
	Forces.z += force.z;
}

void Rigidbody::AddTorque(const Vector3& torque)
{
	Torques.x += torque.x;
	Torques.y += torque.y;
	Torques.z += torque.z;
}

Vector3 Rigidbody::GetAngularVelocityAtPoint(const Vector3& point)
{
	Vector3 diff = Vector3(point.x - Translation.x, point.y - Translation.y, point.z - Translation.z);
	Vector3 out;
	DirectX::XMStoreFloat3(&out, DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&AngularVelocity), DirectX::XMLoadFloat3(&diff)));
	return out;
}