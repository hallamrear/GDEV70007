#pragma once
#include <Physics/Quickhull/QH_Vertex.h>

class QH_Face
{
private:
	enum QH_FACE_VISIBILITY
	{
		VISIBLE = 1,
		NON_CONVEX = 2,
		DELETED = 3
	};

	void ComputeCentroid();
	void ComputeNormal();
	void ComputeNormalAndCentroid();

public:
	float PlaneOffset;
	QH_FACE_VISIBILITY Visibility;
	QH_Face* Prev;
	QH_Face* Next;
	QH_HalfEdge* Edge;
	std::vector<QH_Vertex> ConflictList = std::vector<QH_Vertex>();
	Vector3 Normal;
	Vector3 Centre;
	int VertexCount;
	int Index;
	float Area;
	QH_Vertex* Outside;

};