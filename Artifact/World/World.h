#pragma once
#include <Rendering/IMGUIRenderable.h>
#include <unordered_map>
#include <rpc.h>

class Entity;
class OctreeNode;

struct GUIDHasher
{
	std::size_t operator()(const EntityID& guid) const
	{
		RPC_STATUS status = {};
		return UuidHash((GUID*)&guid, &status);
	}
};

class World : public IIMGUIRenderable
{
private:
	typedef std::unordered_map<EntityID, Entity*, GUIDHasher> EntityMap;

	World();
	~World();

	bool m_IsInitialised;
	EntityMap m_EntityMap;
	OctreeNode* m_Octree;

	bool Initialise();
	bool Shutdown();

	void RenderEntityDetails(Entity& entity);
	void DestroyDeadEntities();

public:
	const bool& IsInitialised() const;

	static World* CreateWorld();
	static bool DestroyWorld(World* world);

	Entity* CreateEntity(const std::string& displayName);
	Entity* CreateEntity();
	Entity* GetEntity(const EntityID& entityID);

	void FixedUpdate();
	void Update(const float& deltaTime);
	void Render(Renderer& renderer);

	// Inherited via IIMGUIRenderable
	void OnIMGUIRender() override;
};

