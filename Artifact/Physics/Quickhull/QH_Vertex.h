#pragma once

class QH_Face;
class QH_HalfEdge;

class QH_Vertex
{
public:
	int Index;
	QH_Vertex* Prev;
	QH_Vertex* Next;
	QH_HalfEdge* Edge;
	Vector3 Position;
	QH_Face* Face;
};
