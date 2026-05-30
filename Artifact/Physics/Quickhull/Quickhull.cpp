#include "pch.h"
#include "Quickhull.h"

using namespace Maths;

// Source - https://stackoverflow.com/a/58717895
// Posted by 463035818_is_not_an_ai, modified by community. See post 'Timeline' for change history
// Retrieved 2026-05-30, License - CC BY-SA 4.0
struct EqualPredicate {
	bool operator()(const Vector3& l, const Vector3& r)
	{
		return (std::abs(l.x - r.x) < FLT_EPSILON && std::abs(l.y - r.y) < FLT_EPSILON && std::abs(l.z - r.z) < FLT_EPSILON);
	}
};

struct LessThanPredicate {
	bool operator()(const Vector3& l, const Vector3& r)
	{
		if (l.x > r.x) return l.x < r.x;
		if (l.y > r.y) return l.y < r.y;
		return l.z < r.z;
	}
};

ConvexHull* Quickhull::GenerateConvexHull(PointCloud& pointCloud)
{
	ConvexHull* convexHull = new ConvexHull();

	float epsilon = Calculate3DEpsilonFromExtents(pointCloud);
	Vector4 searchDirection = Vector4();
	Simplex simplex = BuildInitialSimplex(pointCloud, searchDirection);

	assert(simplex.size() == 4);

	ConstructInitialHullFromSimplex(*convexHull, pointCloud, simplex, searchDirection);

	ConvexHullVertex* conflictVertex = nullptr;
	ConvexHullFace* conflictFace = nullptr;

	while ((conflictVertex = GetNextConflictVertex(convexHull, conflictFace)))
	{
		AddAndResolveNewVertexInHull(*convexHull, conflictFace, conflictVertex, epsilon);
	}

	return convexHull;
}

ConvexHullVertex* Quickhull::GetNextConflictVertex(ConvexHull*& convexHull, ConvexHullFace*& conflictVertexFace)
{
	ConvexHullVertex* vertex =  nullptr;
	conflictVertexFace = nullptr;
	float furthestDistance = 0.0f;

	for (ConvexHullFace* subjectFace : convexHull->Faces)
	{
		Plane facePlane = GetNormalisedSurfacePlaneFromHullFace(*subjectFace);

		for (ConvexHullVertex* conflictVertex : subjectFace->ConflictList)
		{
			float distanceToPlane = Dot(facePlane.Normal, conflictVertex->Vertex);
			//point is furthest
			if (distanceToPlane > furthestDistance)
			{
				furthestDistance = distanceToPlane;
				conflictVertexFace = subjectFace;
				vertex = conflictVertex;
			}
		}
	}

	//remove result from face
	if (conflictVertexFace != nullptr && vertex != nullptr)
	{
		std::erase(conflictVertexFace->ConflictList, vertex);
	}

	return vertex;
}

bool Quickhull::IsFaceVisible(const ConvexHullFace& face, const ConvexHullVertex& eyeVertex, const float& scaledEpsilon)
{
	Plane facePlane = GetNormalisedSurfacePlaneFromHullFace(face);
	return Dot(facePlane.Normal, eyeVertex.Vertex) > -scaledEpsilon;
}

/// <summary>
/// Determine the horizon using a depth first search 
/// </summary>
void Quickhull::DetermineHorizonRecursiveSearch(std::unordered_set<ConvexHullFace*>& visitedFaces, std::list<ConvexHullHalfEdge*>& horizon, std::vector<ConvexHullFace*>& visibleFaceList, ConvexHullFace* conflictFace, ConvexHullVertex* conflictVertex, const float& scaledEpsilon)
{
	//If we havent see the face before in the search, drop it in.
	if (visitedFaces.find(conflictFace) == visitedFaces.end())
	{
		visibleFaceList.push_back(conflictFace);
		visitedFaces.insert(conflictFace);

		std::unordered_set<ConvexHullHalfEdge*> visitedEdgs;

		ConvexHullHalfEdge* edge = conflictFace->Edge;
		//Loop through all edges until we find ourselves.

		while (visitedEdgs.find(edge) == visitedEdgs.end())
		{
			visitedEdgs.insert(edge);

			//Is the paired face visible?
			if (IsFaceVisible(*edge->Twin->Face, *conflictVertex, scaledEpsilon))
			{
				DetermineHorizonRecursiveSearch(visitedFaces, horizon, visibleFaceList, conflictFace, conflictVertex, scaledEpsilon);
			}
			else
			{
				horizon.push_back(edge);
			}

			edge = edge->Next;
		}
	}
}

void Quickhull::BuildNewFaces(std::list<ConvexHullHalfEdge*>& horizon, std::vector<ConvexHullFace*>& newFaces, ConvexHull& convexHull, ConvexHullVertex*& conflictVertex)
{
	std::vector<ConvexHullHalfEdge*> newEdges = std::vector<ConvexHullHalfEdge*>();

	//pairs newly created edges with their twins
	const auto& FindTwinA = [&](ConvexHullHalfEdge* edge)->ConvexHullHalfEdge*
		{
			const auto twin{ std::find_if(newEdges.begin(), newEdges.end(), [&](ConvexHullHalfEdge* potentialTwin)->bool
				{
					//edge begins at eye, find twin edge that terminates at the vertex and begins at the shared horizon vertex
					if (edge->Tail->VertexID == conflictVertex->VertexID)
						return potentialTwin->Tail == edge->Next->Tail;

					//edge terminates at eye, find twin edge that terminates ate shared horizon vertex beginning from eye
					return potentialTwin->Tail == conflictVertex && potentialTwin->Prev->Next->Tail == edge->Tail;
				}) };

			if (twin == newEdges.end())
				return nullptr;

			return *twin;
		};


	//Triangle construct using edge as base.
	for (ConvexHullHalfEdge* edge : horizon)
	{
		//Determine new edges for face.
		ConvexHullHalfEdge* edge0 = new ConvexHullHalfEdge();
		ConvexHullHalfEdge* edge1 = edge;
		ConvexHullHalfEdge* edge2 = new ConvexHullHalfEdge();

		//Create face.
		ConvexHullFace* face = new ConvexHullFace();
		face->Edge = edge0;
		edge0->Face = face;
		edge1->Face = face;
		edge2->Face = face;

		//Put eye vertex on far point
		edge0->Prev = edge2;
		edge0->Next = edge1;
		edge0->Tail = conflictVertex;

		//Avoid dangin pointer by assigning before changing horizon.
		edge2->Tail = edge->Next->Tail;

		//Attach remainder of edges.
		edge1->Prev = edge0;
		edge1->Next = edge2;

		edge2->Prev = edge1;
		edge2->Next = edge0;

		newEdges.push_back(edge0);
		newEdges.push_back(edge2);
		newFaces.push_back(face);
		convexHull.AddFace(face);
		
		//Setting up twin pairs for new edges.
		/*for (ConvexHullHalfEdge* newEdge : newEdges)
		{
			newEdge->Twin = FindTwinEdgeOfEyeVertex(newEdges, newEdge, conflictVertex);

			if (newEdge->Twin != nullptr)
			{
				newEdge->Twin->Twin = newEdge;
			}
			else
			{
				printf("miss");
			}
		}*/

		for (auto& newEdge : newEdges)
		{
			newEdge->Twin = FindTwinA(newEdge);
			if (newEdge->Twin)
				newEdge->Twin->Twin = newEdge;
		}
	}

	

	//todo : potential issues here!
}

bool Quickhull::AreFacesConvex(const ConvexHullFace& faceA, const ConvexHullFace& faceB, const float& scaledEpsilon)
{
	//calculate centroid	
	Vector3 centre = Vector3(0.0f, 0.0f, 0.0f);
	int centroidVertexCount = GetFaceVertexCount(faceA);
	ConvexHullHalfEdge* faceEdge = faceA.Edge;

	for (int edge = 0; edge < centroidVertexCount; ++edge)
	{
		centre.x += faceEdge->Tail->Vertex.x;
		centre.y += faceEdge->Tail->Vertex.y;
		centre.z += faceEdge->Tail->Vertex.z;

		faceEdge = faceEdge->Next;
	}

	centre.x /= centroidVertexCount;
	centre.y /= centroidVertexCount;
	centre.z /= centroidVertexCount;

	//calculate face plane		
	Plane surfacePlane = GetNormalisedSurfacePlaneFromHullFace(faceB);

	return Dot(surfacePlane.Normal, centre) < -scaledEpsilon;
}

int Quickhull::GetFaceVertexCount(const ConvexHullFace& face)
{
	int result{ 1 };
	for (auto edge{ face.Edge->Next }; edge != face.Edge; edge = edge->Next)
		++result;

	return result;

	//int vertexCount = 1;
	//for (ConvexHullHalfEdge* edge = face.Edge->Next; edge != face.Edge; edge = edge->Next)
	//{
	//	++vertexCount;
	//}
	//return vertexCount;
}

void Quickhull::MergeConcaveFaces(ConvexHull& convexHull, ConvexHullFace& conflictFace, ConvexHullHalfEdge*& edge)
{
	//Dangler.
	edge->Face->Edge = edge->Prev;

	//loop through old edges and collate into new face.
	for (ConvexHullHalfEdge* twinEdge = edge->Twin->Next; twinEdge != edge->Twin; twinEdge = twinEdge->Next)
	{
		twinEdge->Face = edge->Face;
	}

	//Linkup edges :)
	edge->Prev->Next = edge->Twin->Next;
	edge->Next->Prev = edge->Twin->Prev;
	edge->Twin->Prev->Next = edge->Next;
	edge->Twin->Next->Prev = edge->Prev;

	//Very shit cleanup.
	ConvexHullFace* twinFace = edge->Twin->Face;
	convexHull.RemoveFace(twinFace);
	edge->Dead = true;
	edge->Twin->Dead = true;
	edge->Twin->Face->Dead = true;

	//delete edge->Twin;
	//edge->Twin = nullptr;
	//delete edge;
	//edge = nullptr;
	//if deallocated face has any conflict points add them to the conflict face to be resolved with other orphaned points
	conflictFace.ConflictList.insert(conflictFace.ConflictList.end(), twinFace->ConflictList.begin(), twinFace->ConflictList.end());
	//delete twinFace;
	//twinFace = nullptr;
}

void Quickhull::MergeFaces(std::vector<ConvexHullFace*>& newFaces, ConvexHull& convexHull, ConvexHullFace*& conflictFace, const float& scaledEpsilon)
{
	size_t faceCount = newFaces.size();
	for (size_t i = 0; i < faceCount; i++)
	{
		ConvexHullFace* face = newFaces[i];

		if (face == nullptr)
			continue;

		std::unordered_set<ConvexHullHalfEdge*> visitedEdges;
		ConvexHullHalfEdge* edge = face->Edge;

		//Loop through edges, comparing to remove concavity.
		while (visitedEdges.find(edge) == visitedEdges.end())
		{
			visitedEdges.insert(edge);

			if (edge->Face->Dead || edge->Twin->Face->Dead)
			{
				edge = edge->Next;
				continue;
			}

			//If we have a convex face both side then we dont need to merge.
			bool isFaceConvex = AreFacesConvex(*edge->Face, *edge->Twin->Face, scaledEpsilon);
			bool isTwinFaceConvex = AreFacesConvex(*edge->Twin->Face, *edge->Face, scaledEpsilon);

			if (isFaceConvex && isTwinFaceConvex)
			{
				edge = edge->Next;
				continue;
			}

			//One/Both faces are concave.
			MergeConcaveFaces(convexHull, *conflictFace, edge);

			//Check if the merging has created a geometric invariances.
			ConvexHullHalfEdge* start = face->Edge;
			ConvexHullHalfEdge* current = start;

			//Loop through all connecting edges.
			do 
			{
				ConvexHullHalfEdge* next = current->Next;
				ConvexHullFace* currentFace = current->Face;
				ConvexHullFace* nextFace = next->Face;

				if (currentFace == nextFace)
				{
					int twinFaceCount = GetFaceVertexCount(*nextFace);

					printf("v %i f %i\n", (int)convexHull.Vertices.size(), (int)convexHull.Faces.size());

					if (twinFaceCount >= 4)
					{
						FixAdditionalPointInvariance(convexHull, conflictFace, current, next);
					}
					else
					{
						FixInternalPointInvariance(convexHull, conflictFace, current, next);
					}

					start = face->Edge;
					current = face->Edge;

					printf("v %i f %i\n", (int)convexHull.Vertices.size(), (int)convexHull.Faces.size());
				}

				current = current->Next;

			} while (current != start);

			visitedEdges.clear();
			edge = edge->Next;
		}
	}
}

/// <summary>
/// Face has more than 3 vertices so remove one of the edges and remerge.
/// </summary>
void Quickhull::FixAdditionalPointInvariance(ConvexHull& convexHull, ConvexHullFace*& conflictFace, ConvexHullHalfEdge*& incoming, ConvexHullHalfEdge*& outgoing)
{
	UNREFERENCED_PARAMETER(conflictFace);

	ConvexHullFace* face = incoming->Face;
	face->Edge = incoming->Prev;

	//ConvexHullFace* twinFace = incoming->Twin->Face;
	///*todo : ? */twinFace->Edge = incoming->Twin->Next;
	incoming->Next = outgoing->Next;
	outgoing->Next->Prev = incoming;
	outgoing->Twin->Next = incoming->Twin->Next;
	incoming->Twin->Next->Prev = outgoing->Twin;

	//todo : fix cleanup
	outgoing->Dead = true;
	incoming->Twin->Dead = true;
	convexHull.RemoveVertex(outgoing->Tail);
	//delete outgoing->Tail;
	//outgoing->Tail = nullptr;
	incoming->Twin = outgoing->Twin;
	outgoing->Twin->Twin = incoming;
}

//Fixing interal point invariance by merging the spare edge into a face.
void Quickhull::FixInternalPointInvariance(ConvexHull& convexHull, ConvexHullFace*& conflictFace, ConvexHullHalfEdge*& edgeA, ConvexHullHalfEdge*& edgeB)
{
	ConvexHullFace* face = edgeA->Face;
	face->Edge = edgeA->Prev;

	//Finding opposite edge from triangel.
	ConvexHullHalfEdge* thirdEdge = edgeA->Twin;
	while (thirdEdge == edgeB->Twin || thirdEdge == edgeA->Twin)
	{
		thirdEdge = thirdEdge->Next;
	}

	//Repointing surrounding edges.
	edgeA->Prev->Next = thirdEdge;
	edgeB->Next->Prev = thirdEdge;
	thirdEdge->Prev = edgeA->Prev;
	thirdEdge->Next = edgeB->Next;
	thirdEdge->Face = face;

	ConvexHullFace* twinFace = edgeA->Twin->Face;
	conflictFace->ConflictList.insert(conflictFace->ConflictList.end(), twinFace->ConflictList.begin(), twinFace->ConflictList.end());

	//todo : fix cleanup.
	convexHull.RemoveVertex(edgeA->Tail);

	//delete edgeA;
	//edgeA = nullptr;
	edgeA->Dead = true;
	//delete edgeB;
	//edgeB = nullptr;
	edgeB->Dead = true;

	//delete edgeA->Twin;
	//edgeA->Twin = nullptr;
	edgeA->Twin->Dead = true;

	//delete edgeB->Twin;
	//edgeB->Twin = nullptr;
	edgeB->Twin->Dead = true;

	//delete edgeB->Tail;
	//edgeB->Tail = nullptr;
	edgeB->Tail->Dead = true;

	//delete edgeA->Twin->Face;
	//edgeA->Twin->Face = nullptr;
	edgeA->Twin->Face->Dead = true;
}

void Quickhull::UpdateExistingFaces(std::vector<ConvexHullFace*>& newFaces, std::vector<ConvexHullFace*>& visibleFaces, ConvexHull& convexHull, ConvexHullFace*& conflictFace)
{
	UNREFERENCED_PARAMETER(newFaces);
	UNREFERENCED_PARAMETER(visibleFaces);
	UNREFERENCED_PARAMETER(convexHull);
	UNREFERENCED_PARAMETER(conflictFace);
}

void Quickhull::AddAndResolveNewVertexInHull(ConvexHull& convexHull, ConvexHullFace*& conflictFace, ConvexHullVertex*& conflictVertex, const float& scaledEpsilon)
{
	convexHull.AddVertex(conflictVertex);

	//Build new horizon
	std::list<ConvexHullHalfEdge*> horizon = std::list<ConvexHullHalfEdge*>();
	std::vector<ConvexHullFace*> visibleFaces = std::vector<ConvexHullFace*>();
	std::unordered_set<ConvexHullFace*> visitedFaces = std::unordered_set<ConvexHullFace*>();
	DetermineHorizonRecursiveSearch(visitedFaces, horizon, visibleFaces, conflictFace, conflictVertex, scaledEpsilon);

	assert(horizon.size() != 0);

	std::vector<ConvexHullFace*> newFaces = std::vector<ConvexHullFace*>();
	BuildNewFaces(horizon, newFaces, convexHull, conflictVertex);

	//Merge Faces
	MergeFaces(newFaces, convexHull, conflictFace, scaledEpsilon);

	//remove old faces
	//std::erase_if()?

	//tie into existing hull faces
	UpdateExistingFaces(newFaces, visibleFaces, convexHull, conflictFace);

	//Ensure convexity?
	//EnsureConvexity(convexHull);
}

Plane Quickhull::GetNormalisedSurfacePlaneFromHullFace(const ConvexHullFace& face)
{
	Plane result;

	int vertexCount = GetFaceVertexCount(face);

	if (vertexCount > 3)
	{
		//todo : implement
		//result = NewellPlane(planarVertexCount, face);
		throw;
	}
	else
	{
		assert(vertexCount == 3);
		Triangle triangle;
		triangle.Vertices[0] = face.Edge->Tail->Vertex;
		triangle.Vertices[1] = face.Edge->Next->Tail->Vertex;
		triangle.Vertices[2] = face.Edge->Prev->Tail->Vertex;
		result = GetPlaneFromTriangle(triangle);		
	}

	return result;
}

// Calculating a scaled epsilon relative to the max size of the model.
// FLT_EPSILON is not relevant enough for a really big model and not big enough for a teeny tiny one.
const float Quickhull::Calculate3DEpsilonFromExtents(const PointCloud& pointCloud)
{
	Vector3 furthestPoint = { -INFINITY, -INFINITY, -INFINITY };

	for (const Vector3& point : pointCloud)
	{
		furthestPoint.x = std::max(furthestPoint.x, std::abs(point.x));
		furthestPoint.y = std::max(furthestPoint.y, std::abs(point.y));
		furthestPoint.z = std::max(furthestPoint.z, std::abs(point.z));
	}

	return 3 * (furthestPoint.x + furthestPoint.y + furthestPoint.z) * FLT_EPSILON;
}

const Simplex Quickhull::BuildInitialSimplex(const PointCloud& pointCloud, Vector4& searchDirection)
{
	size_t pointCount = pointCloud.size();

	Simplex initialHull = Simplex();

	std::pair<int, int> furthestPointPairIndices = { -1, -1 };
	float furthestDistance = -INFINITY;

	Vector3 vertexDifference = { 0.0f, 0.0f, 0.0f };

	for (size_t x = 0; x < pointCount; x++)
	{
		for (size_t y = 0; y < pointCount; y++)
		{
			const Vector3& vertexA = pointCloud[x];
			const Vector3& vertexB = pointCloud[y];
			vertexDifference = { vertexB.x - vertexA.x, vertexB.y - vertexA.y, vertexB.z - vertexA.z };
			float distance = MagnitudeSqr(vertexDifference);

			if (distance >= furthestDistance)
			{
				furthestDistance = distance;
				furthestPointPairIndices.first = (int)x;
				furthestPointPairIndices.second = (int)y;
			}
		}
	}

	assert(furthestPointPairIndices.first != -1);
	assert(furthestPointPairIndices.second != -1);

	//Construct a line between the two furthest points.
	initialHull.push_back(pointCloud[furthestPointPairIndices.first]);
	initialHull.push_back(pointCloud[furthestPointPairIndices.second]);

	Vector3 simplex01 = 
		{ initialHull[1].x - initialHull[0].x,
		  initialHull[1].y - initialHull[0].y,
		  initialHull[1].z - initialHull[0].z };
	Normalise(simplex01);

	int furthestPointFromLineIndex = -1;
	float furthestPointFromLineDistance = -INFINITY;
	Vector3 furthestPointFromLineDirection = Vector3();

	//Finding furthest point from line.
	for (size_t i = 0; i < pointCount; i++)
	{
		if (i == furthestPointPairIndices.first || i == furthestPointPairIndices.second)
			continue;

		const Vector3& p = pointCloud[i];
		Vector3 pointToLineStart = { p.x - initialHull[0].x, p.y - initialHull[0].y, p.z - initialHull[0].z };
		Vector3 cross = Cross(simplex01, pointToLineStart);
		float lengthSqr = MagnitudeSqr(cross);

		if (lengthSqr > furthestPointFromLineDistance)
		{
			furthestPointFromLineIndex = (int)i;
			furthestPointFromLineDistance = lengthSqr;
			furthestPointFromLineDirection = cross;
		}
	}

	assert(furthestPointFromLineIndex != -1);

	int thirdPointIndex = furthestPointFromLineIndex;
	initialHull.push_back(pointCloud[furthestPointFromLineIndex]);
	
	//Recompute normal to ensure its flipped.
	Normalise(furthestPointFromLineDirection);
	float dot = Dot(furthestPointFromLineDirection, simplex01);
	Vector3 scaledNormalAlong01 = MultiplyScalar(dot, simplex01);
	Vector3 delta = {
		furthestPointFromLineDirection.x - scaledNormalAlong01.x,
		furthestPointFromLineDirection.y - scaledNormalAlong01.y,
		furthestPointFromLineDirection.z - scaledNormalAlong01.z };
	furthestPointFromLineDirection = Normalised(delta);

	furthestPointFromLineIndex = -1;
	furthestPointFromLineDistance = -INFINITY;
	float simplex3Dist = Dot(furthestPointFromLineDirection, initialHull[2]);

	//Finding furthest point from line (except simplex[2]).
	for (size_t i = 0; i < pointCount; i++)
	{
		if (i == furthestPointPairIndices.first || i == furthestPointPairIndices.second || i == thirdPointIndex)
			continue;

		//Projected distance of point onto new normal.
		float distance = std::abs(Dot(pointCloud[i], furthestPointFromLineDirection));
		
		if (distance > furthestPointFromLineDistance)
		{
			furthestPointFromLineDistance = distance;
			furthestPointFromLineIndex = (int)i;
		}
	}

	assert(furthestPointFromLineIndex != -1);
	initialHull.push_back(pointCloud[furthestPointFromLineIndex]);

	searchDirection =
	{
		furthestPointFromLineDirection.x,
		furthestPointFromLineDirection.y,
		furthestPointFromLineDirection.z,
		simplex3Dist
	};

	return initialHull;
}

void Quickhull::ConstructInitialHullFromSimplex(ConvexHull& convexHull, PointCloud& pointCloud, const Simplex& simplex, const Vector4& constructionDirection)
{
	static EqualPredicate equal;
	static LessThanPredicate lessThan;

	std::_Erase_remove_if(pointCloud, [&](const auto& p) {return std::find(simplex.begin(), simplex.end(), p) != simplex.end(); });

	////Remove simplex from point cloud list
	//for (size_t i = 0; i < simplex.size(); i++)
	//{
	//	const Vector3& p = simplex[i];
	//	auto itr = std::find_if(pointCloud.begin(), pointCloud.end(), [&p](const Vector3& arg) { return equal(arg, p); });

	//	if (itr != pointCloud.end())
	//	{
	//		pointCloud.erase(itr);
	//	}
	//}

	//std::unique erasing requires a sorted list.
	std::sort(pointCloud.begin(), pointCloud.end(), lessThan);
	
	//Remove duplicates (happens with indexed business.
	pointCloud.erase(std::unique(pointCloud.begin(), pointCloud.end(), equal), pointCloud.end());

	ConvexHullVertex* p0 = new ConvexHullVertex();
	p0->Vertex = simplex[0];
	convexHull.AddVertex(p0);

	ConvexHullVertex* p1 = new ConvexHullVertex();
	p1->Vertex = simplex[1];
	convexHull.AddVertex(p1);

	ConvexHullVertex* p2 = new ConvexHullVertex();
	p2->Vertex = simplex[2];
	convexHull.AddVertex(p2);

	ConvexHullVertex* p3 = new ConvexHullVertex();
	p3->Vertex = simplex[3];
	convexHull.AddVertex(p3);

	ConvexHullFace* simplexFaces[4];

	Vector3 constructionPlaneNormal = { constructionDirection.x, constructionDirection.y, constructionDirection.z };
	float constructionPlaneOffset = constructionDirection.w;

	//Determine winding order based on side of construction plane.
	//Clockwise.
	if (Dot(simplex[3], constructionPlaneNormal) - constructionPlaneOffset < 0)
	{
		simplexFaces[0] = CreateHullFace(convexHull, p0, p1, p2);
		simplexFaces[1] = CreateHullFace(convexHull, p3, p1, p0);
		simplexFaces[2] = CreateHullFace(convexHull, p3, p2, p1);
		simplexFaces[3] = CreateHullFace(convexHull, p3, p0, p2);
	}
	else
	{
		simplexFaces[0] = CreateHullFace(convexHull, p0, p2, p1);
		simplexFaces[1] = CreateHullFace(convexHull, p3, p0, p1);
		simplexFaces[2] = CreateHullFace(convexHull, p3, p1, p2);
		simplexFaces[3] = CreateHullFace(convexHull, p3, p2, p0);
	}

	//Populate twin edges
	//todo : ensure this isn't broken.
	for (ConvexHullFace* subjectFace : convexHull.Faces)
	{
		ConvexHullHalfEdge* edge = subjectFace->Edge;

		do
		{
			if (edge->Twin != nullptr)
			{
				edge = edge->Next;
				continue;
			}

			edge->Twin = FindTwinEdge(convexHull, edge);
			assert(edge->Twin);
			edge->Twin->Twin = edge;
			edge = edge->Next;
		} while (edge != subjectFace->Edge);
	}
	
	//Put all remaining points into the conflict lists.
	//for (auto itr = pointCloud.begin(); itr != pointCloud.end();)
	for (size_t i = 0; i < pointCloud.size(); i++)
	{
		ConvexHullFace* furthestFace = nullptr;
		float furthestDistance = 0.0f;
		Vector3 point = pointCloud[i];
		//Vector3 point = *itr;

		for (ConvexHullFace* subjectFace : convexHull.Faces)
		{
			Plane facePlane = GetNormalisedSurfacePlaneFromHullFace(*subjectFace);
			float distanceToPlane = Dot(facePlane.Normal, point);

			if (distanceToPlane > furthestDistance)
			{
				furthestDistance = distanceToPlane;
				furthestFace = subjectFace;
			}

			if (furthestFace == nullptr)
			{
				//Means distance always less than 0, point is inside hull.
				continue;
			}

			ConvexHullVertex* vertex = new ConvexHullVertex();
			vertex->Vertex = point;
			furthestFace->ConflictList.push_back(vertex);
			//itr = pointCloud.erase(itr);
		}
		assert(furthestFace != nullptr);
	}
}

//Create a new face and setup initial edges.
ConvexHullFace* Quickhull::CreateHullFace(ConvexHull& convexHull, ConvexHullVertex*& vertexA, ConvexHullVertex*& vertexB, ConvexHullVertex*& vertexC)
{
	ConvexHullFace* face = new ConvexHullFace();

	ConvexHullHalfEdge* edges[3] = 
	{
		new ConvexHullHalfEdge(),
		new ConvexHullHalfEdge(),
		new ConvexHullHalfEdge(),
	};

	face->Edge = edges[0];

	edges[0]->Tail = vertexA;
	edges[0]->Face = face;
	edges[0]->Prev = edges[2];
	edges[0]->Next = edges[1];

	edges[1]->Tail = vertexB;
	edges[1]->Face = face;
	edges[1]->Prev = edges[0];
	edges[1]->Next = edges[2];

	edges[2]->Face = face;
	edges[2]->Tail = vertexC;
	edges[2]->Prev = edges[1];
	edges[2]->Next = edges[0];

	convexHull.AddFace(face);

	return face;
}

// Clock it twin!!
// Finds the back facing twin edge for a specific edge.
ConvexHullHalfEdge* Quickhull::FindTwinEdge(const ConvexHull& convexHull, const ConvexHullHalfEdge* edge)
{
	ConvexHullFace* face = convexHull.Faces[0];

	//todo : convert to face : hull.faces itr
	do
	{
		if (face == edge->Face)
		{
			//if self, dont check face.
			face = face->Next;
			continue;
		}

		ConvexHullHalfEdge* possibleTwin = face->Edge;

		do
		{
			//do our tails match each direction?
			//Disgusting linked list stuff.
			if (edge->Tail == possibleTwin->Next->Tail &&
				edge->Next->Tail == possibleTwin->Tail)
			{
				return possibleTwin;
			}

			possibleTwin = possibleTwin->Next;
		} while (possibleTwin != face->Edge);

		face = face->Next;
	} while (face != convexHull.Faces[0]);

	throw new std::exception("Failed to find a twin edge.");
	//return nullptr;
}

/// Finds edge twin pairs within a given list rather than the hull.
ConvexHullHalfEdge* Quickhull::FindTwinEdgeOfEyeVertex(const std::vector<ConvexHullHalfEdge*>& edgeList, const ConvexHullHalfEdge*& edge, const ConvexHullVertex*& eyeVertex)
{
	ConvexHullHalfEdge* twinEdge = nullptr;

	//todo : make this more readable. its gross.
	auto itr = std::find_if(edgeList.begin(), edgeList.end(),

		[&](ConvexHullHalfEdge* potentialTwin)->bool
		{
			//If we're the other edge starting at the eye vertex.
			if (edge->Tail == eyeVertex)
			{
				return potentialTwin->Tail == edge->Next->Tail;
			}

			//Check we arent facing the other way instead.
			return potentialTwin->Tail == eyeVertex && potentialTwin->Next->Tail == edge->Tail;
		});
	
	if (itr != edgeList.end())
	{
		twinEdge = *itr;
	}

	return twinEdge;
}

void ConvexHull::AddVertex(ConvexHullVertex* vertex)
{
	Vertices.push_back(vertex);

	int vertexCount = (int)Vertices.size();
	if (vertexCount > 1)
	{
		ConvexHullVertex* back = Vertices[vertexCount - 2];
		back->Next = vertex;
		vertex->Prev = back;
	}

	Vertices[0]->Prev = vertex;
	vertex->Next = Vertices[0];
}

void ConvexHull::AddFace(ConvexHullFace* face)
{
	Faces.push_back(face);

	int faceCount = (int)Faces.size();
	if (faceCount > 1)
	{
		ConvexHullFace* back = Faces[faceCount - 2];
		back->Next = face;
		face->Prev = back;
	}

	Faces[0]->Prev = face;
	face->Next = Faces[0];
}

void ConvexHull::RemoveVertex(ConvexHullVertex* vertex)
{
	auto itr = std::find(Vertices.begin(), Vertices.end(), vertex);
	Vertices.erase(itr);
	//std::erase(Vertices, itr);
}

void ConvexHull::RemoveFace(ConvexHullFace* face)
{
	auto itr = std::find(Faces.begin(), Faces.end(), face);
	Faces.erase(itr);
	//std::erase(Faces, itr);
}
