#pragma once
#include <Rendering/IMGUIRenderable.h>
#include <Physics/PhysicsWorld.h>
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

enum WORLD_EXAMPLE_SCENE : int
{
	WORLD_EMPTY_SCENE = 0,
	WORLD_COLLIDER_EXAMPLE = 1,
	WORLD_SAT_EXAMPLE = 2,
	WORLD_GJK_EXAMPLE = 3,
	WORLD_SPATIAL_GRID = 4,
	WORLD_SWEEP_AND_PRUNE = 5,
	WORLD_OCTREE = 6,
	WORLD_GJK_EXAMPLE_SPATIAL_GRID = 7,
	WORLD_EXAMPLE_SCENE_COUNT = 8
};

static const std::string c_WorldExampleSceneNames[WORLD_EXAMPLE_SCENE_COUNT] =
{
	"Empty Scene",
	"Collider Demo",
	"Separating Axis Theorem",
	"GJK + EPA",
	"Spatial Grid",
	"Sweep and Prune",
	"Octree",
	"Convex Hulls in Spatial Grid"
};

class World : public IIMGUIRenderable
{
private:
	WORLD_EXAMPLE_SCENE m_CurrentScene;
	bool m_PendingSceneChange;
	typedef std::unordered_map<EntityID, Entity*, GUIDHasher> EntityMap;

	PhysicsWorld m_PhysicsWorld;

	World();
	~World();

	void ChangeExampleScene(const WORLD_EXAMPLE_SCENE& newScene);

	bool m_IsInitialised;
	EntityMap m_EntityMap;
	
	bool Initialise();
	bool Shutdown();

	void RenderEntityDetails(Entity& entity);
	void DestroyDeadEntities();
	Camera* m_Camera;

public:
	static const int c_TestCount;
	static Entity* ControlledEntity;

	const bool& IsInitialised() const;

	static World* CreateWorld();
	static bool DestroyWorld(World* world);

	Entity* CreateEntity(const std::string& displayName);
	Entity* CreateEntity();
	Entity* GetEntity(const EntityID& entityID);

	void FixedUpdate();
	void Update(const double& deltaTime);
	void Render(Renderer& renderer);

	const WORLD_EXAMPLE_SCENE& GetCurrentScene() const;

	std::string GetExtraDetails();

	// Inherited via IIMGUIRenderable
	void OnIMGUIRender() override;
};

