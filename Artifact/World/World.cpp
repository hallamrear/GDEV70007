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
}

World::~World()
{
	assert(m_IsInitialised);
}

bool World::Initialise()
{
	for (size_t i = 0; i < 10; i++)
	{
		Entity* entity = CreateEntity("Entity: " + std::to_string(i));

		Matrix4x4 translation;
		DirectX::XMStoreFloat4x4(&translation, DirectX::XMMatrixTranslation(i * -5.0f, 0.0f, 0.0f));
		entity->SetWorldMatrix(translation);

		m_EntityMap.insert(std::make_pair(entity->GetID(), entity));
	}


	Entity* testRoom = CreateEntity("Test Room");
	ModelRef ref = ServiceLocator::Locate<AssetManager>()->GetModel("Demo_Level.glb");
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

Entity* World::CreateEntity(const std::string& displayName)
{
	Entity* entity = new Entity();
	entity->SetDisplayName(displayName);
	ModelRef ref = ServiceLocator::Locate<AssetManager>()->GetModel("Barrel.glb");
	entity->SetModel(ref);

	return entity;
}

void World::Update(const float& deltaTime)
{
	for (auto& entity : m_EntityMap)
	{
		if (entity.second != nullptr)
		{
			entity.second->Update(deltaTime);
		}
	}
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

void World::IMGUIRender()
{
	ImGui::Begin("World");
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
			case Collider::COLLIDER_TYPE_SPHERE:
			{
				float sphereColliderRadius = { collider.GetSize().x };
				if (ImGui::SliderFloat("Sphere collider size: ", &sphereColliderRadius, 0.1f, 10.0f))
				{
					collider.SetSize(Vector3(sphereColliderRadius, sphereColliderRadius, sphereColliderRadius));
				}
			}
			break;

			default:
				break;
			}
		}
	}

	ImGui::PopID();
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
