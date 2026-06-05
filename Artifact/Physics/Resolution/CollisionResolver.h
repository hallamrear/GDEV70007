#pragma once
#include <glm/glm.hpp>
#include <Physics/Resolution/Jacobian.h>
#include <World/Entity.h>

struct ConstrainedContact;
struct CollisionManifold;
struct Constraint;
struct Contact;
struct Jacobian;

struct InverseMassPair
{
	float inverseContactMass;
	glm::vec3 iTensor;
};

using InverseMassVector = std::array<InverseMassPair, 2>;
using InverseMassMatrix = std::vector<InverseMassVector>;

class CollisionResolver
{
private:
	static void CalculateConstraintBounds(glm::vec2& bounds, const JACOBIAN_TYPE& type, const ConstrainedContact& contact);
	static float DetermineVelocityConstraint(const ConstrainedContact& contact, const JacobianVector& jacobianVector);
public:
	static int RESOLUTION_ITERATIONS;
	static float AIR_FRICTION;
	static float ALLOWED_INTERSECTION;
	static float BAUMGARTE_STABILISATION_VALUE;

	static std::vector<Jacobian> CalculateJacobians(std::vector<ConstrainedContact>& constrainedContacts);
	static InverseMassMatrix ComputeInverseMassMatrix(const std::vector<ConstrainedContact>& contacts);

	static void ConvertContactManifoldsToConstrainedPoints(const std::vector<CollisionManifold>& collisionManifolds, std::vector<ConstrainedContact>& constraintContacts);
	static void PrepareConstrainedContacts(std::vector<ConstrainedContact>& constrainedContacts);

	static std::vector<Constraint> CalculateConstraints(const std::vector<Jacobian>& jacobians, const std::vector<ConstrainedContact>& contacts);
	static void ResolveVelocityConstraint(const float& deltaTime, const std::vector<Jacobian>& jacobians, const InverseMassMatrix& inverseMassMatrix, std::vector<Constraint>& constraints, std::vector<ConstrainedContact>& contacts);
	static void ResolvePositionConstraint(const float& deltaTime, const std::vector<Jacobian>& jacobians, const InverseMassMatrix& inverseMassMatrix, std::vector<Constraint>& constraints, std::vector<ConstrainedContact>& contacts);

	static void SoftResolveCollision(const CollisionManifold& manifold);

    //Position correction - Dividing inv mass of object by total mass to properly scale each way.
    //A heavier object will move less than a lighter object.
    static void PositionalCorrection(Rigidbody& objectA, Rigidbody& objectB, const CollisionManifold& manifold, const Contact& contact);

};