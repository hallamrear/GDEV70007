#pragma once
#include <Physics/Structures.h>

/// <summary>
/// Implementation of the Gilbert-Johnson-Keethri (GJK) distance algorithm based on Casey Muratori's Video (https://www.youtube.com/watch?v=Qupqu1xe7Io) and Real-Time Collision Detection by Christer Ericson
/// </summary>
class GJK
{
private:
	typedef std::vector<SupportVertex> Simplex;

	static bool Line(Simplex& simplex, Vector3& direction);
	static bool Triangle(Simplex& simplex, Vector3& direction);
	static bool Tetrahedron(Simplex& simplex, Vector3& direction);
	static bool UpdateSimplex(Simplex& simplex, Vector3& direction);

	static bool Line(Simplex& simplex, glm::vec3& direction);
	static bool Triangle(Simplex& simplex, glm::vec3& direction);
	static bool Tetrahedron(Simplex& simplex, glm::vec3& direction);
	static bool UpdateSimplex(Simplex& simplex, glm::vec3& direction);

public:
	static bool CheckCollision(const Collider& colliderA, const Collider& colliderB, CollisionManifold* manifold = nullptr);
};