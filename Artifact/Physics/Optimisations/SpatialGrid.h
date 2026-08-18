#pragma once
#include <Physics/Optimisations/BroadPhase.h>
#include <unordered_map>
#include <glm/glm.hpp>
#include <Rendering/Geometry/Model.h>

struct GridIndex
{
public:
	int X = INT_MAX;
	int Y = INT_MAX;
	int Z = INT_MAX;

	bool operator==(const GridIndex& rhs) const
	{
		return ((X == rhs.X) && (Y == rhs.Y) && (Z == rhs.Z));
	}
};

struct IndexHasher
{
	size_t operator()(const GridIndex& index) const
	{
		// Combine hashes of x and y using the bitwise XOR
		return std::hash<int>()(index.X) ^ (std::hash<int>()(index.Y) << 1) ^ (std::hash<int>()(index.Z) << 2);
	}
};

typedef std::unordered_map<GridIndex, std::vector<Entity*>, IndexHasher> GridStorageType;

class SpatialGrid : public BroadPhase
{
private:
	static constexpr float c_GridCellSize = 16.0f;
	static ModelRef m_GridCellModel;
	int m_CollisionPairsThisFrame;
	GridStorageType m_GridMap;
	GridIndex GetIndexFromPosition(const glm::vec3& position);

public:
	SpatialGrid();
	~SpatialGrid();

	bool DetermineCollisionPairs(std::vector<std::pair<Entity*, Entity*>>& collisionPairs) override;
	bool AddObject(Entity* entity) override;
	void RenderIMGUIDetails() override;
	void Render(Renderer& renderer) override;
};

