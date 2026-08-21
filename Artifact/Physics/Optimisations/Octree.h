#pragma once
#include <Physics/Colliders/AABBCollider.h>
#include <Physics/Optimisations/BroadPhase.h>
#include <vector>

class Renderer;

class OctreeNode : public BroadPhase
{
private:
	static constexpr int c_ChildCount = 8;
	static constexpr int c_MaxBucketCapacity = 8;
	static constexpr int c_MaxOctreeNodeDepth = 5;

	static ModelRef m_Model;
	float m_HalfWidth;
	Matrix4x4 m_CentreMatrix;
	glm::vec3 m_Centre;
	glm::vec3 m_AABBMin;
	glm::vec3 m_AABBMax;

	OctreeNode* m_Parent;
	bool m_HasSplit;
	const int m_Depth;
	OctreeNode* m_Children[c_ChildCount];
	std::vector<Entity*> m_Bucket;
	bool SplitNode();

	OctreeNode(OctreeNode* parent, const glm::vec3& centre, const float& colliderSize, const int& depth);

public:
	~OctreeNode();

	const bool& HasSplit() const;
	static OctreeNode* BuildOctree(OctreeNode* parent, const glm::vec3& centre, const float& halfWidth, const int& depth);
	static void DestroyOctree(OctreeNode* root);

	bool DetermineCollisionPairs(std::vector<std::pair<Entity*, Entity*>>& collisionPairs) override;
	bool AddObject(Entity* entity) override;
	void RenderIMGUIDetails() override;
	void Render(Renderer& renderer) override;
	virtual std::string GetBroadPhaseStatsString() override;
};

