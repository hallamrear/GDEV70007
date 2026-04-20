#pragma once
#include <unordered_map>
#include <rpc.h>

class Entity;
class Octree;

struct GUIDHasher
{
	std::size_t operator()(const EntityID& guid) const
	{
		RPC_STATUS status = {};
		return UuidHash((GUID*)&guid, &status);
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

	void RenderEntityDetails(Entity& entity);

public:
	const bool& IsInitialised() const;

	static World* CreateWorld();
	static bool DestroyWorld(World* world);

	Entity* CreateEntity(const std::string& displayName);

	void Update(const float& deltaTime);
	void PostUpdate(const float& deltaTime);
	void IMGUIRender();
	void Render(Renderer& renderer);
};

