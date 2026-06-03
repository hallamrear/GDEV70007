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

	do
	{
		if (queryFace->VertexCount > 3)
			continue;

		Matrix4x4 inverseTransformA;
		DirectX::XMStoreFloat4x4(&inverseTransformA,
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&hullAMatrix))));

		Vector4 facePlane = queryFace->Plane.GetVector4();
		DirectX::XMStoreFloat4(&facePlane, 
			DirectX::XMPlaneTransform(DirectX::XMLoadFloat4(&facePlane), DirectX::XMLoadFloat4x4(&inverseTransformA)));

		Vector3 direction = -(Vector3(facePlane.x, facePlane.y, facePlane.z));
		DirectX::XMStoreFloat3(&direction,
			DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&direction), DirectX::XMLoadFloat4x4(&hullBMatrix)));
		
		int supportVertexIndex = -1;
		Vector3 vertexB = hullB.FindSupportVertex(direction, supportVertexIndex);
		DirectX::XMStoreFloat3(&vertexB, 
			DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&vertexB), DirectX::XMLoadFloat4x4(&hullBMatrix)));

		Plane plane = { facePlane.x, facePlane.y, facePlane.z, facePlane.w };
		float distance = DotPoint(plane, vertexB);

		if (distance - SAT_EPSILON <= faceQuery.Distance)
		{
			continue;
		}

		faceQuery.Face = queryFace;
		faceQuery.Distance = distance;

		if (faceQuery.Distance > SAT_EPSILON)
		{
			return faceQuery;
		}

	} while ((queryFace = queryFace->Next) != startingFace);

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

		Vector3 eA0;
		Vector3 A;
		Vector3 B;
		Vector3 BxA;

		MapVertexToGaussMap(hullAMatrix, *edgeA, eA0, BxA, A, B);

		for (const auto& edgeB : hullBEdgeList)
		{
			assert(edgeB->Twin->Twin == edgeB);

			Vector3 eB0;
			Vector3 C;
			Vector3 D;
			Vector3 DxC;
			
			MapVertexToGaussMap(hullBMatrix, *edgeB, eB0, DxC, C, D);

			if (IsMinkowskiFace(-BxA, -DxC, A, B, -C, -D) == false)
			{
				continue;
			}

			DirectX::XMMATRIX mat = DirectX::XMLoadFloat4x4(&hullAMatrix);
			DirectX::XMVECTOR translationVector = mat.r[3];

			Vector3 colliderOrigin = { 0.0f, 0.0f, 0.0f };
			colliderOrigin.x = DirectX::XMVectorGetX(translationVector);
			colliderOrigin.y = DirectX::XMVectorGetY(translationVector);
			colliderOrigin.z = DirectX::XMVectorGetZ(translationVector);

			float distance = EdgeEdgeDistance(BxA, DxC, eA0, eB0, colliderOrigin);

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

	DirectX::XMStoreFloat3(&edgeVertex,
		DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&edgeVertex),
			DirectX::XMLoadFloat4x4(&worldMatrix)));

	edgeOrigin = edgeVertex;

	Vector3 edgeDestination = edge.Twin->Tail->Vertex;

	DirectX::XMStoreFloat3(&edgeDestination,
		DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&edgeDestination),
			DirectX::XMLoadFloat4x4(&worldMatrix)));

	arcEdge = edgeDestination - edgeVertex;

	Vector3 edgeNormalA = edge.Face->Plane.Normal();
	Vector3 edgeNormalB = edge.Twin->Face->Plane.Normal();

	Matrix4x4 inverseWorld = Inverse(worldMatrix);

	DirectX::XMStoreFloat3(&edgeNormalA,
		DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&edgeNormalA), DirectX::XMLoadFloat4x4(&inverseWorld)));
	vertexA = edgeNormalA;

	DirectX::XMStoreFloat3(&edgeNormalB,
		DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&edgeNormalB), DirectX::XMLoadFloat4x4(&inverseWorld)));
	vertexB = edgeNormalB;
}