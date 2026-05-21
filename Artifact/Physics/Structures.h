#pragma once
#include <System/Types.h>
#include <Physics/Colliders/Collider.h>

struct Contact
{
	const Collider* SubjectA;
	const Collider* SubjectB;
	/// <summary>
	/// Hit point is always relative to the position of subject A.
	/// </summary>
	Vector3 HitPoint;
	Vector3 Normal;

	Contact(const Collider* subjectA, const Collider* subjectB, const Vector3 hitPoint, const Vector3 contactNormal) :
		SubjectA(subjectA),
		SubjectB(subjectB),
		HitPoint(hitPoint),
		Normal(contactNormal)
	{

	}

	Contact()
	{
		SubjectA = nullptr;
		SubjectB = nullptr;
		HitPoint = Vector3(0.0f, 0.0f, 0.0f);
		Normal = Vector3(0.0f, 0.0f, 0.0f);
	}
};

struct CollisionManifold
{
	float Depth;
	std::vector<Contact> ContactPoints;

	CollisionManifold()
	{
		ContactPoints = std::vector<Contact>();
		Depth = 0.0f;
	}

	~CollisionManifold()
	{
		Reset();
		Depth = 0.0f;
	}

	void Reset()
	{
		ContactPoints.clear();
		Depth = 0.0f;
	}
};

struct SupportVertex
{
	inline static SupportVertex GetSupportVertex(const Collider& colliderA, const Collider& colliderB, const Vector3& direction)
	{
		Vector3 dirNorm;
		XMStoreFloat3(&dirNorm, DirectX::XMVector3Normalize(XMLoadFloat3(&direction)));
		Vector3 invDirNorm = Vector3(-dirNorm.x, -dirNorm.y, -dirNorm.z);
		return SupportVertex(colliderA.GetFurthestPointInDirection(dirNorm), colliderB.GetFurthestPointInDirection(invDirNorm));
	};

	Vector3 SupportVertexA;
	Vector3 SupportVertexB;
	Vector3 MinkowskiDifference;

	SupportVertex(const Vector3& supportA, const Vector3& supportB)
	{
		SupportVertexA = supportA;
		SupportVertexB = supportB;
		MinkowskiDifference.x = (SupportVertexA.x - SupportVertexB.x);
		MinkowskiDifference.y = (SupportVertexA.y - SupportVertexB.y);
		MinkowskiDifference.z = (SupportVertexA.z - SupportVertexB.z);
	}

	SupportVertex(const SupportVertex& supportVertex) : 
		SupportVertexA(supportVertex.SupportVertexA), 
		SupportVertexB(supportVertex.SupportVertexB), 
		MinkowskiDifference(supportVertex.MinkowskiDifference)
	{

	}

	SupportVertex()
	{
		SupportVertexA = Vector3();
		SupportVertexB = Vector3();
		MinkowskiDifference = Vector3();
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
};