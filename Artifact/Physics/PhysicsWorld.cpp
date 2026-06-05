#include "pch.h"
#include "PhysicsWorld.h"
#include "Rigidbody.h"
#include <Physics/SAT/SeparatingAxisTheorem.h>

PhysicsWorld::PhysicsWorld()
{
	m_RigidbodyList = new Rigidbody[c_MaxRigidbodyCount];
	m_ActiveRigidbodyCount = 0;
	m_ResolutionType = 0;
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

void PhysicsWorld::OnIMGUIRender()
{
	ImGui::Begin("Physics");

	if (ImGui::CollapsingHeader("Physics Settings"))
	{
		ImGui::DragInt("Resolution Type", &m_ResolutionType, 1.0f, 0, 2);
		ImGui::DragInt("Resolver Iterations", &CollisionResolver::RESOLUTION_ITERATIONS, 1, 1, 100);
		ImGui::DragFloat("Air Friction", &CollisionResolver::AIR_FRICTION, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Baumgarte", &CollisionResolver::BAUMGARTE_STABILISATION_VALUE, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Intersection allowance", &CollisionResolver::ALLOWED_INTERSECTION, 0.01f, 0.0f, 1.0f);
	}

	if (ImGui::CollapsingHeader("Data"))
	{
		ImGui::SeparatorText("Manifold Details");

		if (m_FrameCollisionManifolds.size() > 0)
		{
			for (size_t f = 0; f < m_FrameCollisionManifolds.size(); f++)
			{
				ImGui::Text("Normal: %f %f %f", m_FrameCollisionManifolds[f].Normal.x, m_FrameCollisionManifolds[f].Normal.y, m_FrameCollisionManifolds[f].Normal.z);

				if (ImGui::BeginTable("Hit Points", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
				{
					ImGui::TableSetupColumn("Index");
					ImGui::TableSetupColumn("Hit Point");
					ImGui::TableSetupColumn("Depth");
					ImGui::TableHeadersRow();

					for (size_t i = 0; i < m_FrameCollisionManifolds[f].ContactPoints.size(); i++)
					{
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("%i", i);

						ImGui::TableSetColumnIndex(1);
						ImGui::Text("%f %f %f", m_FrameCollisionManifolds[f].ContactPoints[i].HitPoint.x, m_FrameCollisionManifolds[f].ContactPoints[i].HitPoint.y, m_FrameCollisionManifolds[f].ContactPoints[i].HitPoint.z);

						ImGui::TableSetColumnIndex(2);
						ImGui::Text("%f", m_FrameCollisionManifolds[f].ContactPoints[i].Depth);
					}

					ImGui::EndTable();
				}
			}
		}
		else
		{
			ImGui::Text("No collisions this frame.");
		}
	}

	ImGui::End();
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
	m_FrameCollisionManifolds.clear();
	m_FrameConstraintPoints.clear();

	CollisionManifold manifold;
	Entity* entityA = nullptr;
	Entity* entityB = nullptr;

	for (size_t x = 0; x < c_MaxRigidbodyCount; x++)
	{
		for (size_t y = x; y < c_MaxRigidbodyCount; y++)
		{
			if (m_RigidbodyList[x].IsActive == false ||
				m_RigidbodyList[y].IsActive == false || 
				x == y)
				continue;

			entityA = m_RigidbodyList[x].GetEntity();
			entityB = m_RigidbodyList[y].GetEntity();

			assert(entityA != nullptr);
			assert(entityB != nullptr);

			manifold.Reset();

			bool collision = SeparatingAxisTheorem::CheckCollision(entityA, entityB, &manifold);

			if (collision)
			{
				manifold.CollisionPair.first = entityA;
				manifold.CollisionPair.second = entityB;
				m_FrameCollisionManifolds.push_back(manifold);
			}
		}
	}
}
 
void PhysicsWorld::SolveConstaints()
{
	if (m_FrameCollisionManifolds.size() > 0)
	{
		switch (m_ResolutionType)
		{
		case 1:
		{
			for (size_t i = 0; i < m_FrameCollisionManifolds.size(); i++)
			{
				CollisionResolver::SoftResolveCollision(m_FrameCollisionManifolds[i]);
			}
		}
		break;

		case 2:
		{

			CollisionResolver::ConvertContactManifoldsToConstrainedPoints(m_FrameCollisionManifolds, m_FrameConstraintPoints);
			CollisionResolver::PrepareConstrainedContacts(m_FrameConstraintPoints);

			m_FrameJacobians = CollisionResolver::CalculateJacobians(m_FrameConstraintPoints);
			m_FrameConstraints = CollisionResolver::CalculateConstraints(m_FrameJacobians, m_FrameConstraintPoints);

			InverseMassMatrix m_InverseMassMatrix = CollisionResolver::ComputeInverseMassMatrix(m_FrameConstraintPoints);

			CollisionResolver::ResolvePositionConstraint(FIXED_TIMESTEP, m_FrameJacobians, m_InverseMassMatrix, m_FrameConstraints, m_FrameConstraintPoints);
			CollisionResolver::ResolveVelocityConstraint(FIXED_TIMESTEP, m_FrameJacobians, m_InverseMassMatrix, m_FrameConstraints, m_FrameConstraintPoints);
		}
		break;

		default:
			break;
		}
	}
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