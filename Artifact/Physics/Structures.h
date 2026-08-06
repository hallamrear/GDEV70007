#pragma once
#include <System/Types.h>
#include <Physics/Colliders/Collider.h>
#include <glm/glm.hpp>

#include <deque>
typedef std::deque<struct SupportVertex> Simplex;

struct Contact
{
	Vector3 HitPoint = Vector3(0.0f, 0.0f, 0.0f);
	float Depth = 0.0f;
	Vector3 Normal;
};

struct CollisionManifold
{
	std::pair<Entity*, Entity*> CollisionPair;
	Vector3 Normal = Vector3(0.0f, 0.0f, 0.0f);
	std::vector<Contact> ContactPoints;

	void Reset()
	{
		ContactPoints.clear();
		CollisionPair = { nullptr, nullptr };
	}
};

struct SupportVertex
{
	inline static SupportVertex GetSupportVertex(const Collider& colliderA, const Collider& colliderB, const Vector3& direction)
	{
		Vector3 dirNorm;
		XMStoreFloat3(&dirNorm, DirectX::XMVector3Normalize(XMLoadFloat3(&direction)));
		Vector3 invDirNorm = Vector3(-dirNorm.x, -dirNorm.y, -dirNorm.z);
		return SupportVertex(colliderA.GetSupportPoint(dirNorm), colliderB.GetSupportPoint(invDirNorm));
	};

	inline static SupportVertex GetSupportVertex(const Collider& colliderA, const Collider& colliderB, const glm::vec3& direction)
	{
		glm::vec3 invDirNorm = -direction;

		Vector3 supportA = colliderA.GetSupportPoint(Vector3(direction.x, direction.y, direction.z));
		Vector3 supportB = colliderB.GetSupportPoint(Vector3(invDirNorm.x, invDirNorm.y, invDirNorm.z));

		return SupportVertex(supportA, supportB);
	};

	glm::vec3 SupportVertexA;
	glm::vec3 SupportVertexB;
	glm::vec3 Diff;

	SupportVertex(const Vector3& supportA, const Vector3& supportB)
	{
		SupportVertexA.x = supportA.x;
		SupportVertexA.y = supportA.y;
		SupportVertexA.z = supportA.z;

		SupportVertexB.x = supportB.x;
		SupportVertexB.y = supportB.y;
		SupportVertexB.z = supportB.z;

		Diff.x = (SupportVertexA.x - SupportVertexB.x);
		Diff.y = (SupportVertexA.y - SupportVertexB.y);
		Diff.z = (SupportVertexA.z - SupportVertexB.z);

		//printf("%f %f %f - %f %f %f | %f %f %f\n",
		//	SupportVertexA.x, SupportVertexA.y, SupportVertexA.z,
		//	SupportVertexB.x, SupportVertexB.y, SupportVertexB.z,
		//	MinkDiff.x, MinkDiff.y, MinkDiff.z);
	}
	
	SupportVertex(const glm::vec3& supportA, const glm::vec3& supportB)
	{
		SupportVertexA = supportA;
		SupportVertexB = supportB;

		Diff.x = (SupportVertexB.x - SupportVertexA.x);
		Diff.y = (SupportVertexB.y - SupportVertexA.y);
		Diff.z = (SupportVertexB.z - SupportVertexA.z);
	}

	SupportVertex(const SupportVertex& supportVertex) : 
		SupportVertexA(supportVertex.SupportVertexA), 
		SupportVertexB(supportVertex.SupportVertexB), 
		Diff(supportVertex.Diff)
	{

	}

	SupportVertex()
	{
		SupportVertexA = { 0.0f, 0.0f, 0.0f };
		SupportVertexB = { 0.0f, 0.0f, 0.0f };
		Diff = { 0.0f, 0.0f, 0.0f };
	}
};

class EPA_Result
{
public:
	struct Triangle
	{
		SupportVertex Points[3];
		Vector3 Normal;

		Triangle()
		{
			Points[0] = { SupportVertex() };
			Points[1] = { SupportVertex() };
			Points[2] = { SupportVertex() };
			Normal = { Vector3(0.0f, 0.0f, 0.0f) };
		}
	} Tri;

	SupportVertex supportVertex;
	float Depth = 0.0f;
};