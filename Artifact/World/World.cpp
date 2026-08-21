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
#include <System/Maths.h>

//Temporary stuff.
Entity* World::ControlledEntity = nullptr;
const int World::c_TestCount = 9000;

const bool& World::IsInitialised() const
{
	return m_IsInitialised;
}

World::World()
{
	srand((unsigned int)time(nullptr));
	m_EntityMap = EntityMap();
	m_EntityMap.reserve(c_TestCount);
	m_IsInitialised = false;
	m_Camera = &ServiceLocator::Locate<Renderer>()->GetCamera();
	m_CurrentScene = WORLD_EXAMPLE_SCENE::WORLD_EMPTY_SCENE;
	m_PendingSceneChange = true;
}

World::~World()
{
	assert(m_IsInitialised);
}

void World::ChangeExampleScene(const WORLD_EXAMPLE_SCENE& newScene)
{
	UNREFERENCED_PARAMETER(newScene);
	//for (auto& entity : m_EntityMap)
	//{
	//	entity.second->Kill();
	//}
	//
	//m_CurrentScene = newScene;
	//m_PendingSceneChange = true;
}

bool World::Initialise()
{
	m_CurrentScene = WORLD_EXAMPLE_SCENE::WORLD_OCTREE;
	m_PhysicsWorld.Initialise(m_CurrentScene);

	Vector3 position = Vector3(0.0f, 0.0f, 0.0f);

	ModelRef modelList[] =
	{
		ServiceLocator::Locate<AssetManager>()->GetModel("Suzanne.glb"),
		ServiceLocator::Locate<AssetManager>()->GetModel("TestCar.glb"),
		ServiceLocator::Locate<AssetManager>()->GetModel("Ambulance.glb"),
		ServiceLocator::Locate<AssetManager>()->GetModel("Barrel.glb"),
	};

	switch (m_CurrentScene)
	{
	case WORLD_EMPTY_SCENE:
		break;
	case WORLD_COLLIDER_EXAMPLE:
	{
		ModelRef ship = ServiceLocator::Locate<AssetManager>()->GetModel("PirateShip.glb");
		float gap = 30.0f;
		position = Vector3(-gap * trunc((float)COLLIDER_TYPE::COLLIDER_TYPE_COUNT / 2.0f), 0.0f, 100.0f);

		Entity* entity = nullptr;
		for (size_t i = 0; i < COLLIDER_TYPE::COLLIDER_TYPE_COUNT; i++)
		{
			std::string str = "Collider Type : " + c_ColliderTypeNames[i];
			entity = CreateEntity(str);
			entity->SetModel(ship);
			 
			entity->AddColliderFromModel((COLLIDER_TYPE)i);
			position.x += gap;
			entity->SetPosition(position);

			if (i == (int)COLLIDER_TYPE::COLLIDER_TYPE_OBB)
			{
				entity->Rotate({ 0.0f, 45.0f, 0.0f });
			}
		}

		int count = 7;
		position = Vector3(-gap * trunc(count / 2.0f), 0.0f, 0.0f);
		float rotStep = 360.0f / count;
		float angle = 0.0f;

		for (size_t i = 0; i < count; i++)
		{
			std::string str = "Rotation : " + std::to_string(angle);
			entity = CreateEntity(str);
			entity->SetModel(ship);

			entity->AddColliderFromModel(COLLIDER_TYPE::COLLIDER_TYPE_OBB);
			entity->Rotate(Vector3(0.0f, angle, 0.0f));

			position.x += gap;
			angle += rotStep;

			entity->SetPosition(position);
		}

		std::string str = "OBB Tester";
		entity = CreateEntity(str);
		entity->SetModel(ship);
		entity->AddColliderFromModel(COLLIDER_TYPE::COLLIDER_TYPE_OBB);
		position.x += gap;
		entity->SetPosition(position);
		str = "Hull Tester";
		entity = CreateEntity(str);
		entity->SetModel(ship);
		entity->AddColliderFromModel(COLLIDER_TYPE::COLLIDER_TYPE_CONVEX_HULL);
		position.x += gap;
		entity->SetPosition(position);

		m_PhysicsWorld.ResolutionType = 0;
	}
	break;
	case WORLD_SAT_EXAMPLE:
	{
		float ra = (float)(rand() % 360);
		float rb = (float)(rand() % 360);

		Entity* TestBoxA = CreateEntity("Test Box A");
		TestBoxA->SetPosition({ -10.0f, 0.0f, 0.0f });
		TestBoxA->SetModel(modelList[2]);
		TestBoxA->AddColliderFromModel(COLLIDER_TYPE::COLLIDER_TYPE_OBB);
		TestBoxA->Rotate(Vector3(0.0f, ra, 0.0f));

		Entity* TestBoxB = CreateEntity("Test Box B");
		TestBoxB->SetPosition({ 10.0f, 0.0f, 0.0f });
		TestBoxB->SetModel(modelList[2]);
		TestBoxB->AddColliderFromModel(COLLIDER_TYPE::COLLIDER_TYPE_OBB);
		TestBoxB->Rotate(Vector3(0.0f, rb, 0.0f));

	}
		break;

	case WORLD_GJK_EXAMPLE_SPATIAL_GRID:
	{
		m_CurrentScene = WORLD_SPATIAL_GRID;
		int extents = 2048;

		std::string name = "";
		ModelRef sea = ServiceLocator::Locate<AssetManager>()->GetModel("Sea.glb");

		Entity* Sea = CreateEntity("Sea");
		Sea->SetPosition({ 0.0f, -5.0f, 0.0f });
		Sea->SetModel(sea);

		for (int i = 0; i < c_TestCount; i++)
		{
			float x = (float)((rand() % extents) - (extents / 2));
			float y = (float)((rand() % 10) / 10.0f);
			float z = (float)((rand() % extents) - (extents / 2));
			float r = (float)(rand() % 360);
			
			ModelRef ship = ServiceLocator::Locate<AssetManager>()->GetModel("PirateShip.glb");
			name = "Ship " + std::to_string(i);
			Entity* boat = CreateEntity(name);
			boat->SetPosition({ x, y, z});
			boat->SetModel(ship);
			boat->AddColliderFromModel(COLLIDER_TYPE::COLLIDER_TYPE_CONVEX_HULL);
			boat->Rotate(Vector3(0.0f, r, 0.0f));	
			m_PhysicsWorld.AddToBroadPhase(m_CurrentScene, boat);
		}

		m_PhysicsWorld.ResolutionType = 0;

		m_CurrentScene = WORLD_GJK_EXAMPLE_SPATIAL_GRID;
	}
	break;


	case WORLD_GJK_EXAMPLE:
	{
		float ra = (float)(rand() % 360);
		float rb = (float)(rand() % 360);

		ModelRef ship = ServiceLocator::Locate<AssetManager>()->GetModel("PirateShip.glb");
		ModelRef sea = ServiceLocator::Locate<AssetManager>()->GetModel("Sea.glb");

		Entity* Sea = CreateEntity("Sea");
		Sea->SetPosition({ 0.0f, 3.0f, 0.0f });
		Sea->SetModel(sea);
		Sea->AddColliderFromModel(COLLIDER_TYPE::COLLIDER_TYPE_CONVEX_HULL);

		Entity* TestBoxA = CreateEntity("Ship A");
		TestBoxA->SetPosition({ -10.0f, 0.0f, -3.0f });
		TestBoxA->SetModel(ship);
		TestBoxA->AddColliderFromModel(COLLIDER_TYPE::COLLIDER_TYPE_CONVEX_HULL);
		TestBoxA->Rotate(Vector3(0.0f, ra, 0.0f));

		Entity* TestBoxB = CreateEntity("Ship B");
		TestBoxB->SetPosition({ 10.0f, 0.0f, 6.0f });
		TestBoxB->SetModel(ship);
		TestBoxB->AddColliderFromModel(COLLIDER_TYPE::COLLIDER_TYPE_CONVEX_HULL);
		TestBoxB->Rotate(Vector3(0.0f, rb, 0.0f));

		m_PhysicsWorld.ResolutionType = 0;
	}
		break;

	case WORLD_SPATIAL_GRID:
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float rx = 0.0f;
		float ry = 0.0f;
		float rz = 0.0f;

		int m = 0;
		int extents = 1024;

		ModelRef ship = ServiceLocator::Locate<AssetManager>()->GetModel("PirateShip.glb");

		for (int i = 0; i < c_TestCount; i++)
		{
			m = rand() % _countof(modelList);
			x = (float)((rand() % extents) - (extents / 2));
			y = (float)((rand() % extents) - (extents / 2));
			z = (float)((rand() % extents) - (extents / 2));
			rx = (float)(rand() % 360);
			ry = (float)(rand() % 360);
			rz = (float)(rand() % 360);

			std::string name = "SG Object" + std::to_string(i);
			Entity* entity = CreateEntity(name);
			entity->SetPosition({ x, y, z });
			entity->Rotate(Vector3(rx, ry, rz));
			entity->SetModel(ship);
			entity->AddColliderFromModel(COLLIDER_TYPE::COLLIDER_TYPE_OBB);
			entity->GetRigidbody().SetMass(0.0f);
			m_PhysicsWorld.AddToBroadPhase(m_CurrentScene, entity);
		}
	}
		break;

	case WORLD_SWEEP_AND_PRUNE:
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float rx = 0.0f;
		float ry = 0.0f;
		float rz = 0.0f;
		int m = 0;

		for (int i = 0; i < 1000; i++)
		{
			x = (float)((rand() % 1000) - 500);
			y = (float)((rand() % 1000) - 500);
			z = (float)((rand() % 1000) - 500);
			rx = (float)(rand() % 360);
			ry = (float)(rand() % 360);
			rz = (float)(rand() % 360);
			m = rand() % _countof(modelList);

			std::string name = "SAP Object" + std::to_string(i);
			Entity* entity = CreateEntity(name);
			entity->SetPosition({ x, y, z });
			entity->Rotate(Vector3(rx, ry, rz));
			entity->SetModel(modelList[m]);
			entity->AddColliderFromModel(COLLIDER_TYPE::COLLIDER_TYPE_AABB);
			entity->GetRigidbody().SetMass(0.0f);
			m_PhysicsWorld.AddToBroadPhase(m_CurrentScene, entity);
		}
	}
		break;

	case WORLD_OCTREE:
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float rx = 0.0f;
		float ry = 0.0f;
		float rz = 0.0f;

		int m = 0;
		int extents = 4096;

		ModelRef ship = ServiceLocator::Locate<AssetManager>()->GetModel("PirateShip.glb");

		for (int i = 0; i < c_TestCount; i++)
		{
			m = rand() % _countof(modelList);
			x = (float)((rand() % extents) - (extents / 2));
			y = (float)((rand() % extents) - (extents / 2));
			z = (float)((rand() % extents) - (extents / 2));
			rx = (float)(rand() % 360);
			ry = (float)(rand() % 360);
			rz = (float)(rand() % 360);

			std::string name = "SG Object" + std::to_string(i);
			Entity* entity = CreateEntity(name);
			entity->SetPosition({ x, y, z });
			entity->Rotate(Vector3(rx, ry, rz));
			entity->SetModel(ship);
			entity->AddColliderFromModel(COLLIDER_TYPE::COLLIDER_TYPE_CONVEX_HULL);
			entity->GetRigidbody().SetMass(0.0f);
			m_PhysicsWorld.AddToBroadPhase(m_CurrentScene, entity);
		}
	}
		break;
	case WORLD_EXAMPLE_SCENE_COUNT:
	{

	}
		break;
	default:
	{

	}
		break;
	}

	m_PendingSceneChange = false;
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
	m_EntityMap.insert({ entity->GetID(), entity });
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
	if (m_PendingSceneChange)
	{
		return;
	}

	for (auto& itr : m_EntityMap)
	{
		if (itr.second != nullptr)
		{
			itr.second->FixedUpdate();
		}
	}

	m_PhysicsWorld.FixedUpdate();
}

void World::Update(const double& deltaTime)
{
	if (m_PendingSceneChange)
	{
		return;
	}

	m_PhysicsWorld.Update(deltaTime);

	for (auto& entity : m_EntityMap)
	{
		if (entity.second != nullptr)
		{
			entity.second->Update(deltaTime);
		}
	}
}

void World::OnIMGUIRender()
{
	ImGui::Begin("World");

	//bool perspective = ServiceLocator::Locate<Renderer>()->IsCameraPerpsective();
	//if (ImGui::Checkbox("Camera Perspective", &perspective))
	//{
	//	ServiceLocator::Locate<Renderer>()->SetIsCameraPerspective(perspective);
	//}

	ImGui::Text("Current Demo Scene: %s\n", c_WorldExampleSceneNames[(int)m_CurrentScene].c_str());

	if (ImGui::Button("Create New Entity", ImVec2(-FLT_MIN, 0)))
	{
		CreateEntity();
	}

	if (ImGui::CollapsingHeader("Entities"))
	{
		for (auto& entity : m_EntityMap)
		{
			if (entity.second != nullptr)
			{
				RenderEntityDetails(*entity.second);
			}
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
	
	std::string controlStr = "Take Control of Entity Entity" + imguiHash;
	if (ImGui::Button(controlStr.c_str()))
	{
		ControlledEntity = &entity;
	}

	std::string EntityEditRigidbodyModalStr = "EntityEditRigidbody#" + imguiHash;

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
			entity.GetRigidbody().Translation = { 0.0f, 0.0f, 0.0f };
		}

		ImGui::InputFloat4("Rotation (Quat)", &entity.GetRigidbody().Rotation.x);
		ImGui::SameLine();

		if (ImGui::Button("Reset Rotation"))
		{
			entity.GetRigidbody().Rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
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
				if (ImGui::SliderFloat("Sphere Radius\n", &sphereColliderRadius, 0.1f, 10.0f))
				{
					collider.SetSize(Vector3(sphereColliderRadius, sphereColliderRadius, sphereColliderRadius));
				}
			}
			break;

			case COLLIDER_TYPE_OBB:
			case COLLIDER_TYPE::COLLIDER_TYPE_AABB:
			{
				float boxSize[3] = { collider.GetSize().x, collider.GetSize().y, collider.GetSize().z };
				if (ImGui::DragFloat3("Box Extents (half-size)", &boxSize[0], 1.0f, 0.1f, 10.0f, "%.1f", ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp))
				{
					collider.SetSize({ boxSize[0], boxSize[1], boxSize[2] });
				}
			}
			break;

			case COLLIDER_TYPE_CAPSULE:
			{
				float capsuleSize[3] = { collider.GetSize().x, collider.GetSize().y, collider.GetSize().z };
				if (ImGui::DragFloat("Cylinder Radius", &capsuleSize[0], 1.0f, 0.1f, 10.0f, "%.1f", ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp))
				{
					collider.SetSize({ capsuleSize[0], capsuleSize[1], capsuleSize[2] });
				}

				if (ImGui::DragFloat("Cylinder Height", &capsuleSize[1], 1.0f, 0.1f, 10.0f, "%.1f", ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp))
				{
					collider.SetSize({ capsuleSize[0], capsuleSize[1], capsuleSize[2] });
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
	m_PhysicsWorld.Render(renderer);

	for (auto& entity : m_EntityMap)
	{
		if (entity.second != nullptr)
		{
			entity.second->Render(renderer);
		}
	}
}

const WORLD_EXAMPLE_SCENE& World::GetCurrentScene() const
{
	return m_CurrentScene;
}

std::string World::GetExtraDetails()
{
	return m_PhysicsWorld.GetExtraDetailsString();
}