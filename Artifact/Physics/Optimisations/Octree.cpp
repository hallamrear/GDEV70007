#include "pch.h"
#include "Octree.h"

OctreeNode::OctreeNode(const float colliderSize, const int depth) : m_Depth(depth)
{

}

OctreeNode::~OctreeNode()
{
	for (size_t i = 0; i < OCTREE_CHILD_COUNT; i++)
	{
		if (m_Children[i] != nullptr)
		{
			delete m_Children[i];
			m_Children[i] = nullptr;
		}
	}

	if (m_Collider)
	{
		delete m_Collider;
		m_Collider = nullptr;
	}
}

const bool& OctreeNode::HasSplit() const
{
	return m_HasSplit;
}

void OctreeNode::SplitNode(const std::vector<Entity*>& entitiesToSort)
{
	if ((m_Depth + 1) >= OCTREE_MAX_NODE_DEPTH)
	{
		m_Bucket.insert(m_Bucket.end(), entitiesToSort.begin(), entitiesToSort.end());
		return;
	}
}