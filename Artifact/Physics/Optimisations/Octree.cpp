#include "pch.h"
#include "Octree.h"
#include <World/Entity.h>
#include <Rendering/Renderer.h>
#include <System/ServiceLocator.h>
#include <System/AssetManagement.h>

ModelRef OctreeNode::m_Model = nullptr;

OctreeNode* OctreeNode::BuildOctree(OctreeNode* parent, const Vector3& centre, const float& halfWidth, const int& depth)
{
	OctreeNode* node = nullptr;

	if (depth >= c_MaxOctreeNodeDepth)
	{
		return node;
	}
	else
	{
		if (m_Model == nullptr)
		{
			AssetManager* am = ServiceLocator::Locate<AssetManager>();
			assert(am);
			m_Model = am->GetModel("Colliders\\BoxCollider.glb");
			assert(m_Model);
		}

		node = new OctreeNode(parent, centre, halfWidth, depth);
	}

	return node;
}

void OctreeNode::DestroyOctree(OctreeNode* root)
{
	UNREFERENCED_PARAMETER(root);
	throw new std::exception("Not Implemented");
}

OctreeNode::OctreeNode(OctreeNode* parent, const Vector3& centre, const float& halfWidth, const int& depth) : m_Depth(depth), m_Parent(parent)
{
	m_Centre = centre;
	m_CentreMatrix = IdentityMatrix;

	DirectX::XMStoreFloat4x4(&m_CentreMatrix,
		DirectX::XMMatrixScaling(halfWidth + halfWidth, halfWidth + halfWidth, halfWidth + halfWidth) *
		DirectX::XMMatrixTranslation(centre.x, centre.y, centre.z)
	);

	m_HalfWidth = halfWidth;

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
}

const bool& OctreeNode::HasSplit() const
{
	return m_HasSplit;
}

void OctreeNode::AddEntity(const std::vector<Entity*>& entitiesToAdd)
{
	UNREFERENCED_PARAMETER(entitiesToAdd);
}

bool OctreeNode::SplitNode(const std::vector<Entity*>& entitiesToSort)
{
	UNREFERENCED_PARAMETER(entitiesToSort);

	Vector3 offset = Vector3(0.0f, 0.0f, 0.0f);

	float step = m_HalfWidth * 0.5f;

	Vector3 childCentre = Vector3(0.0f, 0.0f, 0.0f);
	for (size_t i = 0; i < c_ChildCount; i++)
	{
		offset.x = ((i & 1) ? step : -step);
		offset.y = ((i & 2) ? step : -step);
		offset.z = ((i & 4) ? step : -step);

		childCentre.x = m_Centre.x + offset.x;
		childCentre.y = m_Centre.y + offset.y;
		childCentre.z = m_Centre.z + offset.z;
		m_Children[i] = BuildOctree(this, childCentre, step, m_Depth + 1);
	}

	for (size_t i = 0; i < c_ChildCount; i++)
	{

	}


	return true;
}

void OctreeNode::Render(Renderer& renderer, OctreeNode* root)
{
	renderer.SetDebugDrawMode();

	if (root->HasSplit())
	{
		for (size_t i = 0; i < c_ChildCount; i++)
		{
			if (root->m_Children[i] != nullptr)
			{
				OctreeNode::Render(renderer, root->m_Children[i]);
			}
		}
	}
	else
	{
		renderer.Render(m_Model, root->m_CentreMatrix);
	}
}

void OctreeNode::RenderIMGUIDetails()
{

}