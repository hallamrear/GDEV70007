#include "pch.h"
#include "Octree.h"
#include <World/Entity.h>
#include <Rendering/Renderer.h>
#include <System/ServiceLocator.h>
#include <System/AssetManagement.h>
#include <Physics/Collision Detection/CollisionDetection.h>
#include <World/World.h>

ModelRef OctreeNode::m_Model = nullptr;

OctreeNode* OctreeNode::BuildOctree(OctreeNode* parent, const glm::vec3& centre, const float& halfWidth, const int& depth)
{
	OctreeNode* node = nullptr;

	if (depth >= c_MaxOctreeNodeDepth + 1)
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

bool OctreeNode::DetermineCollisionPairs(std::vector<std::pair<Entity*, Entity*>>& collisionPairs)
{
	if (m_HasSplit)
	{
		for (size_t i = 0; i < c_ChildCount; i++)
		{
			m_Children[i]->DetermineCollisionPairs(collisionPairs);
		}
	}
	else
	{
		for (size_t i = 0; i < m_Bucket.size(); i++)
		{
			for (size_t j = i + 1; j < m_Bucket.size(); j++)
			{
				collisionPairs.push_back(std::make_pair(m_Bucket[i], m_Bucket[j]));
			}
		}
	}

	if (m_Depth == 0)
	{
		std::sort(collisionPairs.begin(), collisionPairs.end());
		auto last = std::unique(collisionPairs.begin(), collisionPairs.end());
		collisionPairs.erase(last, collisionPairs.end());
		m_CollisionPairsThisFrame = (int)collisionPairs.size();
	}

	return true;
}

bool OctreeNode::AddObject(Entity* entity)
{
	if (entity->GetCollider() == nullptr)
	{
		return false;
	}

	if (m_HasSplit)
	{
		glm::vec3 entityAABBMax	= { 0.0f, 0.0f, 0.0f };
		glm::vec3 entityAABBMin = { 0.0f, 0.0f, 0.0f };

		entityAABBMax =
		{
			entity->GetPosition().x + entity->GetCollider()->GetBoundingVolumeExtents().x,
			entity->GetPosition().y + entity->GetCollider()->GetBoundingVolumeExtents().y,
			entity->GetPosition().z + entity->GetCollider()->GetBoundingVolumeExtents().z
		};

		entityAABBMin =
		{
			entity->GetPosition().x - entity->GetCollider()->GetBoundingVolumeExtents().x,
			entity->GetPosition().y - entity->GetCollider()->GetBoundingVolumeExtents().y,
			entity->GetPosition().z - entity->GetCollider()->GetBoundingVolumeExtents().z
		};

		for (size_t i = 0; i < c_ChildCount; i++)
		{
			if (CollisionDetection::AABBAABBCollision(m_Children[i]->m_AABBMin, m_Children[i]->m_AABBMax, entityAABBMin, entityAABBMax))
			{
				m_Children[i]->AddObject(entity);
			}
		}
	}
	else
	{
		m_Bucket.push_back(entity);

		if (m_Bucket.size() > c_MaxBucketCapacity && m_Depth != c_MaxOctreeNodeDepth)
		{
			if (m_HasSplit == false)
			{
				SplitNode();
				m_Bucket.clear();
			}
		}
	}

	return true;
}

OctreeNode::OctreeNode(OctreeNode* parent, const glm::vec3& centre, const float& halfWidth, const int& depth) : m_Depth(depth), m_Parent(parent)
{
	m_Centre = centre;
	m_CentreMatrix = IdentityMatrix;

	DirectX::XMStoreFloat4x4(&m_CentreMatrix,
		DirectX::XMMatrixScaling(halfWidth + halfWidth, halfWidth + halfWidth, halfWidth + halfWidth) *
		DirectX::XMMatrixTranslation(centre.x, centre.y, centre.z)
	);

	m_HalfWidth = halfWidth;

	for (int i = 0; i < 3; i++)
	{
		m_AABBMax[i] = { m_Centre[i] + m_HalfWidth };
		m_AABBMin[i] = { m_Centre[i] - m_HalfWidth };
	}

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

bool OctreeNode::SplitNode()
{
	if (HasSplit())
	{
		return false;
	}

	Vector3 offset = Vector3(0.0f, 0.0f, 0.0f);

	float step = m_HalfWidth * 0.5f;

	glm::vec3 childCentre = glm::vec3(0.0f, 0.0f, 0.0f);

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

	m_HasSplit = true;
	for (size_t b = 0; b < m_Bucket.size(); b++)
	{
		for (size_t i = 0; i < c_ChildCount; i++)
		{
			m_Children[i]->AddObject(m_Bucket[b]);
		}
	}

	return true;
}

void OctreeNode::RenderIMGUIDetails()
{
	const ImGuiComboFlags flags = 0;

	if (ImGui::BeginCombo("Octree Draw Mode", c_BroadphaseDrawModeNames[m_DrawMode].c_str(), flags))
	{
		for (int n = 0; n < IM_COUNTOF(c_BroadphaseDrawModeNames); n++)
		{
			const bool is_selected = ((int)m_DrawMode == n);
			if (ImGui::Selectable(c_BroadphaseDrawModeNames[n].c_str(), is_selected))
			{
				m_DrawMode = (BROADPHASE_DRAW_MODE)n;
			}

			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::Text("%i max bucket size before split.\n", c_MaxBucketCapacity);
	ImGui::Text("%i max octree node depth.\n", c_MaxOctreeNodeDepth);
	ImGui::Text("%i broadphase collision pairs found vs. bruteforce's %i\n", m_CollisionPairsThisFrame, ((World::c_TestCount * (World::c_TestCount - 1) / 2)));
}

void OctreeNode::Render(Renderer& renderer)
{
	if (m_DrawMode == BROADPHASE_DRAW_MODE::BROADPHASE_DRAW_NONE)
	{
		return;
	}

	renderer.SetDebugDrawMode();

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
		renderer.GetPushConstants().Padding.m[2][2] = (float)m_Depth;
		if (m_DrawMode == BROADPHASE_DRAW_MODE::BROADPHASE_DRAW_PAIRS)
		{
			if (m_Bucket.size() > 1)
			{
				renderer.Render(m_Model, m_CentreMatrix);
			}
		}
		else
		{
			renderer.Render(m_Model, m_CentreMatrix);
		}
	}
}

std::string OctreeNode::GetBroadPhaseStatsString()
{
	std::string str;
	str += std::to_string(c_MaxBucketCapacity); str += "max bucket size before split.\n";
	str += std::to_string(c_MaxOctreeNodeDepth); str += "max octree node depth.\n";
	str += std::to_string(m_CollisionPairsThisFrame); str += " broadphase collision pairs found vs. bruteforce's ";
	str += std::to_string((World::c_TestCount * (World::c_TestCount - 1) / 2));
	str += "\n";
	return str;
}
