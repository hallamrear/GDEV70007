#pragma once

//todo : move things here.
// 
//#include <Rendering/VertexBuffer.h>
//
//struct ConvexHullFace;
//
//struct ConvexHullVertex
//{
//	friend class ConvexHull;
//	int VertexID = -1;
//	ConvexHullVertex* Next = nullptr;
//	ConvexHullVertex* Prev = nullptr;
//	Vector3 Vertex = Vector3(0.0f, 0.0f, 0.0f);
//	bool Dead = false;
//
//protected:
//	ConvexHullVertex()
//	{
//		static int counter = 0;
//		VertexID = counter;
//		counter++;
//	}
//};
//
//typedef std::vector<Vector3> PointCloud;
//typedef std::vector<Vector3> Simplex;
//typedef std::vector<ConvexHullVertex*> ConflictList;
//
//struct ConvexHullHalfEdge
//{
//	friend class ConvexHull;
//	int EdgeID = -1;
//	ConvexHullHalfEdge* Prev = nullptr;
//	ConvexHullHalfEdge* Next = nullptr;
//	ConvexHullHalfEdge* Twin = nullptr;
//	ConvexHullVertex* Tail = nullptr;
//	ConvexHullFace* Face = nullptr;
//
//	bool Dead = false;
//
//protected:
//	ConvexHullHalfEdge()
//	{
//		static int counter = 0;
//		EdgeID = counter;
//		counter++;
//	}
//};
//
//struct ConvexHullFace
//{
//	friend class ConvexHull;
//	int FaceID = -1;
//	ConvexHullFace* Prev = nullptr;
//	ConvexHullFace* Next = nullptr;
//	ConvexHullHalfEdge* Edge = nullptr;
//	ConflictList ConflictList;
//	bool Dead = false;
//
//protected:
//	ConvexHullFace()
//	{
//		static int counter = 0;
//		FaceID = counter;
//		counter++;
//	}
//};
//
////Half-edge based convex hull.
//class ConvexHull
//{
//private:
//	ConvexHullVertex* m_VertexBuffer;
//	ConvexHullFace* m_FaceBuffer;
//	ConvexHullHalfEdge* m_EdgeBuffer;
//	int m_VertexBufferElementCount;
//	int m_FaceBufferElementCount;
//	int m_EdgeBufferElementCount;
//
//	std::stack<ConvexHullVertex*> m_FreeVertices;
//	std::stack<ConvexHullFace*> m_FreeFaces;
//	std::stack<ConvexHullHalfEdge*> m_FreeEdges;
//	int m_AllocatedVertexCount;
//	int m_AllocatedFaceCount;
//	int m_AllocatedEdgeCount;
//
//public:
//	//todo : encapsulate
//	VertexBuffer m_RenderingVertexBuffer;
//
//	ConvexHullVertex* VerticesListHead;
//	ConvexHullFace* FacesListHead;
//	ConvexHullHalfEdge* EdgesListHead;
//
//	int EdgeCount;
//	int VertexCount;
//	int FaceCount;
//
//	const VertexBuffer& GetDrawVertexBuffer() const { return m_RenderingVertexBuffer; };
//
//	ConvexHullVertex* GetNewVertex();
//	ConvexHullHalfEdge* GetNewEdge();
//	ConvexHullFace* GetNewFace();
//	void DestroyVertex(ConvexHullVertex* vertex);
//	void DestroyHalfEdge(ConvexHullHalfEdge* edge);
//	void DestroyFace(ConvexHullFace* face);
//
//	void AddVertexToHull(ConvexHullVertex* vertex);
//	void AddEdgeToHull(ConvexHullHalfEdge* edge);
//	void AddFaceToHull(ConvexHullFace* face);
//	void RemoveVertexFromHull(ConvexHullVertex* vertex);
//	void RemoveEdgeFromHull(ConvexHullHalfEdge* edge);
//	void RemoveFaceFromHull(ConvexHullFace* face);
//
//	ConvexHull(const int& expectedSize)
//	{
//		m_VertexBufferElementCount = expectedSize * 2;
//		m_VertexBuffer = new ConvexHullVertex[m_VertexBufferElementCount];
//
//		m_EdgeBufferElementCount = (3 * expectedSize - 6) * 2 * 2;
//		m_EdgeBuffer = new ConvexHullHalfEdge[m_EdgeBufferElementCount];
//
//		m_FaceBufferElementCount = (2 * expectedSize - 4) * 2;
//		m_FaceBuffer = new ConvexHullFace[m_FaceBufferElementCount];
//
//		m_AllocatedVertexCount = 0;
//		m_AllocatedEdgeCount = 0;
//		m_AllocatedFaceCount = 0;
//		VerticesListHead = nullptr;
//		FacesListHead = nullptr;
//		EdgesListHead = nullptr;
//		EdgeCount = 0;
//		VertexCount = 0;
//		FaceCount = 0;
//	};
//
//	~ConvexHull()
//	{
//		if (m_VertexBuffer)
//		{
//			delete[] m_VertexBuffer;
//			m_VertexBuffer = nullptr;
//		}
//
//		if (m_EdgeBuffer)
//		{
//			delete[] m_EdgeBuffer;
//			m_EdgeBuffer = nullptr;
//		}
//
//		if (m_FaceBuffer)
//		{
//			delete[] m_FaceBuffer;
//			m_FaceBuffer = nullptr;
//		}
//
//		m_AllocatedVertexCount = 0;
//		m_AllocatedEdgeCount = 0;
//		m_AllocatedFaceCount = 0;
//	};
//
//	void GetFacesAsList(std::vector<Vector3>& lineList);
//};
