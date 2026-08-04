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
Entity* World::TestBoxA = nullptr;
Entity* World::TestBoxB = nullptr;
bool loadDemoScene = false;
bool isMoving = true;

std::string modelList[] =
{
	"LongBox.glb",
	"LongBox.glb",
	"LongBox.glb",
	"LongBox.glb",
	"LongBox.glb",
	"LongBox.glb",
	"LongBox.glb",
};

Vector3 rotations[] =
{
	Vector3(0.0f, 0.0f, 0.0f),
	Vector3(0.0f, 0.0f, 45.0f),
	Vector3(45.0f, 0.0f, 0.0f),
	Vector3(45.0f, 0.0f, 0.0f),
	Vector3(45.0f, 0.0f, 45.0f),
};

float modelSpacing = 15.0f;
float timeToCover = 20.0f;
float timeElapsed = 0.0f;

const bool& World::IsInitialised() const
{
	return m_IsInitialised;
}

World::World()
{
	m_EntityMap = EntityMap();
	m_IsInitialised = false;
	m_OctreeRoot = nullptr;
	m_Camera = &ServiceLocator::Locate<Renderer>()->GetCamera();
}

World::~World()
{
	assert(m_IsInitialised);
}

std::vector<ModelRef> modelStorage;
bool World::Initialise()
{
	if (loadDemoScene)
	{
		TestBoxB = CreateEntity("Tracking Box");
		TestBoxB->AddCollider(COLLIDER_TYPE::COLLIDER_TYPE_CONVEX_HULL);
		TestBoxB->SetPosition(Vector3(0.0f, 0.0f, -10.0f));
		ModelRef suzaane = ServiceLocator::Locate<AssetManager>()->GetModel("Colliders/BoxCollider.glb");
		TestBoxB->SetModel(suzaane);

		TestBoxA = CreateEntity("Follow Box");
		TestBoxA->SetPosition(Vector3(0.0f, 0.0f, -15.0f));
		ModelRef barrel = ServiceLocator::Locate<AssetManager>()->GetModel("Barrel.glb");
		TestBoxA->SetModel(barrel);
		TestBoxA->AddCollider(COLLIDER_TYPE::COLLIDER_TYPE_CONVEX_HULL);

		for (size_t i = 0; i < _countof(modelList); i++)
		{
			Entity* entity = CreateEntity("Test Model");
			entity->SetPosition(Vector3(i * modelSpacing, 0.0f, 0.0f));

			float r_x = (rand() % 100) / 100.0f;
			float r_y = (rand() % 100) / 100.0f;
			float r_z = (rand() % 100) / 100.0f;

			entity->Rotate(Vector3(r_x * 360.0f, r_y * 360.0f, r_z * 360.0f));
			ModelRef model = ServiceLocator::Locate<AssetManager>()->GetModel(modelList[i]);
			entity->SetModel(model);
		}

		return true;
	}

	Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
	//m_OctreeRoot = OctreeNode::BuildOctree(nullptr, Vector3(0.0f, 0.0f, 0.0f), 8192.0f, 0);

	//Entity* testRoom = CreateEntity("Test Room");
	//ModelRef ref = ServiceLocator::Locate<AssetManager>()->GetModel("Demo_Level.glb");
	//testRoom->SetModel(ref);
	//testRoom->SetPosition(Vector3(-60.0f, 0.0f, 0.0f));

	//Entity* AAA = CreateEntity("AAA");
	//ModelRef stacking = ServiceLocator::Locate<AssetManager>()->GetModel("Colliders/BoxCollider.glb");
	//AAA->SetModel(stacking);
	//AAA->SetPosition(Vector3(-60.0f, 0.0f, 0.0f));
	//	
	//auto c = CreateEntity("Cone");
	//c->AddCollider(COLLIDER_TYPE::COLLIDER_TYPE_AABB);
	//ModelRef cone = ServiceLocator::Locate<AssetManager>()->GetModel("Barrel.glb");
	//c->SetModel(cone);
	//c->SetPosition(Vector3(-40.0f, 0.0f, 0.0f));
	
	//TestBoxA = CreateEntity("Box A");
	////TestBoxA->GetCollider()->SetSize(Vector3(10.0f, 10.0f, 10.0f));
	//TestBoxA->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
	////ModelRef suzaane = ServiceLocator::Locate<AssetManager>()->GetModel("TestCone.glb");
	//ModelRef suzaane = ServiceLocator::Locate<AssetManager>()->GetModel("Colliders/BoxCollider.glb");
	//TestBoxA->SetModel(suzaane);
	//TestBoxA->AddCollider(COLLIDER_TYPE::COLLIDER_TYPE_CONVEX_HULL);

	TestBoxA = CreateEntity("Follow Box");
	TestBoxA->SetPosition(Vector3(0.0f, 2.0f, 0.0f));
	ModelRef barrel = ServiceLocator::Locate<AssetManager>()->GetModel("Barrel.glb");
	TestBoxA->SetModel(barrel);
	TestBoxA->AddCollider(COLLIDER_TYPE::COLLIDER_TYPE_CONVEX_HULL);
	//TestBoxA->AddColliderFromModel(COLLIDER_TYPE::COLLIDER_TYPE_AABB);
	//TestBoxA->GetRigidbody().IsGravityEnabled = true;

	float r_x = (rand() % 100) / 100.0f;
	float r_y = (rand() % 100) / 100.0f;
	float r_z = (rand() % 100) / 100.0f;
	TestBoxA->Rotate(Vector3(r_x * 360.0f, r_y * 360.0f, r_z * 360.0f));

	TestBoxB = CreateEntity("Box B");
	//ModelRef test = ServiceLocator::Locate<AssetManager>()->GetModel("TestConvexHull.glb");
	ModelRef test = ServiceLocator::Locate<AssetManager>()->GetModel("Suzanne.glb");
	TestBoxB->SetModel(test);
	////TestBoxB->GetCollider()->SetSize(Vector3(7.0f, 3.0f, 5.0f));
	TestBoxB->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
	TestBoxB->AddCollider(COLLIDER_TYPE::COLLIDER_TYPE_CONVEX_HULL);
	//TestBoxB->AddColliderFromModel(COLLIDER_TYPE::COLLIDER_TYPE_AABB);
	TestBoxB->GetRigidbody().SetMass(0.0f);

	r_x = (rand() % 100) / 100.0f;
	r_y = (rand() % 100) / 100.0f;
	r_z = (rand() % 100) / 100.0f;
	TestBoxB->Rotate(Vector3(r_x * 360.0f, r_y * 360.0f, r_z * 360.0f));

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

static float m_CameraDistance = 20.0f;
void World::Update(const float& deltaTime)
{
	m_PhysicsWorld.Update(deltaTime);

	for (auto& entity : m_EntityMap)
	{
		if (entity.second != nullptr)
		{
			entity.second->Update(deltaTime);
		}
	}

	if (loadDemoScene)
	{
		const Vector3 start = { -10.0f, 0.0f, 0.0f };
		const Vector3 end = { ((float)(_countof(modelList) - 1) * modelSpacing) + 10.0f, 0.0f, 0.0f};

		if (isMoving)
		{

			timeElapsed += deltaTime;

			static bool flipped = false;

			if (timeElapsed >= timeToCover)
			{
				timeElapsed = 0.0f;
				flipped = !flipped;
			}

			float t = timeElapsed / timeToCover;
			Vector3 lerpPosition = (flipped) ? Maths::Lerp(end, start, t) : Maths::Lerp(start, end, t);

			Vector3 followBoxPosition = lerpPosition;

			Vector3 cameraPosition;
			cameraPosition.x = (lerpPosition.x + followBoxPosition.x) * 0.5f;
			cameraPosition.y = (lerpPosition.y + followBoxPosition.y) * 0.5f;
			cameraPosition.z = lerpPosition.z - m_CameraDistance;

			if (flipped)
				lerpPosition.y = (sinf(1.0f - t) * 2.0f - 0.5f) * 3.0f;
			else
				lerpPosition.y = (sinf(t) * 2.0f - 0.5f) * 3.0f;

			if (!flipped)
				followBoxPosition.y = 2.0f * (sinf(1.0f - t) * 2.0f - 0.5f) * 0.75f;
			else
				followBoxPosition.y = 2.0f * (sinf(t) * 2.0f - 0.5f) * 0.75f;

			followBoxPosition.x = lerpPosition.x - 5.0f;

			TestBoxB->SetPosition(lerpPosition);
			TestBoxA->SetPosition(followBoxPosition);

			if (m_Camera)
			{
				m_Camera->SetPosition(cameraPosition);
			}

			float r_x = ((rand() % 100) / 100.0f) * deltaTime;
			float r_y = ((rand() % 100) / 100.0f) * deltaTime;
			float r_z = ((rand() % 100) / 100.0f) * deltaTime;

			TestBoxB->Rotate(Vector3(r_x, r_y, r_z));

			r_x = ((rand() % 100) / 100.0f) * -deltaTime;
			r_y = ((rand() % 100) / 100.0f) * deltaTime;
			r_z = ((rand() % 100) / 100.0f) * -deltaTime;

			TestBoxA->Rotate(Vector3(r_x, r_y, r_z));
		}
	}

}

void World::OnIMGUIRender()
{
	ImGui::Begin("Demo");
	ImGui::Checkbox("Moving?", &isMoving);
	ImGui::End();

	ImGui::Begin("World");

	ImGui::DragFloat("Camera Distance", &m_CameraDistance, 1.0f, 5.0f, 30.0f);

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
					collider.SetSize({ boxSize[0], boxSize[1], boxSize[2] });
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