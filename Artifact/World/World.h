#pragma once
#include <unordered_map>

class Entity;
class Octree;

struct GUIDHasher
{
	std::size_t operator()(const EntityID& guid) const
	{
		return UuidHash((GUID*)&guid, nullptr);
	}
};

class World
{
private:
	typedef std::unordered_map<EntityID, Entity*, GUIDHasher> EntityMap;

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

