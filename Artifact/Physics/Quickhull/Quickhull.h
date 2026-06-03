#pragma once
#include <list>
#include <unordered_set>
#include <type_traits>
#include <stack>
#include <System/Maths.h>
#include <System/Maths/Plane.h>
#include <System/Maths/Triangle.h>

//todo : find a way to remove
#include <Rendering/VertexBuffer.h>

struct ConvexHullFace;

struct ConvexHullVertex
{
	friend class ConvexHull;
	int VertexID = -1;
	ConvexHullVertex* Next = nullptr;
	ConvexHullVertex* Prev = nullptr;
	Vector3 Vertex = Vector3(0.0f, 0.0f, 0.0f);
	bool Dead = false;

protected:
	ConvexHullVertex()
	{
		static int counter = 0;
		VertexID = counter;
		counter++;
	}
};

typedef std::vector<Vector3> PointCloud;
typedef std::vector<ConvexHullVertex*> ConflictList;

struct Simplex
{
private:
	int m_Size;

public:
	Vector3 Points[4];
	
	void PushFront(const Vector3& point)
	{
		Points[3] = Points[2];
		Points[2] = Points[1];
		Points[1] = Points[0];
		Points[0] = point;
		m_Size = std::min(m_Size + 1, 4);
	}

	void PushBack(const Vector3& point)
	{
		Points[m_Size] = point;
		m_Size = std::min(m_Size + 1, 4);
	}
};

struct ConvexHullHalfEdge
{
	friend class ConvexHull;
	int EdgeID = -1;
	ConvexHullHalfEdge* Prev = nullptr;
	ConvexHullHalfEdge* Next = nullptr;
	ConvexHullHalfEdge* Twin = nullptr;
	ConvexHullVertex* Tail = nullptr;
	ConvexHullFace* Face = nullptr;

	bool Dead = false;

protected:
	ConvexHullHalfEdge()
	{
		static int counter = 0;
		EdgeID = counter;
		counter++;
	}
};

struct ConvexHullFace
{
	friend class ConvexHull;
	int FaceID = -1;
	ConvexHullFace* Prev = nullptr;
	ConvexHullFace* Next = nullptr;
	ConvexHullHalfEdge* Edge = nullptr;
	ConflictList ConflictList;
	bool Dead = false;
	int VertexCount = 0;
	Plane Plane;

protected:
	ConvexHullFace()
	{
		static int counter = 0;
		FaceID = counter;
		counter++;
		Plane = { 0.0f, 0.0f, 0.0f, 0.0f };
	}
};

//Half-edge based convex hull.
class ConvexHull
{
private:
	friend class Quickhull;
	ConvexHullVertex* m_VertexBuffer;
	ConvexHullFace* m_FaceBuffer;
	ConvexHullHalfEdge* m_EdgeBuffer;
	int m_VertexBufferElementCount;
	int m_FaceBufferElementCount;
	int m_EdgeBufferElementCount;

	std::stack<ConvexHullVertex*> m_FreeVertices;
	std::stack<ConvexHullFace*> m_FreeFaces;
	std::stack<ConvexHullHalfEdge*> m_FreeEdges;
	int m_AllocatedVertexCount;
	int m_AllocatedFaceCount;
	int m_AllocatedEdgeCount;

	std::vector<Plane> m_FinalFacePlanes;
	std::vector<ConvexHullHalfEdge*> m_FinalEdgeList;
	void AccumulateFinalEdgesFromFaces();

public:
	//todo : encapsulate
	VertexBuffer m_RenderingVertexBuffer;

	ConvexHullVertex* VerticesListHead;
	ConvexHullFace* FacesListHead;
	ConvexHullHalfEdge* EdgesListHead;

	int EdgeCount;
	int VertexCount;
	int FaceCount;

	const VertexBuffer& GetDrawVertexBuffer() const { return m_RenderingVertexBuffer; };

	const std::vector<ConvexHullHalfEdge*>& GetEdgeList() const { return m_FinalEdgeList; };

	Vector3 FindSupportVertex(const Vector3& direction, int& vertexIndex) const;

	ConvexHullVertex* GetNewVertex();
	ConvexHullHalfEdge* GetNewEdge();
	ConvexHullFace* GetNewFace();
	void DestroyVertex(ConvexHullVertex*& vertex);
	void DestroyHalfEdge(ConvexHullHalfEdge*& edge);
	void DestroyFace(ConvexHullFace*& face);

	void AddVertexToHull(ConvexHullVertex*& vertex);
	void AddEdgeToHull(ConvexHullHalfEdge*& edge);
	void AddFaceToHull(ConvexHullFace*& face);
	void RemoveVertexFromHull(ConvexHullVertex*& vertex);
	void RemoveEdgeFromHull(ConvexHullHalfEdge*& edge);
	void RemoveFaceFromHull(ConvexHullFace*& face);

	ConvexHull(const int& expectedSize) 
	{
		m_VertexBufferElementCount = expectedSize * 2;
		m_VertexBuffer = new ConvexHullVertex[m_VertexBufferElementCount];

		m_EdgeBufferElementCount = (3 * expectedSize - 6) * 2 * 2;
		m_EdgeBuffer = new ConvexHullHalfEdge[m_EdgeBufferElementCount];

		m_FaceBufferElementCount = (2 * expectedSize - 4) * 2;
		m_FaceBuffer = new ConvexHullFace[m_FaceBufferElementCount];

		m_AllocatedVertexCount = 0;
		m_AllocatedFaceCount = 0;
		m_AllocatedEdgeCount = 0;
		VerticesListHead = nullptr;
		FacesListHead = nullptr;
		EdgesListHead = nullptr;
		EdgeCount = 0;
		VertexCount = 0;
		FaceCount = 0;
	};

	~ConvexHull()
	{
		if (m_VertexBuffer)
		{
			delete[] m_VertexBuffer;
			m_VertexBuffer = nullptr;
		}

		if (m_EdgeBuffer)
		{
			delete[] m_EdgeBuffer;
			m_EdgeBuffer = nullptr;
		}

		if (m_FaceBuffer)
		{
			delete[] m_FaceBuffer;
			m_FaceBuffer = nullptr;
		}

		m_AllocatedVertexCount = 0;
		m_AllocatedEdgeCount = 0;
		m_AllocatedFaceCount = 0;
	};

	void GetFacesAsList(std::vector<Vector3>& pointList);
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
	static bool AreFacesConvex(const ConvexHullFace& faceA, const ConvexHullFace& otherFace, const float& epsilon);
	static int GetFaceVertexCount(const ConvexHullFace& face);

	//Geometric helpers
private:
	static void MergeConcaveFaces(ConvexHull& convexHull, ConvexHullFace& conflictFace, ConvexHullHalfEdge*& edge);
	static void GenerateHullFacePlanes(ConvexHull& convexHull);

private:

	static void AddAndResolveNewVertexInHull(ConvexHull& convexHull, ConvexHullFace*& conflictFace, ConvexHullVertex*& conflictVertex, const float& scaledEpsilon);
	static Plane CreateNewellPlaneFromTriangle(int planarVertexCount, const ConvexHullFace& face);
	static ConvexHullVertex* GetNextConflictVertex(ConvexHull*& convexHull, ConvexHullFace*& conflictVertexFace);
	static std::vector<ConvexHullFace*>  DetermineHorizon(std::list<ConvexHullHalfEdge*>& horizon, ConvexHullFace*& conflictFace, ConvexHullVertex* conflictVertex, const float& scaledEpsilon);
	static void DetermineHorizonRecursiveSearch(std::unordered_set<ConvexHullFace*>& visitedFaces, std::list<ConvexHullHalfEdge*>& horizon, std::vector<ConvexHullFace*>& visibleFaceList, ConvexHullFace* conflictFace, ConvexHullVertex* conflictVertex, const float& scaledEpsilon);
	static void BuildNewFaces(std::list<ConvexHullHalfEdge*>& horizon, std::vector<ConvexHullFace*>& newFaces, ConvexHull& convexHull, ConvexHullVertex*& conflictVertex);
	static void MergeFaces(std::vector<ConvexHullFace*>& newFaces, ConvexHull& convexHull, ConvexHullFace*& conflictFace, const float& scaledEpsilon);
	static void FixAdditionalPointInvariance(ConvexHull& convexHull, ConvexHullFace*& conflictFace, ConvexHullHalfEdge*& incoming, ConvexHullHalfEdge*& outgoing);
	static void FixInternalPointInvariance(ConvexHull& convexHull, ConvexHullFace*& conflictFace, ConvexHullHalfEdge*& edgeA, ConvexHullHalfEdge*& edgeB);
	static void UpdateExistingFaces(std::vector<ConvexHullFace*>& newFaces, std::vector<ConvexHullFace*>& visibleFaces, ConvexHull& convexHull, ConvexHullFace*& conflictFace);
	static void ResolveOrphanPoints(ConvexHull& convexHull, std::vector<ConvexHullFace*>& newFaces, ConvexHullFace*& conflictFace);
	static const Simplex BuildInitialSimplex(const PointCloud& pointCloud, Vector4& searchDirection);
	static void ConstructInitialHullFromSimplex(ConvexHull& convexHull, PointCloud& pointCloud, const Simplex& simplex, const Vector4& constructionDirection);
	static ConvexHullFace* CreateHullFace(ConvexHull& convexHull, ConvexHullVertex*& vertexA, ConvexHullVertex*& vertexB, ConvexHullVertex*& vertexC);
	static ConvexHullHalfEdge* FindTwinEdge(const ConvexHull& convexHull, const ConvexHullHalfEdge* edge);
	static ConvexHullHalfEdge* FindTwinEdgeOfEyeVertex(const std::vector<ConvexHullHalfEdge*>& edgeList, ConvexHullHalfEdge*& edge, ConvexHullVertex*& eyeVertex);
};