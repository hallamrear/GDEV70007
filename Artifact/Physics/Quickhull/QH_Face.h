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
	void ComputeNormalAndCentroid(float minArea = 0.0f);

	QH_Face* CreateTriangle(QH_Vertex* v0, QH_Vertex* v1, QH_Vertex* v2, float minArea = 0.0f);

	QH_HalfEdge* GetHalfEdge(const int& index);
	QH_HalfEdge* FindEdge(QH_Vertex* startVertex, QH_Vertex* endVertex);
	QH_HalfEdge* GetStartEdge();

public:
	QH_Face();
	~QH_Face();

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

	const Vector3& GetNormal() const;
	const Vector3& GetCentre() const;
	const int& GetVertexCount() const;
};