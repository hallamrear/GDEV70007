#pragma once
#include <Physics/Structures.h>

class EPA
{
private:
	struct Edge
	{
		glm::vec3 start = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 end = glm::vec3(0.0f, 0.0f, 0.0f);

		Edge();
		Edge(const glm::vec3& a, const glm::vec3& b);

		bool operator==(const Edge& rhs)
		{
			return ((start.x == rhs.end.x &&
					 start.y == rhs.end.y &&
					 start.z == rhs.end.z)

				 && (end.x == rhs.end.x&&
				  	 end.y == rhs.end.y&&
				 	 end.z == rhs.end.z  ));
		}
	};

	struct Face
	{
		glm::vec3 points[3];
		Edge edges[3];
		glm::vec3 normal;
		glm::vec3 centre;

		Face(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
	};

	typedef std::vector<Face> Polytope;

	/// <summary>
	/// TODO : Find a proper place for this function. Probably HelperFunctions.h
	///	Calculate distance of point to plane.
	/// </summary>
	/// <param name="point">Point used to determine distance from plane.</param>
	/// <param name="planeOrigin">Position of the plane</param>
	/// <param name="planeNormal">Normal of the plane</param>
	/// <returns></returns>
	static float DistanceFromPointToPlane(const glm::vec3& point, const glm::vec3& planeOrigin, const glm::vec3& planeNormal);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="closestFaceIndex"></param>
	/// <param name="closestFaceDistance"></param>
	/// <param name="polytope"></param>
	static void DetermineClosestFaceToOrigin(size_t& closestFaceIndex, float& closestFaceDistance, const Polytope& polytope);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="polytope"></param>
	/// <param name="newVertex"></param>
	static void ExtendPolytopeWithNewPoint(Polytope& polytope, const SupportVertex& newVertex);
	static int GetFaceNormals(std::vector<glm::vec3>& normals, std::vector<float>& distances, const std::vector<SupportVertex>& simplex, const std::vector<size_t>& faces);
	static void AddEdgeIfUnique(std::vector<std::pair<size_t, size_t>>& edgeList, const std::vector<size_t>& faceList, const size_t& indexA, const size_t& indexB);


public:
	static EPA_Result GetCollisionDetails(std::vector<SupportVertex>& simplex, const Collider& colliderA, const Collider& colliderB, CollisionManifold* manifold = nullptr);
};