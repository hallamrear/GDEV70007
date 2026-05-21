#include "pch.h"
#include "Quickhull.h"

//TODO : Proper Cleanup;
void QH_VertexList::Clear()
{
	Start = nullptr;
	End = nullptr;
}

void QH_VertexList::Add(QH_Vertex* vertex)
{
	assert(vertex);

	if (Start == nullptr)
	{
		Start = vertex;
	}
	else
	{

	}
}

QH_Vertex* QH_VertexList::GetStart() const
{
	return Start;
}

bool QH_VertexList::Empty() const
{
	return (Start == nullptr);
}
//
//bool Quickhull::ConstructQuickhull(const std::vector<QH_Vertex>& vertices)
//{
//	if (BuildInitialHull(vertices) == false)
//	{
//		return false;
//	}
//
//	QH_Vertex* vertex = GetNextConflictVertex();
//
//	while (vertex != nullptr)
//	{
//		AddVertexToHull(vertex);
//		vertex = GetNextConflictVertex();
//	}
//
//	return true;
//}
//
//QH_HalfEdge* Quickhull::FindHalfEdge(QH_Vertex* start, QH_Vertex* end)
//{
//	throw new std::exception("Not Implemented.");
//	return nullptr;
//}
//
//void Quickhull::AddVertexToHull(QH_Vertex* vertex)
//{
//	std::vector<QH_HalfEdge*> Horizon = std::vector<QH_HalfEdge*>();
//	BuildHorizon(Horizon);
//
//	std::vector<QH_Face*> NewFaces = std::vector<QH_Face*>();
//	BuildNewFaces(NewFaces, Horizon);
//	MergeSimilarFaces(NewFaces);
//
//	ResolveOrphanVertices(NewFaces);
//}
//
//bool Quickhull::BuildInitialHull(const std::vector<QH_Vertex>& vertices)
//{
//	return false;
//}
//
//QH_Vertex* Quickhull::GetNextConflictVertex()
//{
//	return nullptr;
//}