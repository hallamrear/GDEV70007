#pragma once

class Entity;

class BroadPhase
{
private:

public:
	virtual ~BroadPhase() = 0;

	virtual bool DetermineCollisionPairs(std::vector<std::pair<Entity*, Entity*>>& collisionPairs) = 0;
	virtual bool AddObject(Entity* entity) = 0;
	virtual void RenderIMGUIDetails() = 0;
	virtual void Render(Renderer& renderer) = 0;
	virtual std::string GetBroadPhaseStatsString() = 0;
};

