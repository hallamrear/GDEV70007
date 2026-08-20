#pragma once
#include <glm/glm.hpp>
#include <Physics/Optimisations/BroadPhase.h>
#include <deque>

class Entity;

struct Interval
{
	float Value;
	bool IsMin;
	const Entity* Object;
};

class SweepAndPrune : public BroadPhase
{
private:
	static constexpr int c_TestAxisCount = 3;

	const glm::vec3 c_TestAxes[c_TestAxisCount] =
	{
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
	};
	
	std::deque<Interval> m_Intervals;
	bool m_NeedsUpdate;

	void InsertionSort();

public:
	SweepAndPrune();
	~SweepAndPrune();

	bool DetermineCollisionPairs(std::vector<std::pair<Entity*, Entity*>>& collisionPairs) override;
	bool AddObject(Entity* entity) override;
	void RenderIMGUIDetails() override;
	void Render(Renderer& renderer) override;

	std::string GetBroadPhaseStatsString() override;
};

