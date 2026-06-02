#include "pch.h"
#include <Physics/SAT/SeparatingAxisTheorem.h>
#include <Physics/Colliders/Collider.h>
#include <Physics/Structures.h>
#include <World/Entity.h>
#include <System/Maths.h>
#include <Physics/Quickhull/Quickhull.h>

using namespace Maths;

FaceQuery SeparatingAxisTheorem::QueryFace(const ConvexHull& hullA, const Matrix4x4& hullAMatrix, const ConvexHull& hullB, const Matrix4x4& hullBMatrix)
{
	FaceQuery faceQuery;
	faceQuery.Face = nullptr;
	faceQuery.Distance = -INFINITY;

	ConvexHullFace* startingFace = hullA.FacesListHead;
	ConvexHullFace* queryFace = startingFace;

	Matrix4x4 inverseWorldMatrixA = InverseTranspose(hullAMatrix);
	Matrix4x4 inverseWorldMatrixB = InverseTranspose(hullBMatrix);

	do
	{
		Plane facePlane = queryFace->Plane;		
		facePlane = TransformPlane(facePlane, hullAMatrix);

		Vector3 direction = MultiplyScalar(-1.0f, facePlane.Normal());
		DirectX::XMStoreFloat3(&direction, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&direction), DirectX::XMLoadFloat4x4(&inverseWorldMatrixB)));
		
		int supportVertexIndex = -1;
		Vector3 vertexB = hullB.FindSupportVertex(direction, supportVertexIndex);
		DirectX::XMStoreFloat3(&vertexB, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&vertexB), DirectX::XMLoadFloat4x4(&inverseWorldMatrixB)));

		float distance = DotPoint(facePlane, vertexB);

		if (distance > faceQuery.Distance)
		{
			faceQuery.Face = queryFace;
			faceQuery.Distance = distance;
		}

	} while (queryFace = queryFace->Next, queryFace != startingFace);

	return faceQuery;
}

EdgeQuery SeparatingAxisTheorem::QueryEdge(const ConvexHull& hullA, const Matrix4x4& hullAMatrix,  const ConvexHull& hullB, const Matrix4x4& hullBMatrix)
{
	EdgeQuery edgeQuery;
	edgeQuery.Distance = -INFINITY;
	edgeQuery.EdgeA = nullptr;
	edgeQuery.EdgeB = nullptr;

	const std::vector<ConvexHullHalfEdge*>& hullAEdgeList = hullA.GetEdgeList();
	const std::vector<ConvexHullHalfEdge*>& hullBEdgeList = hullB.GetEdgeList();

	for (const auto& edgeA : hullAEdgeList)
	{
		assert(edgeA->Twin->Twin == edgeA);

		Vector3 eA_origin;
		Vector3 eA_A;
		Vector3 eA_B;
		Vector3 eA_BcrossA;

		MapVertexToGaussMap(hullAMatrix, *edgeA, eA_origin, eA_BcrossA, eA_A, eA_B);

		for (const auto& edgeB : hullBEdgeList)
		{
			assert(edgeB->Twin->Twin == edgeB);

			Vector3 eB_origin;
			Vector3 eB_A;
			Vector3 eB_B;
			Vector3 eB_BcrossA;
			
			MapVertexToGaussMap(hullBMatrix, *edgeB, eB_origin, eB_BcrossA, eB_A, eB_B);

			Vector3 inverse_e2A = MultiplyScalar(-1.0f, eB_A);
			Vector3 inverse_e2B = MultiplyScalar(-1.0f, eB_B);
			Vector3 inverseBcrossA = MultiplyScalar(-1.0f, eA_BcrossA);
			Vector3 inverseDcrossC = MultiplyScalar(-1.0f, eB_BcrossA);

			if (IsMinkowskiFace(inverseBcrossA, inverseDcrossC, eA_A, eA_B, inverse_e2A, inverse_e2B) == false)
			{
				continue;
			}

			DirectX::XMMATRIX mat = DirectX::XMLoadFloat4x4(&hullAMatrix);
			DirectX::XMVECTOR translationVector = mat.r[3];

			Vector3 colliderOrigin = { 0.0f, 0.0f, 0.0f };
			colliderOrigin.x = DirectX::XMVectorGetX(translationVector);
			colliderOrigin.y = DirectX::XMVectorGetY(translationVector);
			colliderOrigin.z = DirectX::XMVectorGetZ(translationVector);

			float distance = EdgeEdgeDistance(eA_BcrossA, eB_BcrossA, eA_origin, eB_origin, colliderOrigin);

			if (distance <= edgeQuery.Distance)
			{
				continue;
			}

			edgeQuery.Distance = distance;
			edgeQuery.EdgeA = edgeA;
			edgeQuery.EdgeB = edgeB;

			if (distance > SAT_EPSILON)
			{
				return edgeQuery;
			}
		}
	}
	
	return edgeQuery;
}

bool SeparatingAxisTheorem::CheckCollision(const ConvexHull& hullA, const Matrix4x4& hullAMatrix, const ConvexHull& hullB, const Matrix4x4& hullBMatrix, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(manifold);

	SAT_Result result;

	result.FaceTestA = QueryFace(hullA, hullAMatrix, hullB, hullBMatrix);

	if (result.FaceTestA.Distance > SAT_EPSILON)
	{
		return false;
	}

	result.FaceTestB = QueryFace(hullB, hullBMatrix, hullA, hullAMatrix);

	if (result.FaceTestB.Distance > SAT_EPSILON)
	{
		return false;
	}

	result.EdgeTest = QueryEdge(hullA, hullAMatrix, hullB, hullBMatrix);
	
	if (result.EdgeTest.Distance > SAT_EPSILON)
	{
		return false;
	}

	return true;
}

bool SeparatingAxisTheorem::ParallelTest(const Vector3& crossBA, const Vector3& crossDC)
{
	constexpr float parallelToleralance = 0.0005f;

	Vector3 crossed = Cross(crossBA, crossDC);
	float crossMag = Magnitude(crossed);
	float magSum = std::sqrtf(MagnitudeSqr(crossBA) * MagnitudeSqr(crossDC));

	return (crossMag < (magSum * parallelToleralance));
}

float SeparatingAxisTheorem::EdgeEdgeDistance(
	const Vector3& crossBA, const Vector3& crossDC, 
	const Vector3& edgeA_WSOrigin,
	const Vector3& edgeB_WSOrigin,
	const Vector3& colliderWSOrigin)
{
	if (ParallelTest(crossBA, crossDC))
	{
		return -INFINITY;
	}

	Vector3 normal = CrossNormalised(crossBA, crossDC);

	if (Dot(normal, edgeA_WSOrigin - colliderWSOrigin) < 0.0f)
	{
		normal = MultiplyScalar(-1.0f, normal);
	}

	return Dot(normal, edgeB_WSOrigin - edgeA_WSOrigin);
}

bool SeparatingAxisTheorem::IsMinkowskiFace(const Vector3& BxA, const Vector3& DxC, const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d)
{
	//Determine half space plane distances using triple products.
	float halfspaceCBA = Dot(c, BxA);
	float halfspaceDBA = Dot(d, BxA);
	float halfspaceADC = Dot(a, DxC);
	float halfspaceBDC = Dot(b, DxC);

	bool intersectionBA = (halfspaceCBA * halfspaceDBA) < 0.0f;
	bool intersectionDC = (halfspaceADC * halfspaceBDC) < 0.0f;

	//are arcs in the same hemisphere?
	bool sameHemisphere = (halfspaceCBA * halfspaceBDC) > 0.0f;

	return (intersectionBA && intersectionDC && sameHemisphere);
}

void SeparatingAxisTheorem::MapVertexToGaussMap(const Matrix4x4& worldMatrix, const ConvexHullHalfEdge& edge, Vector3& edgeOrigin, Vector3& arcEdge, Vector3& vertexA, Vector3& vertexB)
{
	Vector3 edgeVertex = edge.Tail->Vertex;

	DirectX::XMStoreFloat3(&edgeOrigin,
		DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&edgeVertex),
			DirectX::XMLoadFloat4x4(&worldMatrix)));

	Vector3 edgeDestination = edge.Twin->Tail->Vertex;

	DirectX::XMStoreFloat3(&edgeDestination,
		DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&edgeDestination),
			DirectX::XMLoadFloat4x4(&worldMatrix)));

	arcEdge = edgeDestination - edgeOrigin;

	Vector3 edgeNormalA = edge.Face->Plane.Normal();
	Vector3 edgeNormalB = edge.Twin->Face->Plane.Normal();

	DirectX::XMStoreFloat3(&vertexA,
		DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&edgeNormalA),
			DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&worldMatrix))));

	DirectX::XMStoreFloat3(&vertexB,
		DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&edgeNormalB),
			DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&worldMatrix))));
}