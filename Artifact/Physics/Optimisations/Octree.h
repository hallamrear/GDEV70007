#pragma once
#include <Physics/Colliders/AABBCollider.h>

#define OCTREE_CHILD_COUNT 8
#define OCTREE_MAX_BUCKET_CAPACITY 4
#define OCTREE_MAX_NODE_DEPTH 16

class OctreeNode
{
private:
	bool m_HasSplit;
	const int m_Depth;
	OctreeNode* m_Children[OCTREE_CHILD_COUNT];
	AABBCollider* m_Collider;
	std::vector<Entity*> m_Bucket;
	void SplitNode(const std::vector<Entity*>& entitiesToSort);

public:
	OctreeNode(const float colliderSize, const int depth);
	~OctreeNode();

	const bool& HasSplit() const;
	void AddEntity(const std::vector<Entity*>& entitiesToAdd);
};

