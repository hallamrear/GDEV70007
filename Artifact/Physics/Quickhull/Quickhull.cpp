#include "pch.h"
#include "Quickhull.h"

#define TINY_FLOAT (0.00001f)

using namespace Maths;

// Source - https://stackoverflow.com/a/58717895
// Posted by 463035818_is_not_an_ai, modified by community. See post 'Timeline' for change history
// Retrieved 2026-05-30, License - CC BY-SA 4.0
struct EqualPredicate {
	bool operator()(const Vector3& l, const Vector3& r)
	{
		//return (std::abs(l.x - r.x) < FLT_EPSILON && std::abs(l.y - r.y) < FLT_EPSILON && std::abs(l.z - r.z) < FLT_EPSILON);
		
		if ((l.x == r.x) && (l.y == r.y) && (l.z == r.z))
			return true;
		else
			return false;
	}
};

struct LessThanPredicate {
	bool operator()(const Vector3& l, const Vector3& r)
	{
		if (l.x != r.x)
			return l.x < r.x;
		if (l.y != r.y)
			return l.y < r.y;
		return l.z < r.z;
	}
};

void PrintConflictList(ConvexHull& convexHull)
{
	UNREFERENCED_PARAMETER(convexHull);
	//HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	//SetConsoleTextAttribute(hConsole, 11);
	//
	//printf("Face conflict list list START\n");
	//ConvexHullFace* f = convexHull.FacesListHead;
	//do
	//{
	//	printf("Face %p -> %i\n", f, (int)f->ConflictList.size());
	//
	//	for (size_t i = 0; i < f->ConflictList.size(); i++)
	//	{
	//		Vector3 point = f->ConflictList[i]->Vertex;
	//		printf("Point -> %f %f %f\n", point.x, point.y, point.z);
	//	}
	//
	//	f = f->Next;
	//} while (f != convexHull.FacesListHead);
	//
	//printf("\n");
	//
	//f = convexHull.FacesListHead;
	//do
	//{
	//	printf("Face %p -> %i\n", f, (int)f->ConflictList.size());
	//	f = f->Next;
	//} while (f != convexHull.FacesListHead);
	//
	//printf("Face conflict list list END\n");
	//SetConsoleTextAttribute(hConsole, 15);
}

static int runs = 0;
ConvexHull* Quickhull::GenerateConvexHull(PointCloud& pointCloud)
{
	runs = 0;
	ConvexHull* convexHull = new ConvexHull((int)pointCloud.size());

	float epsilon = Calculate3DEpsilonFromExtents(pointCloud);
	Vector4 searchDirection = Vector4();
	Simplex simplex = BuildInitialSimplex(pointCloud, searchDirection);

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
	float furthestDistance = 0;

	ConvexHullFace* subjectFace = convexHull->FacesListHead;

	do
	{
		Plane facePlane = GetNormalisedSurfacePlaneFromHullFace(*subjectFace);

		for (ConvexHullVertex* conflictVertex : subjectFace->ConflictList)
		{
			float distanceToPlane = DotPoint(facePlane, conflictVertex->Vertex); 
			//point is furthest
			if (distanceToPlane > furthestDistance)
			{
				furthestDistance = distanceToPlane;
				conflictVertexFace = subjectFace;
				vertex = conflictVertex;
			}
		}

		subjectFace = subjectFace->Next;
	} while (subjectFace != convexHull->FacesListHead);

	//remove result from face
	if (vertex != nullptr)
	{
		std::erase(conflictVertexFace->ConflictList, vertex);
	}

	return vertex;
}

/// Finds edge twin pairs within a given list rather than the hull.
ConvexHullHalfEdge* Quickhull::FindTwinEdgeOfEyeVertex(const std::vector<ConvexHullHalfEdge*>& edgeList, ConvexHullHalfEdge*& edge, ConvexHullVertex*& eyeVertex)
{
	//todo : make this more readable. its gross.
	auto twin = std::find_if(edgeList.begin(), edgeList.end(),

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

	if (twin == edgeList.end())
		return nullptr;

	return *twin;
}

void Quickhull::BuildNewFaces(std::list<ConvexHullHalfEdge*>& horizon, std::vector<ConvexHullFace*>& newFaces, ConvexHull& convexHull, ConvexHullVertex*& conflictVertex)
{
	std::vector<ConvexHullHalfEdge*> newEdges = std::vector<ConvexHullHalfEdge*>();

	//Triangle construct using edge as base.
	for (ConvexHullHalfEdge* edge : horizon)
	{
		//Determine new edges for face.
		ConvexHullHalfEdge* edge0 = convexHull.GetNewEdge();
		ConvexHullHalfEdge* edge1 = edge;
		ConvexHullHalfEdge* edge2 = convexHull.GetNewEdge();

		//Create face.
		ConvexHullFace* face = convexHull.GetNewFace();
		face->VertexCount = 3;

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

		convexHull.AddFaceToHull(face);
		
		//Setting up twin pairs for new edges.
		for (ConvexHullHalfEdge* newEdge : newEdges)
		{
			newEdge->Twin = FindTwinEdgeOfEyeVertex(newEdges, newEdge, conflictVertex);

			if (newEdge->Twin != nullptr)
			{
				newEdge->Twin->Twin = newEdge;
			}
		}
	}

	//todo : potential issues here!
}

bool Quickhull::IsFaceVisible(const ConvexHullFace& face, const ConvexHullVertex& eyeVertex, const float& scaledEpsilon)
{
	Plane facePlane = GetNormalisedSurfacePlaneFromHullFace(face);
	float d = DotPoint(facePlane, eyeVertex.Vertex);
	return d > -scaledEpsilon;
}

bool Quickhull::AreFacesConvex(const ConvexHullFace& faceA, const ConvexHullFace& otherFace, const float& scaledEpsilon)
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
	Plane surfacePlane = GetNormalisedSurfacePlaneFromHullFace(otherFace);

	float d = DotPoint(surfacePlane, centre);
	return d - TINY_FLOAT < -scaledEpsilon;
}

int Quickhull::GetFaceVertexCount(const ConvexHullFace& face)
{
	int vertexCount = 1;
	for (ConvexHullHalfEdge* edge = face.Edge->Next; edge != face.Edge; edge = edge->Next)
	{
		//LOOP BREAK
		assert(vertexCount < 10000);
		++vertexCount;
	}
	return vertexCount;
}

void Quickhull::MergeConcaveFaces(ConvexHull& convexHull, ConvexHullFace& conflictFace, ConvexHullHalfEdge*& edge)
{
	UNREFERENCED_PARAMETER(convexHull);

	//Dangler.
	edge->Face->Edge = edge->Prev;

	//loop through old edges and collate into new face.
	for (auto twinEdge{ edge->Twin->Next }; twinEdge != edge->Twin; twinEdge = twinEdge->Next)
	{
		twinEdge->Face = edge->Face;
	}

	//Linkup edges :)
	edge->Prev->Next = edge->Twin->Next; 		
	edge->Next->Prev = edge->Twin->Prev; 		
	edge->Twin->Prev->Next = edge->Next; 		
	edge->Twin->Next->Prev = edge->Prev; 		

	//Very shit cleanup.
	convexHull.DestroyHalfEdge(edge);
	convexHull.DestroyHalfEdge(edge->Twin);
	convexHull.EdgeCount -= 2;

	//if deallocated face has any conflict points add them to the conflict face to be resolved with other orphaned points
	ConvexHullFace* twinFace = edge->Twin->Face;
	conflictFace.ConflictList.insert(conflictFace.ConflictList.end(), twinFace->ConflictList.begin(), twinFace->ConflictList.end());
	convexHull.DestroyFace(twinFace);
	
	++edge->Face->VertexCount;
}

void Quickhull::MergeFaces(std::vector<ConvexHullFace*>& newFaces, ConvexHull& convexHull, ConvexHullFace*& conflictFace, const float& scaledEpsilon)
{
	size_t faceCount = newFaces.size();

	for (size_t i = 0; i < faceCount; i++)
	{
		ConvexHullFace* face = newFaces[i];

		if (face->Dead)
			continue;

		std::unordered_set<ConvexHullHalfEdge*> visitedEdges;

		ConvexHullHalfEdge* edge = face->Edge;

		//Loop through edges, comparing to remove concavity.
		while (visitedEdges.find(edge) == visitedEdges.end())
		{
			visitedEdges.insert(edge);

			if (edge->Dead || edge->Twin->Face->Dead)
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
			ConvexHullHalfEdge* next = current;

			//Loop through all connecting edges.
			do 
			{
				next = current->Next;

				ConvexHullFace* currentTwinFace = current->Twin->Face;
				ConvexHullFace* nextTwinFace = next->Twin->Face;

				if (currentTwinFace == nextTwinFace)
				{
					int twinFaceCount = GetFaceVertexCount(*currentTwinFace);

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
/// Invariance 2.
/// </summary>
void Quickhull::FixAdditionalPointInvariance(ConvexHull& convexHull, ConvexHullFace*& conflictFace, ConvexHullHalfEdge*& incoming, ConvexHullHalfEdge*& outgoing)
{
	UNREFERENCED_PARAMETER(conflictFace);

	ConvexHullFace* face = incoming->Face;
	face->Edge = incoming->Prev;

	/* todo : ? */
	ConvexHullFace* twinFace = incoming->Twin->Face;
	//twinFace->Edge = incoming->Twin->Next;

	incoming->Next = outgoing->Next;
	outgoing->Next->Prev = incoming;
	outgoing->Twin->Next = incoming->Twin->Next;
	incoming->Twin->Next->Prev = outgoing->Twin;

	//todo : fix cleanup
	convexHull.DestroyHalfEdge(outgoing);
	convexHull.DestroyHalfEdge(incoming->Twin);
	convexHull.EdgeCount -= 2;
	convexHull.DestroyVertex(outgoing->Tail);

	incoming->Twin = outgoing->Twin;
	outgoing->Twin->Twin = incoming;
	--face->VertexCount;
	--twinFace->VertexCount;
}

/// <summary>
/// Fixing interal point invariance by merging the spare edge into a face.
/// Invariance 1.
/// </summary>s
void Quickhull::FixInternalPointInvariance(ConvexHull& convexHull, ConvexHullFace*& conflictFace, ConvexHullHalfEdge*& incoming, ConvexHullHalfEdge*& outgoing)
{
	UNREFERENCED_PARAMETER(convexHull);
	ConvexHullFace* face = incoming->Face;
	face->Edge = incoming->Prev;

	//Finding opposite edge from triangel.
	int hits = 0;
	ConvexHullHalfEdge*& thirdEdge = incoming->Twin;
	while (thirdEdge == outgoing->Twin || thirdEdge == incoming->Twin)
	{
		hits++;
		assert(hits < 100000000000);
		thirdEdge = thirdEdge->Next;
	}

	//Repointing surrounding edges.
	incoming->Prev->Next = thirdEdge;
	outgoing->Next->Prev = thirdEdge;
	thirdEdge->Prev = incoming->Prev;
	thirdEdge->Next = outgoing->Next;
	thirdEdge->Face = face;

	convexHull.DestroyHalfEdge(incoming);
	convexHull.DestroyHalfEdge(outgoing);
	convexHull.DestroyHalfEdge(incoming->Twin);
	convexHull.DestroyHalfEdge(outgoing->Twin);
	convexHull.EdgeCount -= 4;
	convexHull.DestroyVertex(outgoing->Tail);

	ConvexHullFace* twinFace = incoming->Twin->Face;
	conflictFace->ConflictList.insert(conflictFace->ConflictList.end(), twinFace->ConflictList.begin(), twinFace->ConflictList.end());
	convexHull.DestroyFace(incoming->Twin->Face);
	++face->VertexCount;
}

void Quickhull::UpdateExistingFaces(std::vector<ConvexHullFace*>& newFaces, std::vector<ConvexHullFace*>& visibleFaces, ConvexHull& convexHull, ConvexHullFace*& conflictFace)
{
	for (auto&face : visibleFaces)
	{
		if (face->FaceID != conflictFace->FaceID)
		{
			conflictFace->ConflictList.insert(conflictFace->ConflictList.end(), face->ConflictList.begin(), face->ConflictList.end());
		}

		convexHull.DestroyFace(face);
	}

	ResolveOrphanPoints(convexHull, newFaces, conflictFace);
}

void Quickhull::ResolveOrphanPoints(ConvexHull& convexHull, std::vector<ConvexHullFace*>& newFaces, ConvexHullFace*& conflictFace)
{
	//Takin all the face points and reallocating to the furthest new faces.
	for (auto& subjectPoint : conflictFace->ConflictList)
	{
		ConvexHullFace* furthestFace = nullptr;
		float furthestDistance = 0;

		for (auto& face : newFaces)
		{
			Plane facePlane = GetNormalisedSurfacePlaneFromHullFace(*face);
			float distance = DotPoint(facePlane, subjectPoint->Vertex);

			if (distance - TINY_FLOAT > furthestDistance)
			{
				furthestDistance = distance;
				furthestFace = face;
			}
		}

		//canont be an edge point
		if (furthestFace == nullptr)
		{
			convexHull.DestroyVertex(subjectPoint);
			continue;
		}

		furthestFace->ConflictList.push_back(subjectPoint);
	}

	return;
}

void Quickhull::AddAndResolveNewVertexInHull(ConvexHull& convexHull, ConvexHullFace*& conflictFace, ConvexHullVertex*& conflictVertex, const float& scaledEpsilon)
{
	runs++;
	printf("\n\nAddAndResolveNewVertexInHull %i\n", runs);

	convexHull.AddVertexToHull(conflictVertex);

	//Build new horizon

	std::list<ConvexHullHalfEdge*> horizon = std::list<ConvexHullHalfEdge*>();
	std::vector<ConvexHullFace*> visibleFaces = DetermineHorizon(horizon, conflictFace, conflictVertex, scaledEpsilon);

	assert(horizon.size() != 0);

	PrintConflictList(convexHull);

	std::vector<ConvexHullFace*> newFaces = std::vector<ConvexHullFace*>();
	BuildNewFaces(horizon, newFaces, convexHull, conflictVertex);

	PrintConflictList(convexHull);

	//Merge Faces
	MergeFaces(newFaces, convexHull, conflictFace, scaledEpsilon);

	PrintConflictList(convexHull);

	//remove old faces
	//todo : std::erase_if()?
	for (auto itr = newFaces.begin(); itr != newFaces.end();)
	{
		if ((*itr)->Dead)
			itr = newFaces.erase(itr);
		else
			++itr;
	}

	PrintConflictList(convexHull);

	//tie into existing hull faces
	UpdateExistingFaces(newFaces, visibleFaces, convexHull, conflictFace);

	PrintConflictList(convexHull);

	//Ensure convexity?
}

std::vector<ConvexHullFace*>  Quickhull::DetermineHorizon(std::list<ConvexHullHalfEdge*>& horizon, ConvexHullFace*& conflictFace, ConvexHullVertex* conflictVertex, const float& scaledEpsilon)
{
	std::vector<ConvexHullFace*> visibleFaces;
	std::unordered_set<ConvexHullFace*> visitedFaces;
	DetermineHorizonRecursiveSearch(visitedFaces, horizon, visibleFaces, conflictFace, conflictVertex, scaledEpsilon);
	return visibleFaces;
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

			ConvexHullFace* nextFace = edge->Twin->Face;

			//Is the paired face visible?
			if (IsFaceVisible(*nextFace, *conflictVertex, scaledEpsilon))
			{
				DetermineHorizonRecursiveSearch(visitedFaces, horizon, visibleFaceList, nextFace, conflictVertex, scaledEpsilon);
			}
			else
			{
				horizon.push_back(edge);
			}

			edge = edge->Next;
		}
	}
}


Plane Quickhull::CreateNewellPlaneFromTriangle(int planarVertexCount, const ConvexHullFace& face)
{
	Vector3 normal = { 0.0f, 0.0f, 0.0f };
	ConvexHullHalfEdge* faceEdge = face.Edge;

	for (auto edge{ 0 }; edge < planarVertexCount; ++edge)
	{
		Vector3 currentVertex = faceEdge->Tail->Vertex;
		Vector3 nextVertex = faceEdge->Next->Tail->Vertex;
		normal.x += (currentVertex.y - nextVertex.y) * (currentVertex.z + nextVertex.z);
		normal.y += (currentVertex.z - nextVertex.z) * (currentVertex.x + nextVertex.x);
		normal.z += (currentVertex.x - nextVertex.x) * (currentVertex.y + nextVertex.y);
		faceEdge = faceEdge->Next;
	}

	float offset = -Dot(normal, face.Edge->Tail->Vertex);
	normal = Normalised(normal);

	return Plane(normal, offset);
}

Plane Quickhull::GetNormalisedSurfacePlaneFromHullFace(const ConvexHullFace& face)
{
	Plane result;

	int vertexCount = GetFaceVertexCount(face);

	if (vertexCount > 3)
	{
		result = CreateNewellPlaneFromTriangle(vertexCount, face);
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

	result = Normalised(result);
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

	Simplex initialSimplex = Simplex();

	const auto cloudsize{ pointCloud.size() };
	Vector2* pairIndices = new Vector2[cloudsize * cloudsize];
	float* pairDistances{ new float[cloudsize * cloudsize] };

	int ix = 0;
	int iy = 0;
	auto pair{ 0 };
	auto index{ 0 };
	auto currentDistance{ -INFINITY };
	for (auto p0{ 0 }; p0 < cloudsize; ++p0)
		for (auto p1{ 0 }; p1 < cloudsize; ++p1)
		{
			pairIndices[pair] = { (float)p0,(float)p1 };
			const Vector3& vertexA = pointCloud[p0];
			const Vector3& vertexB = pointCloud[p1];
			Vector3 vertexDifference = { vertexB.x - vertexA.x, vertexB.y - vertexA.y, vertexB.z - vertexA.z };
			pairDistances[pair] = MagnitudeSqr(vertexDifference);
			if (pairDistances[pair] >= currentDistance)
			{
				ix = p0;
				iy = p1;
				index = pair;
				currentDistance = pairDistances[pair];
			}
			++pair;
		}


	//Construct a line between the two furthest points.
	Vector3 p0{ pointCloud[(int)pairIndices[index].x] };
	Vector3 p1{ pointCloud[(int)pairIndices[index].y] };
	std::pair<int, int> furthestPointPairIndices = { (int)pairIndices[index].x, (int)pairIndices[index].y };
	delete[] pairDistances;
	delete[] pairIndices;

	//Construct a line between the two furthest points.
	initialSimplex.PushBack(pointCloud[furthestPointPairIndices.first]);
	initialSimplex.PushBack(pointCloud[furthestPointPairIndices.second]);

	Vector3 AB = initialSimplex.Points[1] - initialSimplex.Points[0];
	Normalise(AB);

	int furthestPointFromLineIndex = -1;
	float furthestPointFromLineDistance = 0.0f;
	Vector3 furthestPointFromLineDirection = Vector3();
	EqualPredicate equal;

	//Finding furthest point from line.
	for (size_t i = 0; i < pointCount; i++)
	{
		if(i == furthestPointPairIndices.first || i == furthestPointPairIndices.second)
			continue;

		const Vector3& p = pointCloud[i];
		Vector3 pointToLineStart = p - initialSimplex.Points[0];
		Vector3 cross = Cross(AB, pointToLineStart);
		float lengthSqr = MagnitudeSqr(cross);

		if (lengthSqr - TINY_FLOAT > furthestPointFromLineDistance &&
			p != initialSimplex.Points[0] && p != initialSimplex.Points[1])
		{
			initialSimplex.Points[2] = pointCloud[i];
			furthestPointFromLineIndex = (int)i;
			furthestPointFromLineDistance = lengthSqr;
			furthestPointFromLineDirection = cross;
		}
	}

	assert(furthestPointFromLineIndex != -1);

	//initialSimplex.push_back(pointCloud[furthestPointFromLineIndex]);
	
	//Recompute normal to ensure its flipped.
	Normalise(furthestPointFromLineDirection);
	float dot = Dot(furthestPointFromLineDirection, AB);
	Vector3 scaledNormalAlong01 = MultiplyScalar(dot, AB);
	Vector3 delta =
	{
		furthestPointFromLineDirection.x - scaledNormalAlong01.x,
		furthestPointFromLineDirection.y - scaledNormalAlong01.y,
		furthestPointFromLineDirection.z - scaledNormalAlong01.z };

	furthestPointFromLineDirection = Normalised(delta);

	furthestPointFromLineIndex = -1;
	furthestPointFromLineDistance = 0.0f;
	float simplex3Dist = Dot(furthestPointFromLineDirection, initialSimplex.Points[2]);

	//Finding furthest point from line (except simplex[2]).
	for (size_t i = 0; i < pointCount; i++)
	{
		if ((equal(pointCloud[i], initialSimplex.Points[0])) || (equal(pointCloud[i], initialSimplex.Points[1])) || (equal(pointCloud[i], initialSimplex.Points[2])))
			continue;

		//Projected distance of point onto new normal.
		float distance = std::abs(Dot(pointCloud[i], furthestPointFromLineDirection));
		
		if (distance - TINY_FLOAT > furthestPointFromLineDistance)
		{
			furthestPointFromLineDistance = distance;
			furthestPointFromLineIndex = (int)i;
		}
	}

	assert(furthestPointFromLineIndex != -1);
	initialSimplex.Points[3] = pointCloud[furthestPointFromLineIndex];

	searchDirection =
	{
		furthestPointFromLineDirection.x,
		furthestPointFromLineDirection.y,
		furthestPointFromLineDirection.z,
		simplex3Dist
	};

	for (int i = 0; i < 4; i++)
	{
		printf("Simplex %i - %f %f %f\n", i, initialSimplex.Points[i].x, initialSimplex.Points[i].y, initialSimplex.Points[i].z);
	}

	return initialSimplex;
}

void Quickhull::ConstructInitialHullFromSimplex(ConvexHull& convexHull, PointCloud& pointCloud, const Simplex& simplex, const Vector4& constructionDirection)
{
	static EqualPredicate equal;
	static LessThanPredicate lessThan;

	////Remove simplex from point cloud list

	for (int i = 0; i < 4; i++)
	{
		for (auto itr = pointCloud.begin(); itr != pointCloud.end();)
		{
			if (equal((*itr), simplex.Points[i]))
				itr = pointCloud.erase(itr);
			else
				++itr;
		}
	}

	//std::unique erasing requires a sorted list.
	std::sort(pointCloud.begin(), pointCloud.end(), lessThan);
	
	//Remove duplicates (happens with indexed business.
	pointCloud.erase(std::unique(pointCloud.begin(), pointCloud.end(), equal), pointCloud.end());

	ConvexHullVertex* p0 = convexHull.GetNewVertex();
	p0->Vertex = simplex.Points[0];
	convexHull.AddVertexToHull(p0);

	ConvexHullVertex* p1 = convexHull.GetNewVertex();
	p1->Vertex = simplex.Points[1];
	convexHull.AddVertexToHull(p1);

	ConvexHullVertex* p2 = convexHull.GetNewVertex();
	p2->Vertex = simplex.Points[2];
	convexHull.AddVertexToHull(p2);

	ConvexHullVertex* p3 = convexHull.GetNewVertex();
	p3->Vertex = simplex.Points[3];
	convexHull.AddVertexToHull(p3);

	Vector3 constructionPlaneNormal = { constructionDirection.x, constructionDirection.y, constructionDirection.z };
	float constructionPlaneOffset = constructionDirection.w;

	//Determine winding order based on side of construction plane.
	//Clockwise.
	ConvexHullFace* newFaces[4] = { nullptr };

	if (Dot(simplex.Points[3], constructionPlaneNormal) - constructionPlaneOffset < 0)
	{
		newFaces[0] = CreateHullFace(convexHull, p0, p1, p2);
		newFaces[1] = CreateHullFace(convexHull, p3, p1, p0);
		newFaces[2] = CreateHullFace(convexHull, p3, p2, p1);
		newFaces[3] = CreateHullFace(convexHull, p3, p0, p2);
	}
	else
	{
		newFaces[0] = CreateHullFace(convexHull, p0, p2, p1);
		newFaces[1] = CreateHullFace(convexHull, p3, p0, p1);
		newFaces[2] = CreateHullFace(convexHull, p3, p1, p2);
		newFaces[3] = CreateHullFace(convexHull, p3, p2, p0);
	}

	convexHull.AddFaceToHull(newFaces[0]);
	convexHull.AddFaceToHull(newFaces[1]);
	convexHull.AddFaceToHull(newFaces[2]);
	convexHull.AddFaceToHull(newFaces[3]);

	//Populate twin edges
	//todo : ensure this isn't broken.
	ConvexHullFace* loopFace = convexHull.FacesListHead;
	do
	{
		ConvexHullHalfEdge* edge = loopFace->Edge;

		for (size_t e = 0; e < 3; ++e)
		{
			if (edge->Twin)
			{
				continue;
			}

			edge->Twin = FindTwinEdge(convexHull, edge);
			assert(edge->Twin);
			edge->Twin->Twin = edge;
			edge = edge->Next;
		}

		loopFace = loopFace->Next;
	} while (loopFace != convexHull.FacesListHead);
	
	//Put all remaining points into the conflict lists.
	for (auto& point : pointCloud)
	{
		ConvexHullFace* furthestFace = nullptr;
		float furthestDistance = 0;

		ConvexHullFace* testFace = convexHull.FacesListHead;
		do
		{
			Plane facePlane = GetNormalisedSurfacePlaneFromHullFace(*testFace);
			float distanceToPlane = DotPoint(facePlane, point);

			if (distanceToPlane - TINY_FLOAT > furthestDistance)
			{
				furthestDistance = distanceToPlane;
				furthestFace = testFace;
			}

			testFace = testFace->Next;
		} while (testFace != convexHull.FacesListHead);

		if (furthestFace == nullptr)
		{
			//Means distance always less than 0, point is inside hull.
			continue;
		}

		ConvexHullVertex* vertex = convexHull.GetNewVertex();
		vertex->Vertex = point;
		furthestFace->ConflictList.push_back(vertex);
		furthestFace = nullptr;
		furthestDistance = 0;
		vertex = nullptr;
	}

}

//Create a new face and setup initial edges.
ConvexHullFace* Quickhull::CreateHullFace(ConvexHull& convexHull, ConvexHullVertex*& vertexA, ConvexHullVertex*& vertexB, ConvexHullVertex*& vertexC)
{
	ConvexHullFace* face = convexHull.GetNewFace();

	ConvexHullHalfEdge* edges[3] = 
	{
		convexHull.GetNewEdge(),
		convexHull.GetNewEdge(),
		convexHull.GetNewEdge(),
	};

	convexHull.EdgeCount += 2;
	face->VertexCount = 3;

	face->Edge = edges[0];

	edges[0]->Tail = vertexA;
	edges[0]->Face = face;
	edges[0]->Prev = edges[2];
	edges[0]->Next = edges[1];

	edges[1]->Tail = vertexB;
	edges[1]->Face = face;
	edges[1]->Prev = edges[0];
	edges[1]->Next = edges[2];

	edges[2]->Tail = vertexC;
	edges[2]->Face = face;
	edges[2]->Prev = edges[1];
	edges[2]->Next = edges[0];

	return face;
}

// Clock it twin!!
// Finds the back facing twin edge for a specific edge.
ConvexHullHalfEdge* Quickhull::FindTwinEdge(const ConvexHull& convexHull, const ConvexHullHalfEdge* edge)
{
	ConvexHullFace* face = convexHull.FacesListHead;

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
	} while (face != convexHull.FacesListHead);

	throw new std::exception("Failed to find a twin edge.");
}

ConvexHullVertex* ConvexHull::GetNewVertex()
{
	ConvexHullVertex* newVertex = nullptr;
	if (m_FreeVertices.size() > 0)
	{
		newVertex = m_FreeVertices.top();
		m_FreeVertices.pop();
	}
	else
	{
		newVertex = &m_VertexBuffer[m_AllocatedVertexCount++];
	}

	return newVertex;
}

ConvexHullHalfEdge* ConvexHull::GetNewEdge()
{
	ConvexHullHalfEdge* newEdge = nullptr;
	
	if (m_FreeEdges.size() > 0)
	{
		newEdge = m_FreeEdges.top();
		m_FreeEdges.pop();
	}
	else
	{
		newEdge = &m_EdgeBuffer[m_AllocatedEdgeCount++];
	}

	return newEdge;
}

ConvexHullFace* ConvexHull::GetNewFace()
{
	ConvexHullFace* newFace = nullptr;

	if (m_FreeFaces.size() > 0)
	{
		newFace = m_FreeFaces.top();
		m_FreeFaces.pop();
	}
	else
	{
		newFace = &m_FaceBuffer[m_AllocatedFaceCount++];
	}

	return newFace;
}

void ConvexHull::DestroyVertex(ConvexHullVertex*& vertex)
{
	if (vertex->Prev)
	{
		RemoveVertexFromHull(vertex);
	}

	m_FreeVertices.push(vertex);
}

void ConvexHull::DestroyHalfEdge(ConvexHullHalfEdge*& edge)
{
	edge->Dead = true;
	m_FreeEdges.push(edge);
}

void ConvexHull::DestroyFace(ConvexHullFace*& face)
{
	RemoveFaceFromHull(face);
	face->Dead = true;
	m_FreeFaces.push(face);
}


void ConvexHull::AddVertexToHull(ConvexHullVertex*& vertex)
{
	if (VerticesListHead != nullptr)
	{
		ConvexHullVertex* back = VerticesListHead->Prev;
		back->Next = vertex;
		vertex->Prev = back;
	}
	else
	{
		VerticesListHead = vertex;
	}

	vertex->Next = VerticesListHead;
	VerticesListHead->Prev = vertex;

	VertexCount++;
}

void ConvexHull::AddFaceToHull(ConvexHullFace*& face)
{
	if (FacesListHead != nullptr)
	{
		ConvexHullFace* back = FacesListHead->Prev;
		back->Next = face;
		face->Prev = back;
	}
	else
	{
		FacesListHead = face;
	}

	face->Next = FacesListHead;
	FacesListHead->Prev = face;

	FaceCount++;
}

void ConvexHull::AddEdgeToHull(ConvexHullHalfEdge*& edge)
{
	if (EdgesListHead != nullptr)
	{
		ConvexHullHalfEdge* back = EdgesListHead->Prev;
		back->Next = edge;
		edge->Prev = back;
	}
	else
	{
		EdgesListHead = edge;
	}

	edge->Next = EdgesListHead;
	EdgesListHead->Prev = edge;

	EdgeCount++;
}

void ConvexHull::RemoveVertexFromHull(ConvexHullVertex*& vertex)
{
	if (VerticesListHead == vertex)
	{
		VerticesListHead = VerticesListHead->Next;
	}

	vertex->Prev->Next = vertex->Next;
	vertex->Next->Prev = vertex->Prev;

	--VertexCount;
}

void ConvexHull::RemoveFaceFromHull(ConvexHullFace*& face)
{
	if (FacesListHead == face)
	{
		FacesListHead = FacesListHead->Next;
	}

	face->Prev->Next = face->Next;
	face->Next->Prev = face->Prev;

	--FaceCount;
}

void ConvexHull::GetEdgesAsLineList(std::vector<Vector3>& lineList)
{
	lineList.clear();
	
	//Lines
	ConvexHullFace* face = FacesListHead;
	do
	{
		ConvexHullHalfEdge* edge = face->Edge;
		do
		{
			Vector3 lineStart = edge->Tail->Vertex;
			Vector3 lineEnd = edge->Next->Tail->Vertex;
			lineList.push_back(lineStart);
			lineList.push_back(lineEnd);
			edge = edge->Next;

		} while (edge != face->Edge);

		face = face->Next;

	} while (face != FacesListHead);
}

void ConvexHull::RemoveEdgeFromHull(ConvexHullHalfEdge*& edge)
{
	if (EdgesListHead == edge)
	{
		EdgesListHead = EdgesListHead->Next;
	}

	edge->Prev->Next = edge->Next;
	edge->Next->Prev = edge->Prev;

	--EdgeCount;
}