#pragma once
#include <Physics/Colliders/AABBCollider.h>
#include <vector>

class Renderer;

class OctreeNode
{
private:
	static constexpr int c_ChildCount = 8;
	static constexpr int c_MaxBucketCapacity = 4;
	static constexpr int c_MaxOctreeNodeDepth = 4;

	static ModelRef m_Model;
	float m_HalfWidth;
	Matrix4x4 m_CentreMatrix;
	Vector3 m_Centre;

	OctreeNode* m_Parent;
	bool m_HasSplit;
	const int m_Depth;
	OctreeNode* m_Children[c_ChildCount];
	std::vector<Entity*> m_Bucket;
	bool SplitNode(const std::vector<Entity*>& entitiesToSort);

	OctreeNode(OctreeNode* parent, const Vector3& centre, const float& colliderSize, const int& depth);

public:
	~OctreeNode();

	const bool& HasSplit() const;
	void AddEntity(const std::vector<Entity*>& entitiesToAdd);
	static void Render(Renderer& renderer, OctreeNode* root);

	static OctreeNode* BuildOctree(OctreeNode* parent, const Vector3& centre, const float& halfWidth, const int& depth);
	static void DestroyOctree(OctreeNode* root);
};

