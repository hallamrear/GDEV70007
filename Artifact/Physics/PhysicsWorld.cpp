#include "pch.h"
#include "PhysicsWorld.h"
#include "Rigidbody.h"

PhysicsWorld::PhysicsWorld()
{
	m_RigidbodyList = new Rigidbody[c_MaxRigidbodyCount];
	m_ActiveRigidbodyCount = 0;
}

PhysicsWorld::~PhysicsWorld()
{
	if (m_RigidbodyList != nullptr)
	{
		delete[] m_RigidbodyList;
		m_RigidbodyList = nullptr;
	}
}

void PhysicsWorld::FixedUpdate()
{
	IntegrateVelocities();
	CollectCollisionPairs();
	SolveConstaints();
	IntergratePositions();
	UpdateSleepers();
	CleanupPhysicsObjects();
}

void PhysicsWorld::Update(const float& deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);
}

Rigidbody& PhysicsWorld::GetFreshRigidbody()
{
	Rigidbody& rb = m_RigidbodyList[m_ActiveRigidbodyCount];
	m_ActiveRigidbodyCount++;
	rb.IsActive = true;
	return rb;
}

void PhysicsWorld::IntegrateVelocities()
{
	for (size_t i = 0; i < c_MaxRigidbodyCount; i++)
	{
		Rigidbody& rb = m_RigidbodyList[i];
		
		if (rb.GetInverseMass() <= FLT_EPSILON || rb.IsSleeping || rb.IsStatic || (rb.IsActive == false))
			continue;
		
		float mass = rb.GetMass();
		float inverseMass = rb.GetInverseMass();

		//Add Gravity
		if (rb.IsGravityEnabled)
		{
			Vector3 gravity = Vector3(c_Gravity.x * mass, c_Gravity.y * mass, c_Gravity.z * mass);
			rb.AddForce(gravity);
		}

		//Calculate Acceleration
		Vector3 linearAcceleration{ rb.Forces.x * inverseMass, rb.Forces.y * inverseMass, rb.Forces.z * inverseMass };

		//Calculate Velocity
		rb.LinearVelocity.x += (linearAcceleration.x * FIXED_TIMESTEP);
		rb.LinearVelocity.y += (linearAcceleration.y * FIXED_TIMESTEP);
		rb.LinearVelocity.z += (linearAcceleration.z * FIXED_TIMESTEP);

		//Calculate Angular Acceleration
		Vector3 angularAcceleration{ rb.Torques.x * rb.InertiaTensor.x, rb.Torques.y * rb.InertiaTensor.y, rb.Torques.z * rb.InertiaTensor.z };

		//Calculate Angular Velocity
		rb.AngularVelocity.x += (angularAcceleration.x * FIXED_TIMESTEP);
		rb.AngularVelocity.y += (angularAcceleration.y * FIXED_TIMESTEP);
		rb.AngularVelocity.z += (angularAcceleration.z * FIXED_TIMESTEP);

		//Apply Damping
		rb.LinearVelocity.x *= (c_LinearDamping);
		rb.LinearVelocity.y *= (c_LinearDamping);
		rb.LinearVelocity.z *= (c_LinearDamping);
		rb.AngularVelocity.x *= (c_AngularDamping);
		rb.AngularVelocity.y *= (c_AngularDamping);
		rb.AngularVelocity.z *= (c_AngularDamping);

		//Reset Forces
		rb.Forces = Vector3(0.0f, 0.0f, 0.0f);
		rb.Torques = Vector3(0.0f, 0.0f, 0.0f);
	}
}

void PhysicsWorld::CollectCollisionPairs()
{

}
 
void PhysicsWorld::SolveConstaints()
{

}

void PhysicsWorld::IntergratePositions()
{
	for (size_t i = 0; i < c_MaxRigidbodyCount; i++)
	{
		Rigidbody& rb = m_RigidbodyList[i];
		if (rb.GetInverseMass() <= FLT_EPSILON || rb.IsSleeping || rb.IsStatic || (rb.IsActive == false))
			continue;

		rb.Translation.x += (rb.LinearVelocity.x * FIXED_TIMESTEP);
		rb.Translation.y += (rb.LinearVelocity.y * FIXED_TIMESTEP);
		rb.Translation.z += (rb.LinearVelocity.z * FIXED_TIMESTEP);

		//Integrating angular velocity.
		Quaternion omega = Quaternion(rb.AngularVelocity.x, rb.AngularVelocity.y, rb.AngularVelocity.z, 0.0f);
		
		Quaternion angularDeriv;
		DirectX::XMStoreFloat4(&angularDeriv, DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&omega), DirectX::XMLoadFloat4(&rb.Rotation)));
		angularDeriv.x *= 0.5f;
		angularDeriv.y *= 0.5f;
		angularDeriv.z *= 0.5f; 
		angularDeriv.w *= 0.5f;

		rb.Rotation.x += (angularDeriv.x * FIXED_TIMESTEP);
		rb.Rotation.y += (angularDeriv.y * FIXED_TIMESTEP);
		rb.Rotation.z += (angularDeriv.z * FIXED_TIMESTEP);
		rb.Rotation.w += (angularDeriv.w * FIXED_TIMESTEP);

		DirectX::XMStoreFloat4(&rb.Rotation, DirectX::XMQuaternionNormalize(DirectX::XMLoadFloat4(&rb.Rotation)));
	}
}

void PhysicsWorld::UpdateSleepers()
{

}

void PhysicsWorld::CleanupPhysicsObjects()
{

}