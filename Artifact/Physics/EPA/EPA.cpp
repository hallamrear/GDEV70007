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

int EPA::GetFaceNormals(std::vector<Vector3>& normals, std::vector<float>& distances, const std::vector<SupportVertex>& simplex, const std::vector<size_t>& faces)
{
	int minIndex = -1;
	float minDistance = FLT_MAX;

	for (int i = 0; i < faces.size(); i += 3)
	{
		Vector3 a = simplex[faces[i + 0]].MinkowskiDifference;
		Vector3 b = simplex[faces[i + 1]].MinkowskiDifference;
		Vector3 c = simplex[faces[i + 2]].MinkowskiDifference;

		Vector3 lineAB = Vector3(b.x - a.x, b.y - a.y, b.z - a.z);
		Vector3 lineAC = Vector3(c.x - a.x, c.y - a.y, c.z - a.z);


		Vector3 cross = Maths::Cross(lineAB, lineAC);
		Vector3 normal;
		DirectX::XMStoreFloat3(&normal, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&cross)));

		//Project face line onto normal.
		float distance = Maths::Dot(normal, a);

		//Ensure normal is facing the right direction.
		if (distance < 0)
		{
			normal.x *= -1.0f;
			normal.y *= -1.0f;
			normal.z *= -1.0f;

			distance *= -1.0f;
		}

		normals.push_back(normal);
		distances.push_back(distance);

		if (distance < minDistance)
		{
			//Index is the start of the triangle indices in 'faces'
			minIndex = i / 3;
			minDistance = distance;
		}

	}

	return minIndex;
}

void EPA::AddEdgeIfUnique(std::vector<std::pair<size_t, size_t>>& edgeList, const std::vector<size_t>& faceList, const size_t& indexA, const size_t& indexB)
{
	auto edgeItr = std::find(edgeList.begin(), edgeList.end(), std::make_pair(faceList[indexB], faceList[indexA]));

	if (edgeItr != edgeList.end())
	{
		//if it does exist, get rid.
		edgeList.erase(edgeItr);
	}
	else
	{
		//if it doesnt, add it.
		edgeList.emplace_back(faceList[indexA], faceList[indexB]);
	}
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

	//Construct a new polytope (fancy word for 3d simplex) that we can add to without adjusting the original.
	std::vector<SupportVertex> polytope(simplex.begin(), simplex.end());

	int iterations = 0;

	std::vector<size_t> polytopeFaces =
	{
		0, 1, 2,
		0, 3, 1,
		0, 2, 3,
		1, 3, 2
	};

	//Get and store the inital normals and distance values for each face.
	std::vector<Vector3> faceNormals = std::vector<Vector3>();
	std::vector<float> faceDistances = std::vector<float>();
	size_t minIndex = GetFaceNormals(faceNormals, faceDistances, simplex, polytopeFaces);

	float minDistance = FLT_MAX;
	Vector3 minNormal = Vector3();

	while (minDistance == FLT_MAX)
	{
		if (iterations > 16)
		{
			for (size_t i = 0; i < faceDistances.size(); i++)
			{
				if (faceDistances[i] < minDistance)
				{
					minDistance = faceDistances[i];
					minIndex = i;
				}
			}
			break;
		}

		//Rather than generating a normal like in 2D EPA,
		//we get a normal from our pregenerated list.
		minNormal = faceNormals[minIndex];
		minDistance = faceDistances[minIndex];

		//Once we have the furthest edge normal, get the support vertex in that direction.
		SupportVertex supportVertex = SupportVertex::GetSupportVertex(colliderA, colliderB, minNormal);
		//if this point is further out in the direction, we insert this vertex into the simplex and test again.

		float supportDistance = Maths::Dot(minNormal, supportVertex.MinkowskiDifference);

		if (abs(supportDistance - minDistance) > 0.001f)
		{
			minDistance = FLT_MAX;

			//Expanding polytrope in 3D does not just require adding a vertex
			//it needs to repair the faces. just adding a face however, 
			//may result in multiple identical support points.

			std::vector<std::pair<size_t, size_t>> uniqueEdgeList = std::vector<std::pair<size_t, size_t>>();

			for (size_t i = 0; i < faceNormals.size(); i++)
			{
				//Not just removing current face, but also every face pointing in that direciton.
				if (Maths::SameDirection(faceNormals[i], supportVertex.MinkowskiDifference))
				{
					size_t faceIndex = i * 3;

					//Add unique edges based on previous face.
					AddEdgeIfUnique(uniqueEdgeList, polytopeFaces, faceIndex + 0, faceIndex + 1);
					AddEdgeIfUnique(uniqueEdgeList, polytopeFaces, faceIndex + 1, faceIndex + 2);
					AddEdgeIfUnique(uniqueEdgeList, polytopeFaces, faceIndex + 2, faceIndex + 0);

					//Erase face and normals.
					polytopeFaces[faceIndex + 2] = polytopeFaces.back(); polytopeFaces.pop_back();
					polytopeFaces[faceIndex + 1] = polytopeFaces.back(); polytopeFaces.pop_back();
					polytopeFaces[faceIndex + 0] = polytopeFaces.back(); polytopeFaces.pop_back();

					faceNormals[i] = faceNormals.back(); // pop-erase
					faceNormals.pop_back();

					faceDistances[i] = faceDistances.back();
					faceDistances.pop_back();

					i--;
				}
			}

			//Add the new support point to the polytope 
			//and keep track of teh new faces from edges.
			std::vector<size_t> newFaces = std::vector<size_t>();
			for (std::pair<size_t, size_t> edge : uniqueEdgeList)
			{
				newFaces.push_back((size_t)edge.first);
				newFaces.push_back((size_t)edge.second);
				newFaces.push_back(polytope.size());
			}

			polytope.push_back(supportVertex);

			std::vector<Vector3> newNormals = std::vector<Vector3>();
			std::vector<float> newDistances = std::vector<float>();
			int newMinIndex = GetFaceNormals(newNormals, newDistances, polytope, newFaces);

			//Afer getting all the normals, check if that is the new closest face.
			//by comparing the new faceNormals against the originals.
			float oldDistance = FLT_MAX;

			for (size_t i = 0; i < faceDistances.size(); i++)
			{
				if (faceDistances[i] < oldDistance)
				{
					oldDistance = faceDistances[i];
					minIndex = i;
				}
			}

			if (newDistances[newMinIndex] < oldDistance)
			{
				minIndex = newMinIndex + faceDistances.size();
			}

			//Add new faces.
			polytopeFaces.insert(polytopeFaces.end(), newFaces.begin(), newFaces.end());
			//Add new normals.
			faceNormals.insert(faceNormals.end(), newNormals.begin(), newNormals.end());
			//Add new distances.
			faceDistances.insert(faceDistances.end(), newDistances.begin(), newDistances.end());

			iterations++;
		}
	}

	//Return the normal and depth (add a tiny amount to avoid multiple collisions).
	//manifold->Normal = glm::normalize(minNormal);

	EPA_Result::Triangle triangle;
	for (size_t i = 0; i < 3; i++)
	{
		size_t triFaceIndex = (minIndex * 3) + i;
		triangle.Points[i] = (polytope[polytopeFaces[triFaceIndex]]);
	}

	triangle.Normal = Maths::GetNormalOfTriangle(triangle.Points[0].MinkowskiDifference, triangle.Points[1].MinkowskiDifference, triangle.Points[2].MinkowskiDifference);

	result.Tri = triangle;
	result.Depth = minDistance + 0.001f;

	return result;
}

////
// 
// PENDING REWRITE
// PENDING REWRITE
// 
////

//EPA_Result EPA::GetCollisionDetails(std::vector<SupportVertex>& simplex, const Collider& colliderA, const Collider& colliderB, CollisionManifold* manifold)
//{
//	EPA_Result result = EPA_Result();
//
//	if (manifold == nullptr)
//	{
//		throw new std::exception("Called EPA::GetCollisionDetails with a nullptr manifold.\n");
//	}
//
//	if (simplex.size() < 4)
//	{
//		throw new std::exception("Called EPA::GetCollisionDetails with a simplex containing less than a 3d object.\n");
//	}
//
//	////////////////////////////////////////
//	//            HOW EPA WORKS           //
//	////////////////////////////////////////		
//		
//		 
//	// 1) Start with simplex from GJK
//	//Construct a new polytope (fancy word for 3d simplex) that we can add to without adjusting the original.
//	Polytope polytope = Polytope();
//	polytope.push_back(Face(simplex[0].MinkowskiDifference, simplex[1].MinkowskiDifference, simplex[2].MinkowskiDifference));
//	polytope.push_back(Face(simplex[0].MinkowskiDifference, simplex[3].MinkowskiDifference, simplex[1].MinkowskiDifference));
//	polytope.push_back(Face(simplex[0].MinkowskiDifference, simplex[2].MinkowskiDifference, simplex[3].MinkowskiDifference));
//	polytope.push_back(Face(simplex[1].MinkowskiDifference, simplex[3].MinkowskiDifference, simplex[2].MinkowskiDifference));
//
//	int iterations = 0;
//	size_t closestFaceIndex = (size_t)-1;
//	float closestFaceDistance = FLT_MAX;
//
//	constexpr int temp_iteration_cap = 16;
//	while (iterations < temp_iteration_cap)
//	//while (true)
//	{
//		iterations++;
//
//		assert(polytope.size() < 100 || polytope.size() != 0);
//
//		// 2) Calculate closest face (F) of polytope to origin
//		DetermineClosestFaceToOrigin(closestFaceIndex, closestFaceDistance, polytope);
//		
//		// 3) Find support point in direction of face F normal (N)
//		SupportVertex furtherPointInNormal = SupportVertex::GetSupportVertex(colliderA, colliderB, polytope[closestFaceIndex].normal);
//
//		float dot;
//		XMStoreFloat(&dot, DirectX::XMVector3Dot(XMLoadFloat3(&polytope[closestFaceIndex].normal), XMLoadFloat3(&furtherPointInNormal.MinkowskiDifference)));
//		float distanceToFurthestPointInNormal = dot;
//
//		static constexpr float EPA_TOLERANCE = 0.05f;
//
//		/* 
//		* 4) if (distance to P (furthest in Normal) and distance to F are within a small tolerance)
//		*	 {
//		*		P projected onto N is the penetration vector.
//		*	 }
//		*	 else
//		*	 {
//		*		extend polytope to furtherPointInNormal
//		*		loop to 2
//		*	 }
//		*/
//
//		//using support function to find furthest point in direction of the normal of closest triangle.
//		//ONLY EXTEND if the point is further away than the closest triangle we calculated.
//		//Adding a little tolerance to prevent infinitely small extensions (for spheres and cylinders);
//		if (distanceToFurthestPointInNormal > closestFaceDistance + EPA_TOLERANCE)
//		{
//			ExtendPolytopeWithNewPoint(polytope, furtherPointInNormal);
//		}
//		else
//		{
//			break;
//		}
//	}
//
//	//We have closest face
//	float depth = DistanceFromPointToPlane(Vector3(0.0f, 0.0f, 0.0f), polytope[closestFaceIndex].points[0], polytope[closestFaceIndex].normal);
//	manifold->Depth = depth;
//
//	//Return the normal and depth (add a tiny amount to avoid multiple collisions).
//	manifold->Depth = closestFaceDistance + FLT_EPSILON;
//
//	EPA_Result::Triangle triangle;
//	for (size_t i = 0; i < 3; i++)
//	{
//		size_t triFaceIndex = (closestFaceIndex * 3) + i;
//		triangle.Points[i].MinkowskiDifference = (polytope[triFaceIndex]).points[triFaceIndex];
//	}
//
//	triangle.Normal = Maths::GetNormalOfTriangle(triangle.Points[0].MinkowskiDifference, triangle.Points[1].MinkowskiDifference, triangle.Points[2].MinkowskiDifference);
//
//	result.Tri = triangle;
//
//	return result;
//}