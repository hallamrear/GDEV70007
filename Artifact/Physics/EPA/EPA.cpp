#include <pch.h>
#include <Physics/EPA/EPA.h>
#include <System/Maths.h>

EPA::Edge::Edge()
{

}

EPA::Edge::Edge(const Vector3& a, const Vector3& b)
{
	start = a;
	end = b;
}

EPA::Face::Face(const Vector3& a, const Vector3& b, const Vector3& c)
{
	points[0] = a;
	points[1] = b;
	points[2] = c;

	edges[0] = Edge(a, b);
	edges[1] = Edge(a, c);
	edges[2] = Edge(b, c);

	centre.x = (points[0].x + points[1].x + points[2].x) / 3.0f;
	centre.y = (points[0].y + points[1].y + points[2].y) / 3.0f;
	centre.z = (points[0].z + points[1].z + points[2].z) / 3.0f;
	normal = Maths::GetNormalOfTriangle(a, b, c);
}

float EPA::DistanceFromPointToPlane(const Vector3& point, const Vector3& planeOrigin, const Vector3& planeNormal)
{
	Vector3 diff;
	diff.x = point.x - planeOrigin.x;
	diff.y = point.y - planeOrigin.y;
	diff.z = point.z - planeOrigin.z;

	Vector3 length;
	DirectX::XMStoreFloat3(&length, DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&diff), DirectX::XMLoadFloat3(&planeNormal)));
	return fabsf(length.x);
}

void EPA::DetermineClosestFaceToOrigin(size_t& closestFaceIndex, float& closestFaceDistance, const Polytope& polytope)
{
	closestFaceIndex = (size_t)-1;
	closestFaceDistance = FLT_MAX;

	for (size_t i = 0; i < polytope.size(); i++)
	{
		const Face& face = polytope[i];
		float distance = DistanceFromPointToPlane(Vector3(0.0f, 0.0f, 0.0f), face.centre, face.normal);

		if (distance < closestFaceDistance)
		{
			closestFaceDistance = distance;
			closestFaceIndex = i;
		}
	}

	assert(closestFaceIndex != -1);
	assert(closestFaceDistance != FLT_MAX);
}

void EPA::ExtendPolytopeWithNewPoint(Polytope& polytope, const SupportVertex& newVertex)
{
	std::vector<Edge> uniqueEdgeList = std::vector<Edge>();
	for (size_t i = 0; i < polytope.size(); i++)
	{
		for (size_t e = 0; e < 3; e++)
		{
			Edge edge = polytope[i].edges[e];
			Edge reversedEdge = Edge(edge.end, edge.start);

			std::vector<Edge>::iterator itr = std::find(uniqueEdgeList.begin(), uniqueEdgeList.end(), edge);
			std::vector<Edge>::iterator reversedItr = std::find(uniqueEdgeList.begin(), uniqueEdgeList.end(), reversedEdge);

			if (itr == uniqueEdgeList.end() && reversedItr == uniqueEdgeList.end())
			{
				uniqueEdgeList.push_back(polytope[i].edges[e]);
			}
		}
	}

	for (std::vector<Face>::iterator itr = polytope.begin(); itr != polytope.end();)
	{
		Face face = *itr;

		//Detemine if face is visible
		Vector3 triToNew = Vector3(
			newVertex.MinkowskiDifference.x - face.centre.x, 
			newVertex.MinkowskiDifference.y - face.centre.y,
			newVertex.MinkowskiDifference.z - face.centre.z);
		XMStoreFloat3(&triToNew, DirectX::XMVector3Normalize(XMLoadFloat3(&triToNew)));

		Vector3 dot;
		XMStoreFloat3(&dot, DirectX::XMVector3Dot(XMLoadFloat3(&face.normal), XMLoadFloat3(&triToNew)));

		if (dot.x > 0.0f)
		{
			itr = polytope.erase(itr);

			Edge ab = Edge(face.edges[0]);
			Edge ac = Edge(face.edges[1]);
			Edge bc = Edge(face.edges[2]);

			for (size_t i = 0; i < 3; i++)
			{
				Edge reversedEdge = Edge(face.edges[i].end, face.edges[i].start);
					
				std::vector<Edge>::iterator uniqueItr = std::find(uniqueEdgeList.begin(), uniqueEdgeList.end(), reversedEdge);

				if (uniqueItr != uniqueEdgeList.end())
				{
					uniqueEdgeList.erase(uniqueItr);
				}
				else
				{
					uniqueEdgeList.push_back(face.edges[i]);
				}
			}
		}
		else
		{
			++itr;
		}
	}

	//size_t edgeCount = uniqueEdgeList.size();
	//for (size_t i = 0; i < edgeCount; i++)
	//{
	//	Edge edge = uniqueEdgeList[i];
	//	Face newFace = Face(edge.start, edge.end, newVertex.MinkowskiDifference);
	//	polytope.push_back(newFace);
	//}
}

EPA_Result EPA::GetCollisionDetails(std::vector<SupportVertex>& simplex, const Collider& colliderA, const Collider& colliderB, CollisionManifold* manifold)
{
	EPA_Result result = EPA_Result();

	if (manifold == nullptr)
	{
		throw new std::exception("Called EPA::GetCollisionDetails with a nullptr manifold.\n");
	}

	if (simplex.size() < 4)
	{
		throw new std::exception("Called EPA::GetCollisionDetails with a simplex containing less than a 3d object.\n");
	}

	////////////////////////////////////////
	//            HOW EPA WORKS           //
	////////////////////////////////////////		
		
		 
	// 1) Start with simplex from GJK
	//Construct a new polytope (fancy word for 3d simplex) that we can add to without adjusting the original.
	Polytope polytope = Polytope();
	polytope.push_back(Face(simplex[0].MinkowskiDifference, simplex[1].MinkowskiDifference, simplex[2].MinkowskiDifference));
	polytope.push_back(Face(simplex[0].MinkowskiDifference, simplex[3].MinkowskiDifference, simplex[1].MinkowskiDifference));
	polytope.push_back(Face(simplex[0].MinkowskiDifference, simplex[2].MinkowskiDifference, simplex[3].MinkowskiDifference));
	polytope.push_back(Face(simplex[1].MinkowskiDifference, simplex[3].MinkowskiDifference, simplex[2].MinkowskiDifference));

	int iterations = 0;
	size_t closestFaceIndex = (size_t)-1;
	float closestFaceDistance = FLT_MAX;

	constexpr int temp_iteration_cap = 16;
	while (iterations < temp_iteration_cap)
	//while (true)
	{
		iterations++;

		assert(polytope.size() < 100);

		// 2) Calculate closest face (F) of polytope to origin
		DetermineClosestFaceToOrigin(closestFaceIndex, closestFaceDistance, polytope);
		
		// 3) Find support point in direction of face F normal (N)
		SupportVertex furtherPointInNormal = SupportVertex::GetSupportVertex(colliderA, colliderB, polytope[closestFaceIndex].normal);

		Vector3 dot;
		XMStoreFloat3(&dot, DirectX::XMVector3Dot(XMLoadFloat3(&polytope[closestFaceIndex].normal), XMLoadFloat3(&furtherPointInNormal.MinkowskiDifference)));
		float distanceToFurthestPointInNormal = dot.x;

		static constexpr float EPA_TOLERANCE = 0.05f;

		/* 
		* 4) if (distance to P (furthest in Normal) and distance to F are within a small tolerance)
		*	 {
		*		P projected onto N is the penetration vector.
		*	 }
		*	 else
		*	 {
		*		extend polytope to furtherPointInNormal
		*		loop to 2
		*	 }
		*/

		//using support function to find furthest point in direction of the normal of closest triangle.
		//ONLY EXTEND if the point is further away than the closest triangle we calculated.
		//Adding a little tolerance to prevent infinitely small extensions (for spheres and cylinders);
		if (distanceToFurthestPointInNormal > closestFaceDistance + EPA_TOLERANCE)
		{
			ExtendPolytopeWithNewPoint(polytope, furtherPointInNormal);
		}
		else
		{
			//break;
		}
	}

	//We have closest face
	Vector3 normal = polytope[closestFaceIndex].normal;
	float depth = DistanceFromPointToPlane(Vector3(0.0f, 0.0f, 0.0f), polytope[closestFaceIndex].points[0], polytope[closestFaceIndex].normal);
	//depth = closestFaceDistance;
	//Vector3 penVector = normal * depth;

	//for (size_t i = 0; i < polytope.size(); i++)
	//{
	//	//if (closestFaceIndex != i)
	//	{
	//		polytope[i].Render(Vector3(1.0f, 1.0f, 1.0f));
	//	}
	//}

	//polytope[closestFaceIndex].Render(Vector3(0.0f, 1.0f, 0.0f));
	//r.DrawDebugLine(polytope[closestFaceIndex].points[0], polytope[closestFaceIndex].centre, Vector3(0.0f, 1.0f, 0.0f));
	//r.DrawDebugLine(polytope[closestFaceIndex].points[1], polytope[closestFaceIndex].centre, Vector3(0.0f, 1.0f, 0.0f));
	//r.DrawDebugLine(polytope[closestFaceIndex].points[2], polytope[closestFaceIndex].centre, Vector3(0.0f, 1.0f, 0.0f));

	manifold->Depth = depth;
	manifold->ContactPoints.push_back(Contact());
	manifold->ContactPoints[0].Normal = normal;
	manifold->ContactPoints[1].Normal = Vector3(-normal.x, -normal.y, -normal.z);

	return result;
}