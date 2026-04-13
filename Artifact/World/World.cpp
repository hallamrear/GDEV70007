#include "pch.h"
#include "World/World.h"
#include "World/Entity.h"

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
		Entity* entity = new Entity();
		entity->SetDisplayName("Entity: " + std::to_string(i));
	}

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

void World::Render()
{
	for (auto& entity : m_EntityMap)
	{
		if (entity.second != nullptr)
		{
			entity.second->Render();
		}
	}
}
