#pragma once
#include <list>
#include <unordered_set>
#include <type_traits>

struct ConvexHullFace;
struct ConvexHullVertex;
typedef std::vector<Vector3> PointCloud;
typedef std::vector<Vector3> Simplex;
typedef std::vector<ConvexHullVertex*> ConflictList;

struct ConvexHullVertex
{
	int VertexID = -1;
	ConvexHullVertex* Next = nullptr;
	ConvexHullVertex* Prev = nullptr;
	Vector3 Vertex = Vector3(0.0f, 0.0f, 0.0f);
	bool Dead = false;

	ConvexHullVertex()
	{
		static int counter = 0;
		VertexID = counter;
		counter++;
	}
};

struct ConvexHullHalfEdge
{
	int EdgeID = -1;
	ConvexHullHalfEdge* Prev = nullptr;
	ConvexHullHalfEdge* Next = nullptr;
	ConvexHullHalfEdge* Twin = nullptr;
	ConvexHullVertex* Tail = nullptr;
	ConvexHullFace* Face = nullptr;

	bool Dead = false;

	ConvexHullHalfEdge()
	{
		static int counter = 0;
		EdgeID = counter;
		counter++;
	}
};

struct ConvexHullFace
{
	int FaceID = -1;
	ConvexHullFace* Prev = nullptr;
	ConvexHullFace* Next = nullptr;
	ConvexHullHalfEdge* Edge = nullptr;
	ConflictList ConflictList;
	bool Dead = false;

	ConvexHullFace()
	{
		static int counter = 0;
		FaceID = counter;
		counter++;
	}
};

//Half-edge based convex hull.
class ConvexHull
{
private:
	//Todo : I can probably preallocate these.

public:
	std::vector<ConvexHullVertex*> Vertices;
	std::vector<ConvexHullFace*> Faces;
	void AddVertex(ConvexHullVertex* vertex);
	void AddFace(ConvexHullFace* face);

	void RemoveVertex(ConvexHullVertex* vertex);
	void RemoveFace(ConvexHullFace* face);

	ConvexHull() {};
	~ConvexHull() {};
};


class Quickhull
{
public:
	static ConvexHull* GenerateConvexHull(PointCloud& pointCloud);

	//Specific Maths helpers
private:
	static Plane GetNormalisedSurfacePlaneFromHullFace(const ConvexHullFace& face);
	static const float Calculate3DEpsilonFromExtents(const PointCloud& pointCloud);
	static bool IsFaceVisible(const ConvexHullFace& face, const ConvexHullVertex& eyeVertex, const float& scaledEpsilon);
	static bool AreFacesConvex(const ConvexHullFace& faceA, const ConvexHullFace& faceB, const float& epsilon);
	static int GetFaceVertexCount(const ConvexHullFace& face);

	//Geometric helpers
private:
	static void MergeConcaveFaces(ConvexHull& convexHull, ConvexHullFace& conflictFace, ConvexHullHalfEdge*& edge);

private:

	static void AddAndResolveNewVertexInHull(ConvexHull& convexHull, ConvexHullFace*& conflictFace, ConvexHullVertex*& conflictVertex, const float& scaledEpsilon);
	static ConvexHullVertex* GetNextConflictVertex(ConvexHull*& convexHull, ConvexHullFace*& conflictVertexFace);
	static void DetermineHorizonRecursiveSearch(std::unordered_set<ConvexHullFace*>& visitedFaces, std::list<ConvexHullHalfEdge*>& horizon, std::vector<ConvexHullFace*>& visibleFaceList, ConvexHullFace* conflictFace, ConvexHullVertex* conflictVertex, const float& scaledEpsilon);
	static void BuildNewFaces(std::list<ConvexHullHalfEdge*>& horizon, std::vector<ConvexHullFace*>& newFaces, ConvexHull& convexHull, ConvexHullVertex*& conflictVertex);
	static void MergeFaces(std::vector<ConvexHullFace*>& newFaces, ConvexHull& convexHull, ConvexHullFace*& conflictFace, const float& scaledEpsilon);
	static void FixAdditionalPointInvariance(ConvexHull& convexHull, ConvexHullFace*& conflictFace, ConvexHullHalfEdge*& incoming, ConvexHullHalfEdge*& outgoing);
	static void FixInternalPointInvariance(ConvexHull& convexHull, ConvexHullFace*& conflictFace, ConvexHullHalfEdge*& edgeA, ConvexHullHalfEdge*& edgeB);
	static void UpdateExistingFaces(std::vector<ConvexHullFace*>& newFaces, std::vector<ConvexHullFace*>& visibleFaces, ConvexHull& convexHull, ConvexHullFace*& conflictFace);
	static const Simplex BuildInitialSimplex(const PointCloud& pointCloud, Vector4& searchDirection);
	static void ConstructInitialHullFromSimplex(ConvexHull& convexHull, PointCloud& pointCloud, const Simplex& simplex, const Vector4& constructionDirection);
	static ConvexHullFace* CreateHullFace(ConvexHull& convexHull, ConvexHullVertex*& vertexA, ConvexHullVertex*& vertexB, ConvexHullVertex*& vertexC);
	static ConvexHullHalfEdge* FindTwinEdge(const ConvexHull& convexHull, const ConvexHullHalfEdge* edge);
	static ConvexHullHalfEdge* FindTwinEdgeOfEyeVertex(const std::vector<ConvexHullHalfEdge*>& edgeList, const ConvexHullHalfEdge*& edge, const ConvexHullVertex*& eyeVertex);
};