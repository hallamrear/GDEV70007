#pragma once

class Entity;

enum BROADPHASE_DRAW_MODE
{
	BROADPHASE_DRAW_NONE = 0,
	BROADPHASE_DRAW_PAIRS = 1,
	BROADPHASE_DRAW_ALL = 2,
	BROADPHASE_DRAW_MODE_COUNT = 3,
};

static const std::string c_BroadphaseDrawModeNames[BROADPHASE_DRAW_MODE::BROADPHASE_DRAW_MODE_COUNT] =
{
	"Drawing no cells",
	"Drawing only cells with pairs",
	"Drawing all cells",
};


class BroadPhase
{
protected:
	static BROADPHASE_DRAW_MODE m_DrawMode;
	int m_EntityCount;
	int m_CollisionPairsThisFrame;

public:
	virtual ~BroadPhase() = 0;

	virtual bool DetermineCollisionPairs(std::vector<std::pair<Entity*, Entity*>>& collisionPairs) = 0;
	virtual bool AddObject(Entity* entity) = 0;
	virtual void RenderIMGUIDetails() = 0;
	virtual void Render(Renderer& renderer) = 0;
	virtual std::string GetBroadPhaseStatsString() = 0;
};

