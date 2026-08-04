#include <pch.h>
#include <Physics/EPA/EPA.h>
#include <System/Maths.h>

EPA::Edge::Edge()
{

}

EPA::Edge::Edge(const glm::vec3& a, const glm::vec3& b)
{
	start = a;
	end = b;
}

EPA::Face::Face(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
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

float EPA::DistanceFromPointToPlane(const glm::vec3& point, const glm::vec3& planeOrigin, const glm::vec3& planeNormal)
{
	glm::vec3 diff = point - planeOrigin;
	float length = glm::dot(diff, planeNormal);
	return fabsf(length);
}

void EPA::DetermineClosestFaceToOrigin(size_t& closestFaceIndex, float& closestFaceDistance, const Polytope& polytope)
{
	closestFaceIndex = (size_t)-1;
	closestFaceDistance = FLT_MAX;

	for (size_t i = 0; i < polytope.size(); i++)
	{
		const Face& face = polytope[i];
		float distance = DistanceFromPointToPlane(glm::vec3(0.0f, 0.0f, 0.0f), face.centre, face.normal);

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
		glm::vec3 triToNew = glm::vec3(
			newVertex.MinkowskiDifference.x - face.centre.x, 
			newVertex.MinkowskiDifference.y - face.centre.y,
			newVertex.MinkowskiDifference.z - face.centre.z);
		triToNew = glm::normalize(triToNew);

		float dot = glm::dot(face.normal, triToNew);

		if (dot > 0.0f)
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

int EPA::GetFaceNormals(std::vector<glm::vec3>& normals, std::vector<float>& distances, const std::vector<SupportVertex>& simplex, const std::vector<size_t>& faces)
{
	int minIndex = -1;
	float minDistance = FLT_MAX;

	for (int i = 0; i < faces.size(); i += 3)
	{
		glm::vec3 a = simplex[faces[i + 0]].MinkowskiDifference;
		glm::vec3 b = simplex[faces[i + 1]].MinkowskiDifference;
		glm::vec3 c = simplex[faces[i + 2]].MinkowskiDifference;

		glm::vec3 lineAB = glm::vec3(b.x - a.x, b.y - a.y, b.z - a.z);
		glm::vec3 lineAC = glm::vec3(c.x - a.x, c.y - a.y, c.z - a.z);

		glm::vec3 cross = glm::cross(lineAB, lineAC);
		glm::vec3 normal= glm::normalize(cross);

		//Project face line onto normal.
		float distance = glm::dot(normal, a);

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
	std::vector<glm::vec3> faceNormals = std::vector<glm::vec3>();
	std::vector<float> faceDistances = std::vector<float>();
	size_t minIndex = GetFaceNormals(faceNormals, faceDistances, simplex, polytopeFaces);

	float minDistance = FLT_MAX;
	glm::vec3 minNormal = glm::vec3();

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

		float supportDistance = glm::dot(minNormal, supportVertex.MinkowskiDifference);

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

			std::vector<glm::vec3> newNormals = std::vector<glm::vec3>();
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

	glm::vec3 triNormal = Maths::GetNormalOfTriangle(triangle.Points[0].MinkowskiDifference, triangle.Points[1].MinkowskiDifference, triangle.Points[2].MinkowskiDifference);
	triangle.Normal = { triNormal.x, triNormal.y, triNormal.z };

	result.Tri = triangle;
	result.Depth = minDistance + 0.001f;

	return result;
}

#define EPA_CONVERGENCE_TOLERANCE 0.0001f
#define EPA_MAX_POLYTOPE_FACES 64
#define EPA_MAX_POLYTOPE_LOOSE_EDGES 32
#define EPA_MAX_ITERATIONS 64

using namespace glm;

glm::vec3 EPA::ConstructManifold(const Collider& colliderA, const Collider& colliderB, const Simplex& simplex, CollisionManifold& manifold)
{
	//Cache maximum possible faces (3x points + normal per face);
	vec3 polytope[EPA_MAX_POLYTOPE_FACES][4];

	//Construct initial polytope from GJK simplex.
	//ABC
	polytope[0][0] = simplex[0].MinkowskiDifference;
	polytope[0][1] = simplex[1].MinkowskiDifference;
	polytope[0][2] = simplex[2].MinkowskiDifference;
	polytope[0][3] = normalize(cross(polytope[0][1] - polytope[0][0], polytope[0][2] - polytope[0][0]));
	//ACD
	polytope[1][0] = simplex[0].MinkowskiDifference;
	polytope[1][1] = simplex[2].MinkowskiDifference;
	polytope[1][2] = simplex[3].MinkowskiDifference;
	polytope[1][3] = normalize(cross(polytope[1][1] - polytope[1][0], polytope[1][2] - polytope[1][0]));
	//ADB
	polytope[2][0] = simplex[0].MinkowskiDifference;
	polytope[2][1] = simplex[3].MinkowskiDifference;
	polytope[2][2] = simplex[1].MinkowskiDifference;
	polytope[2][3] = normalize(cross(polytope[2][1] - polytope[2][0], polytope[2][2] - polytope[2][0]));
	//BDC
	polytope[3][0] = simplex[1].MinkowskiDifference;
	polytope[3][1] = simplex[3].MinkowskiDifference;
	polytope[3][2] = simplex[2].MinkowskiDifference;
	polytope[3][3] = normalize(cross(polytope[3][1] - polytope[3][0], polytope[3][2] - polytope[3][0]));

	int faceCount = 4;
	int nearestFace = -1;

	for (size_t itr = 0; itr < EPA_MAX_ITERATIONS; itr++)
	{
		#pragma region Determine Nearest Face
		//Determine Nearest Face
		float minDistance = dot(polytope[0][0], polytope[0][3]);
		nearestFace = 0;

		for (size_t i = 1; i < faceCount; i++)
		{
			float d = dot(polytope[i][0], polytope[i][3]);

			if (d < minDistance)
			{
				nearestFace = (int)i;
				minDistance = d;
			}
		}
		#pragma endregion

		vec3 direction = normalize(polytope[nearestFace][3]);
		SupportVertex sv = SupportVertex::GetSupportVertex(colliderA, colliderB, direction);

		if (dot(sv.MinkowskiDifference, direction) - minDistance < EPA_CONVERGENCE_TOLERANCE)
		{
			//Convergence on the origin!
			glm::vec3 MTV = polytope[nearestFace][3] * dot(sv.MinkowskiDifference, direction);
			manifold.Normal = { MTV.x, MTV.y, MTV.z };
			return MTV;
		}

		vec3 looseEdges[EPA_MAX_POLYTOPE_LOOSE_EDGES][2];
		int looseEdgeCount = 0;

		for (size_t i = 0; i < faceCount; i++)
		{
			//if face triangle faces the support point, remove it to expand polytope.
			if (Maths::SameDirection(polytope[i][3], sv.MinkowskiDifference - polytope[i][0]))
			{
				//Add old triangle edges to edge list.

				for (size_t t = 0; t < 3; t++)
				{
					vec3 edges[2] = { polytope[i][t], polytope[i][(t + 1) % 3] };

					bool duplicateEdge = false;

					for (size_t e = 0; e < looseEdgeCount; e++)
					{
						if(looseEdges[e][1] == edges[0] && looseEdges[e][0] == edges[1])
						{
							//If its an already hanging edge we gotta clean it up as it can onnly be shared by 2 tris.
							looseEdges[e][0] = looseEdges[looseEdgeCount - 1][0];
							looseEdges[e][1] = looseEdges[looseEdgeCount - 1][1];
							looseEdgeCount--;
							duplicateEdge = true;
							e = looseEdgeCount;
						}
					}

					if (duplicateEdge == false)
					{
						assert(looseEdgeCount < EPA_MAX_POLYTOPE_LOOSE_EDGES);

						looseEdges[looseEdgeCount][0] = edges[0];
						looseEdges[looseEdgeCount][1] = edges[1];
						looseEdgeCount++;
					}
				}

				//Cut out the triangle.
				polytope[i][0] = polytope[faceCount - 1][0];
				polytope[i][1] = polytope[faceCount - 1][1];
				polytope[i][2] = polytope[faceCount - 1][2];
				polytope[i][3] = polytope[faceCount - 1][3];
				faceCount--;
			}
		}

		//Rebuild new polytope using new support vertex

		for (size_t i = 0; i < looseEdgeCount; i++)
		{
			assert(faceCount < EPA_MAX_POLYTOPE_FACES);

			polytope[faceCount][0] = looseEdges[i][0];
			polytope[faceCount][1] = looseEdges[i][1];
			polytope[faceCount][2] = sv.MinkowskiDifference;
			polytope[faceCount][3] = normalize(cross(looseEdges[i][0] - looseEdges[i][1], looseEdges[i][0] - sv.MinkowskiDifference));

			const float offset = 0.0000001f;
			//Make sure its facing the right direction.
			if (dot(polytope[faceCount][0], polytope[faceCount][3]) + offset < 0)
			{
				//Flip winding order
				vec3 holder = polytope[faceCount][0];
				polytope[faceCount][0] = polytope[faceCount][1];
				polytope[faceCount][1] = holder;
				polytope[faceCount][3] = -polytope[faceCount][3];
			}

			faceCount++;

		}
	}

	printf("Expanding Polytrope Algorithm failed to converge on a encompassing polytope. Dropping out due to iteration count. Returning most recent nearest point.");	
	return polytope[nearestFace][3] * dot(polytope[nearestFace][0], polytope[nearestFace][3]);
}