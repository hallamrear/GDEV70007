#include "pch.h"
#include "Octree.h"
#include <World/Entity.h>
#include <Rendering/Renderer.h>

OctreeNode::OctreeNode(OctreeNode* parent, const float colliderSize, const int depth) : m_Depth(depth), m_Parent(parent)
{
	m_Entity = new Entity();
	m_Entity->SetDisplayName("Collider Node");

	if (m_Parent != nullptr)
	{
		Vector3 position = parent->m_Entity->GetPosition();
		position.x += (colliderSize / 2.0f);
		position.y += (colliderSize / 2.0f);
		position.z += (colliderSize / 2.0f);
		m_Entity->Translate(position);
	}

	m_Collider = new AABBCollider(*m_Entity, Vector3(colliderSize, colliderSize, colliderSize));

	for (size_t i = 0; i < c_ChildCount; i++)
	{
		m_Children[i] = nullptr;
	}

	m_HasSplit = false;
}

OctreeNode::~OctreeNode()
{
	for (size_t i = 0; i < c_ChildCount; i++)
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

	if (m_Entity)
	{
		delete m_Entity;
		m_Entity = nullptr;
	}
}

const bool& OctreeNode::HasSplit() const
{
	return m_HasSplit;
}

void OctreeNode::AddEntity(const std::vector<Entity*>& entitiesToAdd)
{
	if ((m_Depth + 1) >= c_MaxOctreeNodeDepth)
	{
		m_Bucket.insert(m_Bucket.end(), entitiesToAdd.begin(), entitiesToAdd.end());
		return;
	}

	if (m_Bucket.size() >= c_MaxBucketCapacity)
	{
		if (SplitNode(m_Bucket))
		{
			m_Bucket.clear();
		}
	}
}

bool OctreeNode::SplitNode(const std::vector<Entity*>& entitiesToSort)
{
	UNREFERENCED_PARAMETER(entitiesToSort);

	if ((m_Depth + 1) >= c_MaxOctreeNodeDepth)
	{
		//m_Bucket.insert(m_Bucket.end(), entitiesToSort.begin(), entitiesToSort.end());
		return false;
	}

	return true;
}

void OctreeNode::Render(Renderer& renderer)
{
	if (HasSplit())
	{
		for (size_t i = 0; i < c_ChildCount; i++)
		{
			if (m_Children[i] != nullptr) 
			{
				m_Children[i]->Render(renderer);
			}
		}
	}
	else
	{
		m_Collider->Render(renderer);
	}
}