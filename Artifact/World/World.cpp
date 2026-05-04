#include "pch.h"
#include "World/World.h"
#include "World/Entity.h"
#include <System/ServiceLocator.h>
#include <System/AssetManagement.h>
#include <Rendering/IMGUIIncludes.h>
#include <Physics/Colliders/Collider.h>

const bool& World::IsInitialised() const
{
	return m_IsInitialised;
}

World::World()
{
	m_EntityMap = EntityMap();
	m_IsInitialised = false;
	m_Octree = nullptr;
}

World::~World()
{
	assert(m_IsInitialised);
}

bool World::Initialise()
{
	Entity* testRoom = CreateEntity("Test Room");
	ModelRef ref = ServiceLocator::Locate<AssetManager>()->GetModel("Demo_Level.glb");
	testRoom->SetModel(ref);
	m_EntityMap.insert(std::make_pair(testRoom->GetID(), testRoom));

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
	return entity;
}

Entity* World::CreateEntity(const std::string& displayName)
{
	Entity* entity = CreateEntity();
	entity->SetDisplayName(displayName);
	return entity;
}

void World::Update(const float& deltaTime)
{
	for (auto& itr : m_EntityMap)
	{
		if (itr.second != nullptr)
		{
			itr.second->Update(deltaTime);
		}
	}

	DestroyDeadEntities();
}

void World::PostUpdate(const float& deltaTime)
{
	for (auto& entity : m_EntityMap)
	{
		if (entity.second != nullptr)
		{
			entity.second->PostUpdate(deltaTime);
		}
	}
}

void World::OnIMGUIRender()
{
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

	ImGui::Text(entity.GetDisplayName().c_str()); 
	ImGui::SameLine(); 

	std::string str = "EntityEditModal" + imguiHash;
	if(ImGui::Button("Edit Entity"))
	{
		ImGui::OpenPopup(str.c_str());
	}

	if (ImGui::BeginPopupModal(str.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
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

		std::string modelDisplayName = "None";

		if (entity.GetModel().get() != nullptr)
		{
			modelDisplayName = entity.GetModel()->GetDisplayName();
		}

		ImGui::Text("Model: %s\n", modelDisplayName.c_str());
		ImGui::SameLine();

		if (ImGui::Button("Change Model"))
		{
			ImGui::OpenPopup("Change Model Modal");
		}

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
					entity.SetColliderFromModel(entity.GetCollider()->GetType());
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
				entity.SetCollider(selectedCollider);
			}
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
}