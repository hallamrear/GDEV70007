#include "pch.h"
#include "Rigidbody.h"

Rigidbody::Rigidbody(Entity& entity) : m_AttachedEntity(entity)
{
	Translation = Vector3(0.0f, 0.0f, 0.0f);
	Rotation = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	LinearVelocity = Vector3(0.0f, 0.0f, 0.0f);
	AngularVelocity = Vector3(0.0f, 0.0f, 0.0f);
	IsGravityEnabled = false;
	SetMass(c_DefaultMass);
}

Rigidbody::~Rigidbody()
{
	Translation = Vector3(0.0f, 0.0f, 0.0f);
	Rotation = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	LinearVelocity = Vector3(0.0f, 0.0f, 0.0f);
	AngularVelocity = Vector3(0.0f, 0.0f, 0.0f);
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
}

const float& Rigidbody::GetMass() const
{
	return m_Mass;
}

void Rigidbody::FixedUpdate()
{
	if (m_InverseMass <= FLT_EPSILON)
	{
		return;
	}

	if (IsGravityEnabled)
	{
		//Adding Weight
		m_ForceSum.x += c_Gravity.x * m_InverseMass;
		m_ForceSum.y += c_Gravity.y * m_InverseMass;
		m_ForceSum.z += c_Gravity.z * m_InverseMass;
	}

	Vector3 length = Vector3();
	DirectX::XMStoreFloat3(&length, DirectX::XMVector3LengthSq(DirectX::XMLoadFloat3(&m_ForceSum)));
	if (length.x > FLT_EPSILON)
	{
		//Acceleration
		LinearVelocity.x += (m_ForceSum.x * m_InverseMass);
		LinearVelocity.y += (m_ForceSum.y * m_InverseMass);
		LinearVelocity.z += (m_ForceSum.z * m_InverseMass);
	}

	DirectX::XMStoreFloat3(&length, DirectX::XMVector3LengthSq(DirectX::XMLoadFloat3(&LinearVelocity)));
	if (length.x < c_RestSpeed)
	{
		LinearVelocity = Vector3(0.0f, 0.0f, 0.0f);
	}

	//Velocity
	Translation.x += LinearVelocity.x * FIXED_TIMESTEP;
	Translation.y += LinearVelocity.y * FIXED_TIMESTEP;
	Translation.z += LinearVelocity.z * FIXED_TIMESTEP;

	m_ForceSum = Vector3(0.0f, 0.0f, 0.0f);
}

void Rigidbody::Update(const float& deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);
}