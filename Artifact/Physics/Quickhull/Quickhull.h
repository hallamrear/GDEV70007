#pragma once
#include <list>
#include <unordered_set>

struct ConvexHullFace;
struct ConvexHullVertex;
typedef std::vector<Vector3> PointCloud;
typedef std::vector<Vector3> Simplex;
typedef std::vector<ConvexHullVertex*> ConflictList;

struct ConvexHullVertex
{
	ConvexHullVertex* Next = nullptr;
	ConvexHullVertex* Prev = nullptr;
	Vector3 Vertex = Vector3(0.0f, 0.0f, 0.0f);
};

struct ConvexHullHalfEdge
{
	ConvexHullHalfEdge* Prev = nullptr;
	ConvexHullHalfEdge* Next = nullptr;
	ConvexHullHalfEdge* Twin = nullptr;
	ConvexHullVertex* Tail = nullptr;
	ConvexHullFace* Face = nullptr;
};

struct ConvexHullFace
{
	ConvexHullFace* Prev = nullptr;
	ConvexHullFace* Next = nullptr;
	ConvexHullHalfEdge* Edge = nullptr;
	ConflictList ConflictList;
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

private:


	static void AddAndResolveNewVertexInHull(ConvexHull& convexHull, ConvexHullFace* conflictFace, ConvexHullVertex* conflictVertex, const float& scaledEpsilon);
	static ConvexHullVertex* GetNextConflictVertex(ConvexHull* convexHull, ConvexHullFace*& conflictVertexFace);
	static void DetermineHorizonRecursiveSearch(std::unordered_set<ConvexHullFace*>& visitedFaces, std::list<ConvexHullHalfEdge*>& horizon, std::vector<ConvexHullFace*>& visibleFaceList, ConvexHullFace* conflictFace, ConvexHullVertex* conflictVertex, const float& scaledEpsilon);
	static void BuildNewFaces(std::list<ConvexHullHalfEdge*>& horizon, std::vector<ConvexHullFace*> newFaces, ConvexHull& convexHull, ConvexHullVertex* conflictVertex);
	static void MergeFaces(std::vector<ConvexHullFace*> newFaces, ConvexHull& convexHull, ConvexHullFace* conflictFace, const float& scaledEpsilon);
	static void UpdateExistingFaces(std::vector<ConvexHullFace*> newFaces, std::vector<ConvexHullFace*>& visibleFaces, ConvexHull& convexHull, ConvexHullFace* conflictFace);
	static const Simplex BuildInitialSimplex(const PointCloud& pointCloud, Vector4& searchDirection);
	static void ConstructInitialHullFromSimplex(ConvexHull& convexHull, PointCloud& pointCloud, const Simplex& simplex, const Vector4& constructionDirection);
	static ConvexHullFace* CreateHullFace(ConvexHull& convexHull, ConvexHullVertex* vertexA, ConvexHullVertex* vertexB, ConvexHullVertex* vertexC);
	static ConvexHullHalfEdge* FindTwinEdge(const ConvexHull& convexHull, const ConvexHullHalfEdge* edge);
	static ConvexHullHalfEdge* FindTwinEdgeOfEyeVertex(const std::vector<ConvexHullHalfEdge*>& edgeList, const ConvexHullHalfEdge* edge, const ConvexHullVertex* eyeVertex);
};