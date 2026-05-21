#pragma once

class QH_Face;
class QH_Vertex;

class QH_HalfEdge
{
public:
	//Point
	QH_Vertex* HeadVertex;

	//Last half edge in tri
	QH_HalfEdge* Prev;
	//Next half edge in tri
	QH_HalfEdge* Next;

	//Twin edge covering adjacent face.
	QH_HalfEdge* OppositeTwin;

	//Associated Face.
	QH_Face* Face;

	QH_HalfEdge(QH_Vertex* vertex, QH_Face* face);
};
