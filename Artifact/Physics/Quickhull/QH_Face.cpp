#include "pch.h"
#include "QH_Face.h"
#include <Physics/Quickhull/QH_HalfEdge.h>

QH_Face::QH_Face()
{
	Normal = Vector3(0.0f, 0.0f, 0.0f);
	Centre = Vector3(0.0f, 0.0f, 0.0f);
	Visibility = QH_FACE_VISIBILITY::VISIBLE;
	PlaneOffset = 0.0f;
}

QH_Face::~QH_Face()
{

}

void QH_Face::ComputeCentroid()
{
	Centre = Vector3(0.0f, 0.0f, 0.0f);

	QH_HalfEdge* halfEdge = Edge;

	do
	{
		assert(halfEdge->HeadVertex);
		Centre.x += halfEdge->HeadVertex->Position.x;
		Centre.y += halfEdge->HeadVertex->Position.y;
		Centre.z += halfEdge->HeadVertex->Position.z;
		halfEdge = halfEdge->Next;
	} 
	while (halfEdge != Edge);

	float scalar = (1.0f / VertexCount);
	Centre.x *= scalar;
	Centre.y *= scalar;
	Centre.z *= scalar;
}

void QH_Face::ComputeNormal()
{
	QH_HalfEdge* edge1 = Edge->Next;
	QH_HalfEdge* edge2 = edge1->Next;

	Normal = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 p0 = Edge->HeadVertex->Position;
	Vector3 p2 = edge1->HeadVertex->Position;

	Vector3 edgeDiffA = Vector3(p2.x - p0.x, p2.y - p0.y, p2.z - p0.z);
	int vertexCount = 2; // { p0, p2 }

	while (edge2 != Edge)
	{
		Vector3 edgeDiffB = edgeDiffA;
		p2 = edge2->HeadVertex->Position;
		edgeDiffA = Vector3(p2.x - p0.x, p2.y - p0.y, p2.z - p0.z);

		Normal.x += (edgeDiffB.y * edgeDiffA.z) - (edgeDiffB.z * edgeDiffA.y);
		Normal.y += (edgeDiffB.z * edgeDiffA.x) - (edgeDiffB.x * edgeDiffA.z);
		Normal.z += (edgeDiffB.x * edgeDiffA.y) - (edgeDiffB.y * edgeDiffA.x);

		edge1 = edge2;
		edge2 = edge2->Next;
		vertexCount++;
	}

	Vector3 temp;
	DirectX::XMStoreFloat3(&temp, DirectX::XMVector3Length(DirectX::XMLoadFloat3(&Normal)));
	Area = temp.x;
	DirectX::XMStoreFloat3(&Normal, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&Normal)));
}

void QH_Face::ComputeNormalAndCentroid(float minArea)
{
	UNREFERENCED_PARAMETER(minArea);

	ComputeNormal();
	ComputeCentroid();
	Vector3 temp;
	DirectX::XMStoreFloat3(&temp, DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&Normal), DirectX::XMLoadFloat3(&Centre)));
	PlaneOffset = temp.x;
}

QH_Face* QH_Face::CreateTriangle(QH_Vertex* v0, QH_Vertex* v1, QH_Vertex* v2, float minArea)
{
	QH_Face* face = new QH_Face();

	QH_HalfEdge* halfedge0 = new QH_HalfEdge(v0, face);
	QH_HalfEdge* halfedge1 = new QH_HalfEdge(v1, face);
	QH_HalfEdge* halfedge2 = new QH_HalfEdge(v2, face);
	halfedge0->Prev = halfedge2;
	halfedge0->Next = halfedge1;
	halfedge1->Prev = halfedge0;
	halfedge1->Next = halfedge2;
	halfedge2->Prev = halfedge1;
	halfedge2->Next = halfedge0;
	face->Edge = halfedge0;
	face->ComputeNormalAndCentroid(minArea);

	return face;
}

QH_HalfEdge* QH_Face::GetHalfEdge(const int& index)
{
	UNREFERENCED_PARAMETER(index);
	//todo : not implemented
	assert(true);
	QH_HalfEdge* edge = Edge;
	UNREFERENCED_PARAMETER(edge);
	return nullptr;
}

QH_HalfEdge* QH_Face::FindEdge(QH_Vertex* startVertex, QH_Vertex* endVertex)
{
	QH_HalfEdge* edge = Edge;

	do
	{
		if (edge->HeadVertex == startVertex && edge->Prev->HeadVertex == endVertex)
		{

		}
	}
	while (edge != Edge);

	return nullptr;
}

QH_HalfEdge* QH_Face::GetStartEdge()
{
	return Edge;
}

const Vector3& QH_Face::GetNormal() const
{
	return Normal;
}

const Vector3& QH_Face::GetCentre() const
{
	return Centre;
}

const int& QH_Face::GetVertexCount() const
{
	return VertexCount;
}