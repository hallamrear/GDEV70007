#include "pch.h"
#include "World/World.h"
#include "World/Entity.h"
#include <System/ServiceLocator.h>
#include <System/AssetManagement.h>
#include <Rendering/IMGUIIncludes.h>
#include <Physics/Colliders/Collider.h>
#include <Physics/Optimisations/Octree.h>
#include <Physics/Rigidbody.h>
#include <Physics/GJK/GJK.h>

Entity* World::TestBoxA = nullptr;
Entity* World::TestBoxB = nullptr;
static CollisionManifold manifold{};
static bool state = false;
static bool resolveCollision = false;

const bool& World::IsInitialised() const
{
	return m_IsInitialised;
}

World::World()
{
	m_EntityMap = EntityMap();
	m_IsInitialised = false;
	m_OctreeRoot = nullptr;
}

World::~World()
{
	assert(m_IsInitialised);
}

bool World::Initialise()
{
	Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
	//m_OctreeRoot = OctreeNode::BuildOctree(nullptr, Vector3(0.0f, 0.0f, 0.0f), 8192.0f, 0);

	//Entity* testRoom = CreateEntity("Test Room");
	//ModelRef ref = ServiceLocator::Locate<AssetManager>()->GetModel("Demo_Level.glb");
	//testRoom->SetModel(ref);
	//testRoom->SetPosition(Vector3(-60.0f, 0.0f, 0.0f));

	Entity* AAA = CreateEntity("AAA");
	ModelRef stacking = ServiceLocator::Locate<AssetManager>()->GetModel("Colliders/BoxCollider.glb");
	AAA->SetModel(stacking);
	AAA->SetPosition(Vector3(-60.0f, 0.0f, 0.0f));
		
	auto c = CreateEntity("Cone");
	c->AddCollider(COLLIDER_TYPE::COLLIDER_TYPE_AABB);
	ModelRef cone = ServiceLocator::Locate<AssetManager>()->GetModel("Barrel.glb");
	c->SetModel(cone);
	c->SetPosition(Vector3(-40.0f, 0.0f, 0.0f));

	TestBoxA = CreateEntity("Box A");
	TestBoxA->AddCollider(COLLIDER_TYPE::COLLIDER_TYPE_CONVEX_HULL);
	//TestBoxA->GetCollider()->SetSize(Vector3(10.0f, 10.0f, 10.0f));
	TestBoxA->SetPosition(Vector3(-20.0f, 30.0f, 0.0f));
	ModelRef suzaane = ServiceLocator::Locate<AssetManager>()->GetModel("LargeSuzanne.glb");
	TestBoxA->SetModel(suzaane);

	TestBoxB = CreateEntity("Box B");
	ModelRef test = ServiceLocator::Locate<AssetManager>()->GetModel("TestConvexHull.glb");
	TestBoxB->SetModel(test);
	//TestBoxB->GetCollider()->SetSize(Vector3(7.0f, 3.0f, 5.0f));
	TestBoxB->SetPosition(Vector3(20.0f, 0.0f, 0.0f));

	/*Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
	Entity* entity = nullptr;
	for (size_t i = 0; i < 10000; i++)
	{
		entity = CreateEntity("Test Room");
		ModelRef suzaane = ServiceLocator::Locate<AssetManager>()->GetModel("Suzanne.glb");
		entity->SetModel(suzaane);

		position.x = (float)((rand() % 2000) - 1000);
		position.y = (float)((rand() % 2000) - 1000);
		position.z = (float)((rand() % 2000) - 1000);

		entity->SetPosition(position);
	}*/

	m_IsInitialised = true;
	return true;
}

bool World::Shutdown()
{
	for (auto& entity : m_EntityMap)
	{
		if (entity.second != nullptr)
		{
			delete entity.second;
			entity.second = nullptr;
		}
	}

	m_EntityMap.clear();
	m_IsInitialised = false;

	return false;
}

World* World::CreateWorld()
{
	World* world = new World();

	bool initialised = world->Initialise();

	if (initialised == false)
	{
		printf("Failed to initialise world.");
		delete world;
		world = nullptr;
	}

	return world;
}

bool World::DestroyWorld(World* world)
{
	if (world == nullptr)
	{
		printf("Trying to destroy a world that doesn't exist.");
		return false;
	}

	if (world->IsInitialised() == false)
	{
		printf("Trying to destroy a world that doesn't exist.");
		return false;
	}

	return true;
}

Entity* World::CreateEntity()
{
	Entity* entity = new Entity();
	m_EntityMap.insert(std::make_pair(entity->GetID(), entity));

	if (m_OctreeRoot)
	{
		m_OctreeRoot->AddEntity({ entity });
	}

	entity->SetRigidbody(m_PhysicsWorld.GetFreshRigidbody());

	return entity;
}

Entity* World::CreateEntity(const std::string& displayName)
{
	Entity* entity = CreateEntity();
	entity->SetDisplayName(displayName);
	return entity;
}

void World::FixedUpdate()
{
	for (auto& itr : m_EntityMap)
	{
		if (itr.second != nullptr)
		{
			itr.second->FixedUpdate();
		}
	}

	DestroyDeadEntities();

	m_PhysicsWorld.FixedUpdate();
}

#include <System/InputListener.h>
#include <Physics/SAT/SeparatingAxisTheorem.h>
void World::Update(const float& deltaTime)
{
	m_PhysicsWorld.Update(deltaTime);

	for (auto& entity : m_EntityMap)
	{
		if (entity.second != nullptr)
		{
			entity.second->Update(deltaTime);
		}

		/*for (auto& other : m_EntityMap)
		{
			if (entity == other)
				continue;

			if (entity.second == nullptr || other.second == nullptr)
				continue;

			Collider* colliderA = other.second->GetCollider();
			Collider* colliderB = entity.second->GetCollider();

			if (colliderA != nullptr && colliderB != nullptr)
			{
				state = GJK::CheckCollision(*colliderA, *colliderB, &manifold);
			}
		}*/
	}


	state = false;
	manifold.Reset();

	if (TestBoxA && TestBoxB)
	{
		Collider* colliderA = TestBoxA->GetCollider();
		Collider* colliderB = TestBoxB->GetCollider();
		if (colliderA != nullptr && colliderB != nullptr)
		{
			state = SeparatingAxisTheorem::CheckCollision(*colliderA, *colliderB, &manifold);
		}

		if (resolveCollision && state)
		{
			float halfDepth = -manifold.Depth * 0.5f;
			Vector3 displacement = Vector3(
				manifold.ContactPoints[0].Normal.x * halfDepth,
				manifold.ContactPoints[0].Normal.y * halfDepth,
				manifold.ContactPoints[0].Normal.z * halfDepth);

			TestBoxA->Translate(displacement);

			displacement = Vector3(
				manifold.ContactPoints[1].Normal.x * halfDepth,
				manifold.ContactPoints[1].Normal.y * halfDepth,
				manifold.ContactPoints[1].Normal.z * halfDepth);

			TestBoxB->Translate(displacement);
		}
	}
}

void World::OnIMGUIRender()
{
	ImGui::Begin("Test Collision");

	ImGui::Checkbox("Resolve Collision?", &resolveCollision);

	ImGui::Text(state ? "Colliding" : "Not Colliding");

	ImGui::SeparatorText("Manifold Details");
	if (state)
	{
		ImGui::Text("Depth : %f", manifold.Depth);

		if (ImGui::BeginTable("Hit Points", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
		{
			ImGui::TableSetupColumn("Index");
			ImGui::TableSetupColumn("Hit Point");
			ImGui::TableSetupColumn("Normal");
			ImGui::TableHeadersRow();

			for (size_t i = 0; i < manifold.ContactPoints.size(); i++)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%i", i);

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%f %f %f\n", manifold.ContactPoints[i].HitPoint.x, manifold.ContactPoints[i].HitPoint.y, manifold.ContactPoints[i].HitPoint.z);

				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%f %f %f\n", manifold.ContactPoints[i].Normal.x, manifold.ContactPoints[i].Normal.y, manifold.ContactPoints[i].Normal.z);
			}

			ImGui::EndTable();
		}
	}
	else
	{
		ImGui::Text("No collision.");
	}

	ImGui::End();

	ImGui::Begin("World");

	if (ImGui::Button("Create New Entity"))
	{
		CreateEntity();
	}

	for (auto& entity : m_EntityMap)
	{
		if (entity.second != nullptr)
		{
			RenderEntityDetails(*entity.second);
		}

	}
	ImGui::End();
}

void World::RenderEntityDetails(Entity& entity)
{
	std::string imguiHash = "###" + entity.GetIDString();
	
	ImGui::PushID(imguiHash.c_str());
	std::string headerHash = entity.GetDisplayName() + imguiHash;

	ImGui::SeparatorText(entity.GetDisplayName().c_str());

	std::string entityEditModalStr = "EntityEditModal" + imguiHash;
	if(ImGui::Button("Edit Entity"))
	{
		ImGui::OpenPopup(entityEditModalStr.c_str());
	}

	std::string EntityEditRigidbodyModalStr = "EntityEditRigidbody222" + imguiHash;

	if (ImGui::BeginPopupModal(entityEditModalStr.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("GUID: %s", entity.GetIDString().c_str());
		ImGui::SameLine();
		if (ImGui::Button("Delete Entity"))
		{
			entity.Destroy();
		}

		char buffer[MAX_DISPLAY_NAME_LENGTH] = { 0 };
		strcpy_s(&buffer[0], MAX_DISPLAY_NAME_LENGTH, entity.GetDisplayName().c_str());
		if (ImGui::InputText("Display Name: ", &buffer[0], MAX_DISPLAY_NAME_LENGTH))
		{
			entity.SetDisplayName(buffer);
		}

		ImGui::SeparatorText("Position and Rotation");

		ImGui::InputFloat3("Position", &entity.GetRigidbody().Translation.x);
		ImGui::SameLine();

		if (ImGui::Button("Reset Position"))
		{
			entity.GetRigidbody().Translation = Vector3(0.0f, 0.0f, 0.0f);
		}

		ImGui::InputFloat4("Rotation (Quat)", &entity.GetRigidbody().Rotation.x);
		ImGui::SameLine();

		if (ImGui::Button("Reset Rotation"))
		{
			entity.GetRigidbody().Rotation = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		}

		ImGui::SeparatorText("Models and Colliders");

		std::string modelDisplayName = "None";

		if (entity.GetModel().get() != nullptr)
		{
			modelDisplayName = entity.GetModel()->GetDisplayName();
		}

		ImGui::Text("Model: %s\n", modelDisplayName.c_str());

		if (ImGui::Button("Change Model"))
		{
			ImGui::OpenPopup("Change Model Modal");
		}

		ImGui::Spacing();

		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Change Model Modal", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			static char modelName[MAX_DISPLAY_NAME_LENGTH] = { "\0" };

			if (entity.GetModel() != NULL)
			{
				ImGui::Text("Current File: %s\n", entity.GetModel()->GetDisplayName().c_str());
			}

			ImGui::Text("New Model location (relative to the content folder)");
			ImGui::SameLine();
			ImGui::InputText("###NewModelTextInput", &modelName[0], MAX_DISPLAY_NAME_LENGTH, ImGuiInputFlags_::ImGuiInputFlags_None);

			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				if (strlen(modelName) > 0)
				{
					ModelRef attemptedModel = ServiceLocator::Locate<AssetManager>()->GetModel(modelName);
					entity.SetModel(attemptedModel);
				}

				ImGui::CloseCurrentPopup();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();

			}
			ImGui::EndPopup();
		}

		if (entity.GetCollider() != nullptr)
		{
			Collider& collider = *entity.GetCollider();

			switch (collider.GetType())
			{
			case COLLIDER_TYPE::COLLIDER_TYPE_SPHERE:
			{
				float sphereColliderRadius = { collider.GetSize().x };
				if (ImGui::SliderFloat("Sphere Collider Radius\n", &sphereColliderRadius, 0.1f, 10.0f))
				{
					collider.SetSize(Vector3(sphereColliderRadius, sphereColliderRadius, sphereColliderRadius));
				}
			}
			break;

			case COLLIDER_TYPE::COLLIDER_TYPE_AABB:
			{
				float boxSize[3] = { collider.GetSize().x, collider.GetSize().y, collider.GetSize().z };
				if (ImGui::DragFloat3("Box Collider (half-size)", &boxSize[0], 1.0f, 0.1f, 10.0f, "%.1f", ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp))
				{
					collider.SetSize(Vector3(boxSize));
				}
			}
			break;

			default:
				break;
			}

			if (entity.GetModel() != nullptr)
			{
				if (ImGui::Button("Set Collider Size from current mesh\n"))
				{
					entity.AddColliderFromModel(entity.GetCollider()->GetType());
				}
			}
		}
		else
		{
			const ImGuiComboFlags flags = 0;

			static COLLIDER_TYPE selectedCollider = COLLIDER_TYPE::COLLIDER_TYPE_SPHERE;

			if (ImGui::BeginCombo("Collider Type List", c_ColliderTypeNames[selectedCollider].c_str(), flags))
			{
				for (int n = 0; n < IM_COUNTOF(c_ColliderTypeNames); n++)
				{
					const bool is_selected = ((int)selectedCollider == n);
					if (ImGui::Selectable(c_ColliderTypeNames[n].c_str(), is_selected))
					{
						selectedCollider = (COLLIDER_TYPE)n;
					}

					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			if (ImGui::Button("Create selected collider"))
			{
				entity.AddCollider(selectedCollider);
			}
		}

		ImGui::SeparatorText("Rigidbody");

		if (ImGui::Button("Edit Rigidbody"))
		{
			ImGui::OpenPopup("Edit Rigidbody Data");
		}

		if (ImGui::BeginPopupModal("Edit Rigidbody Data", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			Rigidbody& rigidbody = entity.GetRigidbody();

			ImGui::Checkbox("Is Gravity Enabled", &rigidbody.IsGravityEnabled);

			ImGui::InputFloat3("Linear Velocity", &rigidbody.LinearVelocity.x);
			ImGui::InputFloat3("Angular Velocity", &rigidbody.AngularVelocity.x);

			float mass = rigidbody.GetMass();

			if (ImGui::DragFloat("Mass", &mass, 1.0f, 0.0f, 1000.0f))
			{
				rigidbody.SetMass(mass);
			}

			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (ImGui::Button("OK", ImVec2(120, 0))) 
		{ 
			ImGui::CloseCurrentPopup();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{ 
			ImGui::CloseCurrentPopup(); 
		}

		ImGui::EndPopup();
	}

	ImGui::PopID();
}

void World::DestroyDeadEntities()
{
	for (EntityMap::iterator itr = m_EntityMap.begin(); itr != m_EntityMap.end(); )
	{
		if (itr->second->IsPendingDestroy())
		{
			itr = m_EntityMap.erase(itr);
		}
		else 
		{
			++itr;
		}
	}	
}

void World::Render(Renderer& renderer)
{
	for (auto& entity : m_EntityMap)
	{
		if (entity.second != nullptr)
		{
			entity.second->Render(renderer);
		}
	}

	if (m_OctreeRoot != nullptr)
	{
		//OctreeNode::Render(renderer, m_OctreeRoot);
	}
}