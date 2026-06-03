#pragma once
#include <Physics/Structures.h>
#include <System/Maths.h>
#include <System/Maths/Plane.h>
#include <System/Maths/Triangle.h>
#include <glm/glm.hpp>

#define SAT_EPSILON (0.00001f)

class Collider;
class ConvexHull;
struct ConvexHullFace;
struct ConvexHullHalfEdge;
struct CollisionManifold;

struct SAT_Query
{
	float Distance = -INFINITY;

private:
};

struct EdgeQuery : public SAT_Query
{
	ConvexHullHalfEdge* EdgeA = nullptr;
	ConvexHullHalfEdge* EdgeB = nullptr;
};

struct FaceQuery : public SAT_Query
{
	ConvexHullFace* Face = nullptr;
};

struct SAT_Result
{
	FaceQuery FaceTestA;
	FaceQuery FaceTestB;
	EdgeQuery EdgeTest;
};

struct EdgeContact
{
	EdgeQuery Query;
	Vector3 MidPoint = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 Axis = Vector3(0.0f, 0.0f, 0.0f);
};

struct FaceContact
{
	FaceQuery Query;
	std::vector<Vector3> vertices;
};

class SeparatingAxisTheorem
{
private:	
	static EdgeContact CreateEdgeContacts(const EdgeQuery& edgeQuery, const ConvexHull& hullA, const glm::mat4x4& hullAMatrix, const ConvexHull& hullB, const glm::mat4x4& hullBMatrix);
	static std::vector<Contact> ConvertContactToWorldSpace(const FaceContact& faceContact);
	static Contact ConvertContactToWorldSpace(const EdgeContact& edgeContact);
	static void ConstructContactManifold(CollisionManifold& manifold, const SAT_Result& satResult, const ConvexHull& hullA, const glm::mat4x4& hullAMatrix, const ConvexHull& hullB, const glm::mat4x4& hullBMatrix);

	static bool IsMinkowskiFace(const glm::vec3& BxA, const glm::vec3& DxC, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d);
	static bool ParallelTest(const glm::vec3& crossBA, const glm::vec3& crossDC);

	static float EdgeEdgeDistance(const glm::vec3& crossBA, const glm::vec3& crossDC, const glm::vec3& edgeA_WSOrigin, const glm::vec3& edgeB_WSOrigin, const glm::vec3& colliderWSOrigin);

	static void MapVertexToGaussMap(const glm::mat3x3& worldMatrix, const ConvexHullHalfEdge& edge,
		   glm::vec3& edgeOrigin, glm::vec3& arcEdge, glm::vec3& vertexA, glm::vec3& vertexB);

	static FaceQuery QueryFace(const ConvexHull& hullA, const glm::mat4x4& hullAMatrix, const ConvexHull& hullB, const glm::mat4x4& hullBMatrix);
	static EdgeQuery QueryEdge(const ConvexHull& hullA, const glm::mat4x4& hullAMatrix, const ConvexHull& hullB, const glm::mat4x4& hullBMatrix);

public:
	static bool CheckCollision(SAT_Result& result, const ConvexHull& hullA, const glm::mat4x4& hullAMatrix, const ConvexHull& hullB, const glm::mat4x4& hullBMatrix, CollisionManifold* manifold = nullptr);
};

