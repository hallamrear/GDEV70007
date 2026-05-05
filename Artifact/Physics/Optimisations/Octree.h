#pragma once
#include <Physics/Colliders/AABBCollider.h>
#include <vector>

class Renderer;

class OctreeNode
{
private:
	static constexpr int c_ChildCount = 8;
	static constexpr int c_MaxBucketCapacity = 4;
	static constexpr int c_MaxOctreeNodeDepth = 16;

	OctreeNode* m_Parent;
	Entity* m_Entity;
	bool m_HasSplit;
	const int m_Depth;
	OctreeNode* m_Children[c_ChildCount];
	AABBCollider* m_Collider;
	std::vector<Entity*> m_Bucket;
	bool SplitNode(const std::vector<Entity*>& entitiesToSort);

public:
	OctreeNode(OctreeNode* parent, const float colliderSize, const int depth);
	~OctreeNode();

	const bool& HasSplit() const;
	void AddEntity(const std::vector<Entity*>& entitiesToAdd);
	void Render(Renderer& renderer);
};

