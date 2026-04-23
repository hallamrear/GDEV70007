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
	ModelRef ref = ServiceLocator::Locate<AssetManager>()->GetModel("Content\\Demo_Level.glb");
	testRoom->SetModel(ref);
	m_EntityMap.insert(std::make_pair(testRoom->GetID(), testRoom));

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

	if (ImGui::CollapsingHeader(headerHash.c_str()))
	{
		ImGui::Text("GUID: %s", entity.GetIDString().c_str());
		ImGui::SameLine();
		if (ImGui::Button("Delete Entity"))
		{
			entity.Destroy();
		}

		char buffer[MAX_DISPLAY_NAME_SIZE] = { 0 };
		strcpy_s(&buffer[0], MAX_DISPLAY_NAME_SIZE, entity.GetDisplayName().c_str());
		if (ImGui::InputText("Display Name: ", &buffer[0], MAX_DISPLAY_NAME_SIZE))
		{
			entity.SetDisplayName(buffer);
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
			const char* colliderNames[COLLIDER_TYPE::COLLIDER_TYPE_COUNT] =
			{
				/* COLLIDER_TYPE_SPHERE */ "Sphere Collider",
				/* COLLIDER_TYPE_AABB */ "Box Collider",
				/* COLLIDER_TYPE_MESH */ "Complex Mesh Collider",

			};

			const ImGuiComboFlags flags = 0;

			static COLLIDER_TYPE selectedCollider = COLLIDER_TYPE::COLLIDER_TYPE_SPHERE;

			if (ImGui::BeginCombo("Collider Type List", colliderNames[selectedCollider], flags))
			{
				for (int n = 0; n < IM_COUNTOF(colliderNames); n++)
				{
					const bool is_selected = ((int)selectedCollider == n);
					if (ImGui::Selectable(colliderNames[n], is_selected))
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