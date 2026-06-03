#include "pch.h"
#include <Physics/SAT/SeparatingAxisTheorem.h>
#include <Physics/Colliders/Collider.h>
#include <World/Entity.h>
#include <System/Maths.h>
#include <Physics/Quickhull/Quickhull.h>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_access.hpp>

using namespace Maths;

/// <summary>
/// This function comes from the real time collision detection book.
/// Returns the squared distance between SegmentA(tA) && SegmentB(tB)
/// </summary>
float ClosestPoint_SegmentVsSegment(
	const glm::vec3& segmentA0,
	const glm::vec3& segmentA1,
	const glm::vec3& segmentB0,
	const glm::vec3& segmentB1,
	glm::vec3& closestPointA,
	glm::vec3& closestPointB,
	float& tA, float& tB)
{

	glm::vec3 dirA = segmentA1 - segmentA0;
	glm::vec3 dirB = segmentB1 - segmentB0;

	glm::vec3 vertical = segmentA0 - segmentB0;

	float dirASqr = glm::length2(dirA);
	float dirBSqr = glm::length2(dirB);

	float lineBCenterProjection = glm::dot(dirB, vertical);

	//Are both points degenerate edge points?
	if (dirASqr <= SAT_EPSILON && dirBSqr <= SAT_EPSILON)
	{
		tA = 0.0f;
		tB = 0.0f;
		closestPointA = segmentA0;
		closestPointB = segmentB0;
		return glm::dot(closestPointA - closestPointA, closestPointA - closestPointA);
	}

	//First point is an end point.
	if (dirASqr <= SAT_EPSILON)
	{
		tA = 0.0f;
		tB = lineBCenterProjection / dirASqr;
		tB = std::clamp(tB, 0.0f, 1.0f);
	}
	else
	{
		float lineACenterProjection = glm::dot(dirA, vertical);

		if (dirBSqr <= SAT_EPSILON)
		{
			//second point is an edng point.
			tA = -lineACenterProjection / dirASqr;
			tA = std::clamp(tA, 0.0f, 1.0f);
			tB = 0.0f;
		}
		else
		{
			//Nothing is degenerate! Yipppee!!

			float b = glm::dot(dirA, dirB);
			float denominator = dirASqr * dirBSqr - b * b; //Ensure always non-negative

			//If not parallel, compute closest point on line, then clamp to segment
			//else 0?
			if (denominator != 0.0f)
			{
				tA = (b * lineBCenterProjection - lineACenterProjection * dirBSqr) / denominator;
				tA = std::clamp(tA, 0.0f, 1.0f);
			}
			else
			{
				tA = 0.0f;
			}

			tB = (b * tA + lineBCenterProjection) / dirBSqr;

			//poor mans multi clamp
			if (tB < 0.0f)
			{
				tA = -lineACenterProjection / dirASqr;
				tA = std::clamp(tA, 0.0f, 1.0f);
				tB = 0.0f;
			}
			else if(tB > 1.0f)
			{
				tA = (b - lineACenterProjection) / dirASqr;
				tA = std::clamp(tA, 0.0f, 1.0f);
				tB = 1.0f;
			}

		}
	}

	closestPointA = segmentA0 + dirA * tA;
	closestPointB = segmentB0 + dirB * tB;

	return glm::dot(closestPointA - closestPointB, closestPointA - closestPointB);
}

FaceQuery SeparatingAxisTheorem::QueryFace(const ConvexHull& hullA, const glm::mat4x4& hullAMatrix, const ConvexHull& hullB, const glm::mat4x4& hullBMatrix)
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

		glm::mat4x4 inverseTransform = glm::inverse(hullAMatrix);

		glm::vec4 facePlane = { queryFace->Plane.x, queryFace->Plane.y, queryFace->Plane.z, queryFace->Plane.w };
		facePlane = facePlane * inverseTransform;

	/*	glm::vec4 O = { queryFace->Plane.x * queryFace->Plane.w, queryFace->Plane.y * queryFace->Plane.w, queryFace->Plane.z * queryFace->Plane.w, VECTOR_W_POSITION };
		glm::vec4 N = { queryFace->Plane.x, queryFace->Plane.y, queryFace->Plane.z, VECTOR_W_DIRECTION };
		O = hullAMatrix * O;
		N = glm::inverse(hullAMatrix) * N;
		glm::vec3 On = { O.x, O.y, O.z };
		glm::vec3 Nn = { N.x, N.y, N.z };
		N = glm::normalize(N);
		facePlane.w = glm::dot(On, Nn);
		facePlane.x = N.x;
		facePlane.y = N.y;
		facePlane.z = N.z;*/

		glm::mat3x3 rotationMatrixB =
		{
			hullBMatrix[0][0], hullBMatrix[0][1], hullBMatrix[0][2], //hullBMatrix[0][3],
			hullBMatrix[1][0], hullBMatrix[1][1], hullBMatrix[1][2], //hullBMatrix[1][3],
			hullBMatrix[2][0], hullBMatrix[2][1], hullBMatrix[2][2], //hullBMatrix[2][3],
			//hullBMatrix[3][0], hullBMatrix[3][1], hullBMatrix[3][2], //hullBMatrix[3][3],
		};

		glm::vec3 direction = -glm::vec3(facePlane.x, facePlane.y, facePlane.z);
		direction = glm::transpose(rotationMatrixB) * direction;
		direction = glm::normalize(direction);

		int supportVertexIndex = -1;
		glm::vec4 vertexB = { hullB.FindSupportVertex(direction, supportVertexIndex), 1.0f };
		vertexB = hullBMatrix * vertexB;

		float distance = glm::dot(facePlane, vertexB);

		if (distance <= faceQuery.Distance)
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

EdgeQuery SeparatingAxisTheorem::QueryEdge(const ConvexHull& hullA, const glm::mat4x4& hullAMatrix,  const ConvexHull& hullB, const glm::mat4x4& hullBMatrix)
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

		glm::vec3 eA0;
		glm::vec3 A;
		glm::vec3 B;
		glm::vec3 BxA;

		MapVertexToGaussMap(glm::transpose(hullAMatrix), *edgeA, eA0, BxA, A, B);

		for (const auto& edgeB : hullBEdgeList)
		{
			assert(edgeB->Twin->Twin == edgeB);

			glm::vec3 eB0;
			glm::vec3 C;
			glm::vec3 D;
			glm::vec3 DxC;
			
			MapVertexToGaussMap(glm::transpose(hullBMatrix), *edgeB, eB0, DxC, C, D);

			if (IsMinkowskiFace(-BxA, -DxC, A, B, -C, -D) == false)
			{
				continue;
			}

			glm::vec4 column3 = glm::row(hullAMatrix, 3);
			glm::vec3 colliderOrigin = column3;

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

bool SeparatingAxisTheorem::CheckCollision(SAT_Result& result, const ConvexHull& hullA, const glm::mat4x4& hullAMatrix, const ConvexHull& hullB, const glm::mat4x4& hullBMatrix, CollisionManifold* manifold)
{
	result.EdgeTest.Distance = -INFINITY;
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

	if (manifold != nullptr)
	{
		ConstructContactManifold(*manifold, result, hullA, hullAMatrix, hullB, hullBMatrix);
	}

	return true;
}

bool SeparatingAxisTheorem::ParallelTest(const glm::vec3& crossBA, const glm::vec3& crossDC)
{
	glm::vec3 crossed = glm::cross(crossBA, crossDC);
	float crossMag = glm::length(crossed);
	float magSum = std::sqrtf(glm::length2(crossBA) * glm::length2(crossDC));

	return (crossMag < (magSum * SAT_EPSILON));
}

float SeparatingAxisTheorem::EdgeEdgeDistance(
	const glm::vec3& crossBA, const glm::vec3& crossDC, 
	const glm::vec3& edgeA_WSOrigin,
	const glm::vec3& edgeB_WSOrigin,
	const glm::vec3& colliderWSOrigin)
{
	if (ParallelTest(crossBA, crossDC))
	{
		return -INFINITY;
	}

	glm::vec3 normal = glm::normalize(glm::cross(crossBA, crossDC));

	if (glm::dot(normal, edgeA_WSOrigin - colliderWSOrigin) < 0.0f)
	{
		normal = -normal;
	}

	return glm::dot(normal, edgeB_WSOrigin - edgeA_WSOrigin);
}

//calculates an epsilon value between a relative and absolute tolerance
extern float Epsilon(const float& relativeTolerance, const float& relativeValue, const float& absoluteTolerance)
{
	return relativeTolerance * relativeValue + absoluteTolerance;
}

ConvexHullFace* SeparatingAxisTheorem::FindIncidentFace(const ConvexHull& incidentHull, const glm::mat4x4& inverseIncidentMatrix, const ConvexHullFace* referenceFace)
{
	ConvexHullFace* incidentFace = nullptr;

	float furthestDistance = INFINITY;

	glm::vec4 referencePlane = { referenceFace->Plane.x,referenceFace->Plane.y, referenceFace->Plane.z, referenceFace->Plane.w };
	glm::vec4 potentialPlane = { };
	glm::vec3 referencePlaneNormal = {};
	glm::vec3 potentialPlaneNormal = {};
	ConvexHullFace* start = incidentHull.FacesListHead;
	ConvexHullFace* next = start ;
	do
	{
		potentialPlane = { next->Plane.x, next->Plane.y, next->Plane.z, next->Plane.w };
		potentialPlane = { potentialPlane * inverseIncidentMatrix };
		
		referencePlaneNormal = { referencePlane.x, referencePlane.y, referencePlane.z };
		potentialPlaneNormal = { potentialPlane.x, potentialPlane.y, potentialPlane.z };

		float distance = glm::dot(referencePlaneNormal, potentialPlaneNormal);

		if (distance - SAT_EPSILON < furthestDistance)
		{
			furthestDistance = distance;
			incidentFace = next;
		}

		next = next->Next;
	} while (next != start);

	assert(incidentFace != nullptr);

	return incidentFace;
}

EdgeContact SeparatingAxisTheorem::CreateEdgeContacts(const EdgeQuery& edgeQuery, const ConvexHull& hullA, const glm::mat4x4& hullAMatrix, const ConvexHull& hullB, const glm::mat4x4& hullBMatrix)
{
	UNREFERENCED_PARAMETER(hullA);
	UNREFERENCED_PARAMETER(hullB);

	EdgeContact contact;
	contact.Query = edgeQuery;

	glm::vec4 edgeAV1 = { edgeQuery.EdgeA->Tail->Vertex, VECTOR_W_POSITION };
	glm::vec4 edgeAV2 = { edgeQuery.EdgeA->Twin->Tail->Vertex, VECTOR_W_POSITION };
	glm::vec4 edgeBV1 = { edgeQuery.EdgeB->Tail->Vertex, VECTOR_W_POSITION };
	glm::vec4 edgeBV2 = { edgeQuery.EdgeB->Twin->Tail->Vertex, VECTOR_W_POSITION };

	glm::vec4 edgeAStart = hullAMatrix * edgeAV1;
	glm::vec4 edgeAEnd = hullAMatrix * edgeAV2;
	glm::vec4 edgeBStart = hullBMatrix * edgeBV1;
	glm::vec4 edgeBEnd = hullBMatrix * edgeBV2;

	glm::vec3 LineA;
	float tA = 0.0f;
	glm::vec3 LineB;
	float tB = 0.0f;
	ClosestPoint_SegmentVsSegment(
		edgeAV1, edgeAV2,
		edgeBV1, edgeBV2,
		LineA, LineB,
		tA, tB);

	glm::vec3 midpoint = (LineA + LineB) * 0.5f;
	glm::vec3 axis = glm::normalize(LineA - LineB);

	glm::vec3 hullAPosition = glm::row(hullAMatrix, 3);

	//Make sure its pointing out!
	glm::vec3 dirFromCentre = edgeQuery.EdgeA->Tail->Vertex - hullAPosition;
	if (glm::dot(axis, dirFromCentre) < SAT_EPSILON)
	{
		axis = -axis;
	}

	contact.MidPoint = Vector3(midpoint.x, midpoint.y, midpoint.z);
	contact.Axis = Vector3(axis.x, axis.y, axis.z);

	return contact;
}

FaceContact SeparatingAxisTheorem::CreateFaceContacts(const FaceQuery& faceQuery,
	const ConvexHull& referenceHull, const glm::mat4x4& referenceHullMatrix, 
	const ConvexHull& incidentHull, const glm::mat4x4& incidentHullMatrix)
{
	UNREFERENCED_PARAMETER(referenceHull);

	FaceContact faceContact;
	faceContact.Query = faceQuery;

	glm::mat4x4 inverseReferenceHullMatrix = glm::inverse(referenceHullMatrix);
	glm::mat4x4 inverseIncidentMatrix = glm::inverse(incidentHullMatrix);

	//Gather faces.
	const ConvexHullFace* referenceFace = faceQuery.Face;
	glm::vec4 referenceFacePlane = { referenceFace->Plane.x, referenceFace->Plane.y, referenceFace->Plane.z, referenceFace->Plane.w };
	referenceFacePlane = referenceFacePlane * inverseReferenceHullMatrix;

	const ConvexHullFace* incidentFace = FindIncidentFace(incidentHull, incidentHullMatrix, referenceFace);

	std::vector<glm::vec3> incidentEdges;
	glm::vec3 vertex;
	glm::vec4 preTransformVertex;
	
	ConvexHullHalfEdge* start = incidentFace->Edge;
	ConvexHullHalfEdge* edge = start;

	//Collect edges of incident face.
	do
	{
		preTransformVertex = { edge->Tail->Vertex, VECTOR_W_POSITION };
		vertex = incidentHullMatrix * preTransformVertex;
		incidentEdges.push_back(vertex);
		edge = edge->Next;
	} while (edge != start);

	//Clip edges against reference face.
	start = referenceFace->Edge;
	edge = start;
	ConvexHullFace* sideFace = nullptr;
	glm::vec4 sideFacePlane;

	do
	{
		if (edge->Face != referenceFace)
		{
			sideFace = edge->Face;
		}
		else
		{
			sideFace = edge->Twin->Face;
		}

		sideFacePlane = { sideFace->Plane.x, sideFace->Plane.y, sideFace->Plane.z, sideFace->Plane.w };
		sideFacePlane = sideFacePlane * inverseReferenceHullMatrix;

		incidentEdges = ClipEdgesAgainstPlane(sideFacePlane, incidentEdges);

		edge = edge->Next;
	} while (edge != start);

	for (auto& incidentVertex : incidentEdges)
	{
		faceContact.HitPoints.push_back(ClosestPointOnPlane(referenceFacePlane, incidentVertex));
	}

	//todo : ACTUALLY REDUCE CONTACT POINTS DOWN TO 4 HOLY CHRIST
	//ReduceContactPoints(faceContact);

	//todo : is this necessary?
	glm::vec4 incidentFacePlane = { incidentFace->Plane.x, incidentFace->Plane.y, incidentFace->Plane.z, incidentFace->Plane.w };
	incidentFacePlane = incidentFacePlane * inverseIncidentMatrix;

	faceContact.HitPoints.erase(std::remove_if(faceContact.HitPoints.begin(), faceContact.HitPoints.end(), 
		
		[&](glm::vec3& contactPoint) 
		{
			glm::vec4 cp = { contactPoint, VECTOR_W_POSITION };
			return glm::dot(incidentFacePlane, cp) > -0.001f;
		}), faceContact.HitPoints.end());

	return faceContact;
}

bool IntersectionPlaneVsRay(
	const glm::vec3& rayStart, const glm::vec3& rayDirection,
	const glm::vec4& plane, glm::vec3& intersectionPoint)
{
	float p = glm::dot(plane, { rayStart, VECTOR_W_POSITION } );
	float v = glm::dot(plane, { rayDirection, VECTOR_W_DIRECTION } );

	float t = -1.0f * (p / v);

	if (t >= -INFINITY && std::fabsf(t) != INFINITY)//t acts a  projecting scalar value of p through v
	{
		intersectionPoint = rayStart + (rayDirection * t);
		return true;
	}

	return false;
}

/// <summary>
/// Utilises the sutherland-hodg(e?)man clippibng algorithm.
/// Returns list of clipped vertices.
/// </summary>
std::vector<glm::vec3> SeparatingAxisTheorem::ClipEdgesAgainstPlane(const glm::vec4& plane, const std::vector<glm::vec3>& edges)
{
	size_t faceVertexCount = edges.size();
	std::vector<glm::vec3> clippedVertices;
	clippedVertices.resize(faceVertexCount);

	for (size_t i = 0; i < faceVertexCount; i++)
	{
		int nextVertexIndex = ((int)i + 1) % faceVertexCount;

		//Determine edge.
		glm::vec3 edgeStart = edges[i];
		glm::vec3 edgeEnd = edges[nextVertexIndex];

		//Determine edge projections 
		float startDistance = glm::dot(plane, { edgeStart, VECTOR_W_POSITION });
		float endDistance = glm::dot(plane, { edgeEnd, VECTOR_W_POSITION });

		//(if behind plane we dont need to clip at all)
		if (startDistance <= 0.0f && endDistance <= 0.0f)
		{
			if (std::find(clippedVertices.begin(), clippedVertices.end(), edgeEnd) == clippedVertices.end())
			{
				clippedVertices.push_back(edgeEnd);
			}

			if (std::find(clippedVertices.begin(), clippedVertices.end(), edgeStart) == clippedVertices.end())
			{
				clippedVertices.push_back(edgeStart);
			}

			continue;
		}

		//doign the same thing for the front.
		if (startDistance > 0.0f && endDistance > 0.0f)
		{
			glm::vec3 closestEnd = ClosestPointOnPlane(plane, edgeEnd);
			if (std::find(clippedVertices.begin(), clippedVertices.end(), closestEnd) == clippedVertices.end())
			{
				clippedVertices.push_back(closestEnd);
			}

			glm::vec3 closestStart = ClosestPointOnPlane(plane, edgeStart);
			if (std::find(clippedVertices.begin(), clippedVertices.end(), closestStart) == clippedVertices.end())
			{
				clippedVertices.push_back(closestStart);
			}

			continue;
		}

		glm::vec3 edgeDirection = glm::normalize(edgeStart - edgeEnd);
		glm::vec3 intersection;
		IntersectionPlaneVsRay(edgeEnd, edgeDirection, plane, intersection);

		if (endDistance <= 0.0f)
		{
			if (std::find(clippedVertices.begin(), clippedVertices.end(), edgeEnd) == clippedVertices.end())
			{
				clippedVertices.push_back(edgeEnd);
			}

			if (std::find(clippedVertices.begin(), clippedVertices.end(), intersection) == clippedVertices.end())
			{
				clippedVertices.push_back(intersection);
			}

			continue;
		}

		if (std::find(clippedVertices.begin(), clippedVertices.end(), intersection) == clippedVertices.end())
		{
			clippedVertices.push_back(intersection);
		}

		if (std::find(clippedVertices.begin(), clippedVertices.end(), edgeStart) == clippedVertices.end())
		{
			clippedVertices.push_back(edgeStart);
		}
	}

	return clippedVertices;
}

/// <summary>
/// Taken from real time collision detection
/// </summary>
glm::vec3 SeparatingAxisTheorem::ClosestPointOnPlane(const glm::vec4& plane, const glm::vec3& point)
{
	glm::vec3 planeNormal = { plane.x, plane.y, plane.z };
	planeNormal = glm::normalize(planeNormal);
	float t = glm::dot(planeNormal, point) + plane.w;
	glm::vec3 result = { point - planeNormal * t };
	return result;
}

std::vector<Contact> SeparatingAxisTheorem::ConvertContactToWorldSpace(const FaceContact& faceContact)
{
	std::vector<Contact> contacts;
	
	for (const glm::vec3& vertex : faceContact.HitPoints)
	{
		Contact contact;
		contact.Depth = faceContact.Query.Distance;
		contact.HitPoint = Vector3(vertex.x, vertex.y, vertex.z);
		contacts.push_back(contact);
	} 
	return contacts;
}

Contact SeparatingAxisTheorem::ConvertContactToWorldSpace(const EdgeContact& edgeContact)
{
	Contact contact;
	contact.Depth = edgeContact.Query.Distance;
	contact.HitPoint = edgeContact.MidPoint;
	return contact;
}

void SeparatingAxisTheorem::ConstructContactManifold(CollisionManifold& manifold, const SAT_Result& satResult,
	const ConvexHull& hullA, const glm::mat4x4& hullAMatrix,
	const ConvexHull& hullB, const glm::mat4x4& hullBMatrix)
{
	float faceADistance = satResult.FaceTestA.Distance;
	float faceBDistance = satResult.FaceTestB.Distance;
	float edgeDistance = satResult.EdgeTest.Distance;

	//https://www.gamedev.net/forums/topic/667499-3d-sat-problem/?page=2#:~:text=MKS%20for%20the-,absolute%20tolerance,-)%3A - ty dirk
	constexpr float kLinearSlop = (0.005f);
	constexpr float kRelEdgeTolerance = (0.90f);
	constexpr float kRelFaceTolerance = (0.98f);
	constexpr float kAbsTolerance = (0.5f * kLinearSlop);

	float edgeEpsilon = Epsilon(kRelEdgeTolerance, std::fmaxf(faceADistance, faceBDistance), kAbsTolerance);
	float faceEpsilon = Epsilon(kRelFaceTolerance, faceADistance, kAbsTolerance);

	//test for edge contact first
	if (edgeDistance > edgeEpsilon)
	{
		EdgeContact edgeContact = CreateEdgeContacts(satResult.EdgeTest, hullA, hullAMatrix, hullB, hullBMatrix);
		Contact contact = ConvertContactToWorldSpace(edgeContact);
		manifold.Normal = edgeContact.Axis;
		manifold.ContactPoints.push_back(contact);
	}
	else if (faceBDistance > faceEpsilon)
	{
		FaceContact faceContact = CreateFaceContacts(satResult.FaceTestB, hullB, hullBMatrix, hullA, hullAMatrix);
		std::vector<Contact> contacts = ConvertContactToWorldSpace(faceContact);
		
		////invert normal and convert to world space
		Vector3 normal = satResult.FaceTestB.Face->Plane.Normal();
		glm::vec4 inverseNormal = glm::vec4(-1.0f * normal.x, -1.0f * normal.y, -1.0f * normal.z, VECTOR_W_DIRECTION);
		inverseNormal = glm::normalize(inverseNormal * glm::inverse(hullBMatrix));
		manifold.Normal = Vector3(inverseNormal.x, inverseNormal.y, inverseNormal.z);

		size_t contactCount = contacts.size();
		for (size_t i = 0; i < contactCount; i++)
		{
			manifold.ContactPoints.push_back(contacts[i]);
		}
	}
	else
	{
		FaceContact faceContact = CreateFaceContacts(satResult.FaceTestA, hullA, hullAMatrix, hullB, hullBMatrix);
		std::vector<Contact> contacts = ConvertContactToWorldSpace(faceContact);
		
		Vector3 normal = satResult.FaceTestA.Face->Plane.Normal();
		glm::vec4 inverseNormal = glm::vec4(-1.0f * normal.x, -1.0f * normal.y, -1.0f * normal.z, VECTOR_W_DIRECTION);
		inverseNormal = glm::normalize(inverseNormal * glm::inverse(hullAMatrix));
		manifold.Normal = Vector3(inverseNormal.x, inverseNormal.y, inverseNormal.z);

		size_t contactCount = contacts.size();
		for (size_t i = 0; i < contactCount; i++)
		{
			manifold.ContactPoints.push_back(contacts[i]);
		}
	}

	//contactManifold->constraintType = ConstraintType::CONTACT;
}

bool SeparatingAxisTheorem::IsMinkowskiFace(const glm::vec3& BxA, const glm::vec3& DxC, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d)
{
	//Determine half space plane distances using triple products.
	float halfspaceCBA = glm::dot(c, BxA);
	float halfspaceDBA = glm::dot(d, BxA);
	float halfspaceADC = glm::dot(a, DxC);
	float halfspaceBDC = glm::dot(b, DxC);

	bool intersectionBA = (halfspaceCBA * halfspaceDBA) < 0.0f;
	bool intersectionDC = (halfspaceADC * halfspaceBDC) < 0.0f;

	//are arcs in the same hemisphere?
	bool sameHemisphere = (halfspaceCBA * halfspaceBDC) > 0.0f;

	return (intersectionBA && intersectionDC && sameHemisphere);
}

//DeriveGaussMapping(const MFtransform& worldSpace, const HullHalfEdge const* edge, MFpoint3& edgeOrigin, MFvec3& arcEdge, MFvec3& vertexA, MFvec3& vertexB)
void SeparatingAxisTheorem::MapVertexToGaussMap(const glm::mat3x3& worldMatrix, const ConvexHullHalfEdge& edge, glm::vec3& edgeOrigin, glm::vec3& arcEdge, glm::vec3& vertexA, glm::vec3& vertexB)
{
	edgeOrigin = worldMatrix * edge.Tail->Vertex;
	glm::vec3 edgeDestination = worldMatrix * edge.Twin->Tail->Vertex;

	arcEdge = edgeDestination - edgeOrigin;

	Vector3 faceNormalA = edge.Face->Plane.Normal();
	Vector3 faceNormalB = edge.Twin->Face->Plane.Normal();

	glm::mat3x3 inverseWorld = glm::inverse(worldMatrix);
	glm::vec3 edgeNormalA = {faceNormalA.x, faceNormalA.y, faceNormalA.z };
	glm::vec3 edgeNormalB = { faceNormalB.x, faceNormalB.y, faceNormalB.z };

	vertexA = glm::normalize(edgeNormalA) * inverseWorld;
	vertexB = glm::normalize(edgeNormalB) * inverseWorld;
}