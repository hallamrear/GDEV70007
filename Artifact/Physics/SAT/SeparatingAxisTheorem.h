#pragma once

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

class SeparatingAxisTheorem
{
private:	
	static bool IsMinkowskiFace(const Vector3& BxA, const Vector3& DxC, const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d);
	static bool ParallelTest(const Vector3& crossBA, const Vector3& crossDC);

	static float EdgeEdgeDistance(const Vector3& crossBA, const Vector3& crossDC, const Vector3& edgeA_WSOrigin, const Vector3& edgeB_WSOrigin, const Vector3& colliderWSOrigin);

	static void MapVertexToGaussMap(const Matrix4x4& worldMatrix, const ConvexHullHalfEdge& edge,
		   Vector3& edgeOrigin, Vector3& arcEdge, Vector3& vertexA, Vector3& vertexB);

	static FaceQuery QueryFace(const ConvexHull& hullA, const Matrix4x4& hullAMatrix, const ConvexHull& hullB, const Matrix4x4& hullBMatrix);
	static EdgeQuery QueryEdge(const ConvexHull& hullA, const Matrix4x4& hullAMatrix, const ConvexHull& hullB, const Matrix4x4& hullBMatrix);

public:
	static bool CheckCollision(const ConvexHull& hullA, const Matrix4x4& hullAMatrix, const ConvexHull& hullB, const Matrix4x4& hullBMatrix, CollisionManifold* manifold = nullptr);
};

