#pragma once
#include <Physics/Quickhull/QH_HalfEdge.h>
#include <Physics/Quickhull/QH_Vertex.h>

struct QH_VertexList
{
private:
	QH_Vertex* Start;
	QH_Vertex* End;

public:
	void Clear();
	void Add(QH_Vertex* vertex);

	QH_Vertex* GetStart() const;
	bool Empty() const;
};

class Quickhull
{
public:
	static bool ConstructQuickhull(const std::vector<QH_Vertex>& vertices);

private:
	QH_HalfEdge* FindHalfEdge(QH_Vertex* start, QH_Vertex* end);
};