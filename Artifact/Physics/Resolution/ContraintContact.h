#pragma once
#include <glm/glm.hpp>

class Rigidbody;
class Entity;

struct ConstrainedContact
{
	Entity* EntityA = nullptr;
	Entity* EntityB = nullptr;

	Rigidbody* BodyA = nullptr;
	Rigidbody* BodyB = nullptr;

	glm::vec3 ContactOrigins[2];
	glm::vec3 ContactLocalPositions[2];

	glm::vec3 ContactNormal;
	glm::vec3 ContactTangent;
	glm::vec3 ContactBitangent;
	
	glm::vec3 RelativeVelocity;

	glm::mat3x3 InverseInertiaTensors[2];

	glm::vec3 LinearVelocity[2];
	glm::vec3 AngularVelocity[2];
	glm::vec3 Positions[2];

	glm::vec4 Orientations[2];

	float InverseMasses[2];
	float InverseContactMasses[2];

	float CollisionDepth;
	float Restitution;
	float Friction;
	bool IsDynamics[2];

	//Gets relative velocity between the two bodys.
	static glm::vec3 ComputeRelativeVelocity(const ConstrainedContact& contact);

	/// Generates orthogonal basis vectors of contact normal.
	static void ComputeContactOrthoVectors(const glm::vec3& relativeContactVelocity,
		glm::vec3& normal,
		glm::vec3& binormal,
		glm::vec3& tangent);
};

