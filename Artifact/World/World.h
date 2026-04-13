#pragma once
#include <unordered_map>

class Entity;
class Octree;

class World
{
private:
	typedef std::unordered_map<EntityID, Entity*> EntityMap;

	World();
	~World();

	bool m_IsInitialised;
	EntityMap m_EntityMap;
	Octree* m_Octree;

	bool Initialise();
	bool Shutdown();

public:
	const bool& IsInitialised() const;

	static World* CreateWorld();
	static bool DestroyWorld(World* world);

	void Update(const float& deltaTime);
	void PostUpdate(const float& deltaTime);
	void Render();
};

