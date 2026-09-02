#include "pch.h"
#include "PhysicsWorld.h"
#include "Rigidbody.h"
#include <Physics/SAT/SeparatingAxisTheorem.h>
#include <World/World.h>

#include <Physics/Optimisations/Octree.h>
#include <Physics/Optimisations/SweepAndPrune.h>
#include <Physics/Optimisations/SpatialGrid.h>

PhysicsWorld::PhysicsWorld()
{
	m_RigidbodyList = new Rigidbody[c_MaxRigidbodyCount];
	m_ActiveRigidbodyCount = 0;
	ResolutionType = 1;
	m_OctreeRoot = nullptr;
	m_SweepAndPrune = nullptr;
	m_SpatialGrid = nullptr;
}

PhysicsWorld::~PhysicsWorld()
{
	if (m_RigidbodyList != nullptr)
	{
		delete[] m_RigidbodyList;
		m_RigidbodyList = nullptr;
	}

	if (m_OctreeRoot != nullptr)
	{
		delete m_OctreeRoot;
		m_OctreeRoot = nullptr;
	}

	if (m_SweepAndPrune != nullptr)
	{
		delete m_SweepAndPrune;
		m_SweepAndPrune = nullptr;
	}

	if (m_SpatialGrid != nullptr)
	{
		delete m_SpatialGrid;
		m_SpatialGrid = nullptr;
	}
}

void PhysicsWorld::Initialise(const WORLD_EXAMPLE_SCENE& exampleScene)
{
	switch (exampleScene)
	{
	case WORLD_GJK_EXAMPLE_SPATIAL_GRID:
	case WORLD_SPATIAL_GRID:
	{
		m_SpatialGrid = new SpatialGrid();
	}
	break;

	case WORLD_SWEEP_AND_PRUNE:
	{
		m_SweepAndPrune = new SweepAndPrune();
	}
	break;

	case WORLD_OCTREE:
	{
		m_OctreeRoot = OctreeNode::BuildOctree(nullptr, { 0.0f, 0.0f, 0.0f }, 4096.0f, 0);
	}
	break;

	case WORLD_EMPTY_SCENE:
	case WORLD_COLLIDER_EXAMPLE:
	case WORLD_SAT_EXAMPLE:
	case WORLD_GJK_EXAMPLE:
	default:
	{

	}
	break;
	}
}

void PhysicsWorld::AddToBroadPhase(const WORLD_EXAMPLE_SCENE& exampleScene, Entity* entity)
{
	if (entity->GetCollider() == nullptr)
		return;

	switch (exampleScene)
	{
	case WORLD_SPATIAL_GRID:
	{
		m_SpatialGrid->AddObject(entity);
	}
	break;

	case WORLD_SWEEP_AND_PRUNE:
	{
		m_SweepAndPrune->AddObject(entity);
	}
	break;

	case WORLD_OCTREE:
	{
		m_OctreeRoot->AddObject({ entity });
	}
	break;

	default:
		break;
	}
}

void PhysicsWorld::FixedUpdate()
{
	IntegrateAccelerationAndVelocities();
	CollectCollisionPairs();
	SolveConstaints();
	IntergratePositions();
	UpdateSleepers();
	CleanupPhysicsObjects();

	for (size_t i = 0; i < m_ActiveRigidbodyCount; i++)
	{
		m_RigidbodyList[i].StopMoving();
		m_RigidbodyList[i].ClearForces();
	}
}

void PhysicsWorld::Update(const double& deltaTime)
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

#include <Physics/Collision Detection/CollisionDetection.h>
void PhysicsWorld::OnIMGUIRender()
{
	ImGui::Begin("Physics");

	if (m_OctreeRoot)
	{
		m_OctreeRoot->RenderIMGUIDetails();
	}
	
	if (m_SweepAndPrune)
	{
		m_SweepAndPrune->RenderIMGUIDetails();
	}

	if (m_SpatialGrid)
	{
		m_SpatialGrid->RenderIMGUIDetails();
	}

	const ImGuiComboFlags flags = 0;

	static COLLIDER_DRAW_LEVEL s_ColliderDrawType = Collider::g_DrawColliders;

	if (ImGui::BeginCombo("Draw Colliders?", c_ColliderDrawLevelStrings[s_ColliderDrawType].c_str(), flags))
	{
		for (int n = 0; n < IM_COUNTOF(c_ColliderDrawLevelStrings); n++)
		{
			const bool is_selected = ((int)s_ColliderDrawType == n);
			if (ImGui::Selectable(c_ColliderDrawLevelStrings[n].c_str(), is_selected))
			{
				s_ColliderDrawType = Collider::g_DrawColliders = (COLLIDER_DRAW_LEVEL)n;
			}

			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	//ImGui::DragInt("Resolution Type", &ResolutionType, 1.0f, 0, 2);
	//ImGui::DragInt("Resolver Iterations", &CollisionResolver::RESOLUTION_ITERATIONS, 1, 1, 100);
	//ImGui::DragFloat("Air Friction", &CollisionResolver::AIR_FRICTION, 0.01f, 0.0f, 1.0f);
	//ImGui::DragFloat("Baumgarte", &CollisionResolver::BAUMGARTE_STABILISATION_VALUE, 0.01f, 0.0f, 1.0f);
	//ImGui::DragFloat("Intersection allowance", &CollisionResolver::ALLOWED_INTERSECTION, 0.01f, 0.0f, 1.0f);

	if (ImGui::CollapsingHeader("Data"))
	{
		ImGui::SeparatorText("Manifold Details");

		if (m_FrameCollisionManifolds.size() > 0)
		{
			for (size_t f = 0; f < m_FrameCollisionManifolds.size(); f++)
			{
				if ((m_FrameCollisionManifolds[f].IsBroadPhaseColliding || m_FrameCollisionManifolds[f].IsNarrowPhaseColliding) == false)
				{
					continue;
				}

				ImGui::Separator();

				ImGui::Text("%s vs. %s\n", m_FrameCollisionManifolds[f].CollisionPair.first->GetDisplayName().c_str(), m_FrameCollisionManifolds[f].CollisionPair.second->GetDisplayName().c_str());
				ImGui::Text("Broad phase collision: %s\n", (m_FrameCollisionManifolds[f].IsBroadPhaseColliding) ? "True" : "False");
				ImGui::Text("Narrow phase collision: %s\n", (m_FrameCollisionManifolds[f].IsNarrowPhaseColliding) ? "True" : "False");

				if (m_FrameCollisionManifolds[f].IsNarrowPhaseColliding == false)
					continue;

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

void PhysicsWorld::IntegrateAccelerationAndVelocities()
{
	for (size_t i = 0; i < m_ActiveRigidbodyCount; i++)
	{
		Rigidbody& rb = m_RigidbodyList[i];

		if (rb.GetInverseMass() <= FLT_EPSILON || rb.IsSleeping || rb.IsStatic || (rb.IsActive == false))
			continue;

		float inverseMass = rb.GetInverseMass();
		glm::vec3 acceleration = rb.Forces * inverseMass;

		//Add Gravity
		if (rb.IsGravityEnabled)
		{
			acceleration += glm::vec3(c_Gravity.x, c_Gravity.y, c_Gravity.z);
		}

		rb.LinearVelocity += acceleration * FIXED_TIMESTEP;

		rb.UpdateInertiaTensor();

		glm::vec3 angularAcceleration = rb.WorldInverseInertiaTensor * rb.Torques;
		rb.AngularVelocity += angularAcceleration * FIXED_TIMESTEP;

	}
}

void PhysicsWorld::CollectCollisionPairs()
{
	m_FrameCollisionManifolds.clear();
	m_FrameConstraintPoints.clear();

	CollisionManifold manifold;
	Entity* entityA = nullptr;
	Entity* entityB = nullptr;

	if (m_SpatialGrid || m_SweepAndPrune || m_OctreeRoot)
	{
		if (m_CollisionPairs.size() == 0)
		{
			if (m_SpatialGrid)
			{
				m_SpatialGrid->DetermineCollisionPairs(m_CollisionPairs);
			}

			if (m_SweepAndPrune)
			{
				m_SweepAndPrune->DetermineCollisionPairs(m_CollisionPairs);
			}

			if (m_OctreeRoot)
			{
				m_OctreeRoot->DetermineCollisionPairs(m_CollisionPairs);
			}
		}
	}
	else
	{
		m_CollisionPairs.clear();

		for (size_t i = 0; i < m_ActiveRigidbodyCount; i++)
		{
			for (size_t j = i + 1; j < m_ActiveRigidbodyCount; j++)
			{
				m_CollisionPairs.push_back({ m_RigidbodyList[i].GetEntity(), m_RigidbodyList[j].GetEntity() });
			}
		}
	}

	for (size_t i = 0; i < m_CollisionPairs.size(); i++)
	{
		std::pair<Entity*, Entity*>& pair = m_CollisionPairs[i];

		entityA = pair.first;
		entityB = pair.second;
		
		if (entityA->GetCollider() == nullptr || entityB->GetCollider() == nullptr)
		{
			continue;
		}

		manifold.Reset();

		CollisionDetection::CheckNarrowPhaseCollision(entityA->GetCollider(), entityB->GetCollider(), &manifold);

		manifold.CollisionPair.first = entityA;
		manifold.CollisionPair.second = entityB;
		m_FrameCollisionManifolds.push_back(manifold);
	}
}
 
void PhysicsWorld::SolveConstaints()
{
	if (m_FrameCollisionManifolds.size() > 0)
	{
		switch (ResolutionType)
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
	for (size_t i = 0; i < m_ActiveRigidbodyCount; i++)
	{
		Rigidbody& rb = m_RigidbodyList[i];
		if (rb.GetInverseMass() <= FLT_EPSILON || rb.IsSleeping || rb.IsStatic || (rb.IsActive == false))
			continue;

		rb.Translation += (rb.LinearVelocity * FIXED_TIMESTEP);

		//Apply Damping
		rb.LinearVelocity *= (c_LinearDamping);

		//Integrating angular velocity.
		glm::vec3 omega = rb.AngularVelocity * FIXED_TIMESTEP * 0.5f;
		glm::quat angularDeriv = glm::quat(omega.x, omega.y, omega.z, 0.0f) * rb.Rotation;
		rb.Rotation += angularDeriv;
		rb.Rotation = glm::normalize(rb.Rotation);

		rb.AngularVelocity *= (c_AngularDamping);
	}
}

void PhysicsWorld::UpdateSleepers()
{

}

void PhysicsWorld::CleanupPhysicsObjects()
{

}

void PhysicsWorld::Render(Renderer& renderer)
{
	if (m_OctreeRoot)
	{
		m_OctreeRoot->Render(renderer);
	}

	if (m_SweepAndPrune)
	{
		m_SweepAndPrune->Render(renderer);
	}

	if (m_SpatialGrid)
	{
		m_SpatialGrid->Render(renderer);
	}
}

std::string PhysicsWorld::GetExtraDetailsString()
{
	if (m_OctreeRoot)
	{
		return m_OctreeRoot->GetBroadPhaseStatsString();
	}

	if (m_SweepAndPrune)
	{
		return m_SweepAndPrune->GetBroadPhaseStatsString();
	}

	if (m_SpatialGrid)
	{
		return m_SpatialGrid->GetBroadPhaseStatsString();
	}

	return "";
}