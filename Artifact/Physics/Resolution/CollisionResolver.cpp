#include "pch.h"
#include "CollisionResolver.h"
#include <Physics/Resolution/ContraintContact.h>
#include <Physics/Resolution/Constraint.h>
#include <Physics/Structures.h>
#include <World/Entity.h>
#include <Physics/Rigidbody.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

int CollisionResolver::RESOLUTION_ITERATIONS = 20;
float CollisionResolver::AIR_FRICTION = 0.9f;
float CollisionResolver::ALLOWED_INTERSECTION = 0.05f;
float CollisionResolver::BAUMGARTE_STABILISATION_VALUE = 0.15f;

void CollisionResolver::CalculateConstraintBounds(glm::vec2& bounds, const JACOBIAN_TYPE& type, const ConstrainedContact& contact)
{
	switch (type)
	{
	case JACOBIAN_TYPE::NORMAL:
	{
		bounds.x = 0.0f;
		bounds.y = INFINITY;
	}
	break;

	case JACOBIAN_TYPE::FRICTION:
	{
		float contactMass = (1.0f / (contact.InverseContactMasses[0] + contact.InverseContactMasses[1]));
		float gravitationFriction = AIR_FRICTION * 9.8f;
		float maxFriction =  contactMass * gravitationFriction;
		
		bounds.x = -maxFriction;
		bounds.y = maxFriction;
	}
	break;

	default:
		throw;
		break;
	}
}

std::vector<Jacobian> CollisionResolver::CalculateJacobians(std::vector<ConstrainedContact>& constrainedContacts)
{
	std::vector<Jacobian> jacobianEntries;

	for (int i = 0; i < (int)constrainedContacts.size(); i++)
	{
		Jacobian tangentialFriction = Jacobian(
			i,
			JACOBIAN_TYPE::FRICTION,
			Jacobian::CalculateFrictionalJacobian(
				constrainedContacts[i].ContactTangent,
				constrainedContacts[i].ContactLocalPositions[0],
				constrainedContacts[i].ContactLocalPositions[1]));

		Jacobian biTangentialFriction = Jacobian(
			i,
			JACOBIAN_TYPE::FRICTION, 
				Jacobian::CalculateFrictionalJacobian(
				constrainedContacts[i].ContactBitangent,
				constrainedContacts[i].ContactLocalPositions[0],
				constrainedContacts[i].ContactLocalPositions[1]));


		Jacobian resolutionJacobian = Jacobian(
			i,
			JACOBIAN_TYPE::NORMAL,		
				Jacobian::CalculateNormalJacobian(
				constrainedContacts[i].ContactNormal,
				constrainedContacts[i].ContactLocalPositions[0],
				constrainedContacts[i].ContactLocalPositions[1]));


		jacobianEntries.push_back(tangentialFriction);
		jacobianEntries.push_back(biTangentialFriction);
		jacobianEntries.push_back(resolutionJacobian);
	}

	return jacobianEntries;
}

InverseMassMatrix CollisionResolver::ComputeInverseMassMatrix(const std::vector<ConstrainedContact>& contacts)
{
	InverseMassMatrix matrix;

	for (size_t c = 0; c < contacts.size(); c++)
	{
		const ConstrainedContact& contact = contacts[c];

		InverseMassVector vector;

		for (size_t i = 0; i < 2; i++)
		{
			vector[i].inverseContactMass = contact.InverseContactMasses[i];
			vector[i].iTensor = contact.InverseInertiaTensors[i];
		}

		matrix.push_back(vector);
	}

	return matrix;
}

struct EqualVectorPredicate
{
	bool operator()(const glm::vec3& l, const glm::vec3& r)
	{
		static constexpr float epsilon = 0.00001f;
		return abs(r.x - l.x) < epsilon && abs(r.y - l.y) < epsilon && abs(r.z - l.z) < epsilon;
	}
};

struct LessThanPredicate {
	bool operator()(const glm::vec3& l, const glm::vec3& r)
	{
		static constexpr float epsilon = 0.00001f;

		if (l.x != r.x)
			return (r.x - l.x) > epsilon;

		if (l.y != r.y)
			return (r.y - l.y) > epsilon;

		return (r.z - l.z) > epsilon;
	}
};

void CollisionResolver::ConvertContactManifoldsToConstrainedPoints(const std::vector<CollisionManifold>& collisionManifolds, std::vector<ConstrainedContact>& constraintContacts)
{
	constraintContacts.clear();

	for (size_t i = 0; i < collisionManifolds.size(); i++)
	{
		const CollisionManifold& manifold = collisionManifolds[i];

		int contactPointCount = (int)manifold.ContactPoints.size();
		float invContactPointCount = 1.0f / contactPointCount;

		//Properly convert data in individual spaces.

		for (int c = 0; c < contactPointCount; c++)
		{
			ConstrainedContact constrainedContact;
			Entity* entityA = manifold.CollisionPair.first;
			Rigidbody& rigidbodyA = entityA->GetRigidbody();
			Entity* entityB = manifold.CollisionPair.second;
			Rigidbody& rigidbodyB = entityB->GetRigidbody();

			constrainedContact.EntityA = entityA;
			constrainedContact.EntityB = entityB;
			constrainedContact.BodyA = &rigidbodyA;
			constrainedContact.BodyB = &rigidbodyB;

			constrainedContact.ContactNormal = glm::normalize(glm::vec3(manifold.Normal.x, manifold.Normal.y, manifold.Normal.z));
			constrainedContact.CollisionDepth = manifold.ContactPoints[c].Depth;
			
			constrainedContact.IsDynamics[0] = !rigidbodyA.IsStatic;
			constrainedContact.IsDynamics[1] = !rigidbodyB.IsStatic;

			constrainedContact.LinearVelocity[0] = glm::vec3(rigidbodyA.LinearVelocity.x, rigidbodyA.LinearVelocity.y, rigidbodyA.LinearVelocity.z);
			constrainedContact.LinearVelocity[1] = glm::vec3(rigidbodyB.LinearVelocity.x, rigidbodyB.LinearVelocity.y, rigidbodyB.LinearVelocity.z);

			constrainedContact.AngularVelocity[0] = glm::vec3(rigidbodyA.AngularVelocity.x, rigidbodyA.AngularVelocity.y, rigidbodyA.AngularVelocity.z);
			constrainedContact.AngularVelocity[1] = glm::vec3(rigidbodyB.AngularVelocity.x, rigidbodyB.AngularVelocity.y, rigidbodyB.AngularVelocity.z);

			constrainedContact.Positions[0] = glm::vec3(entityA->GetPosition().x, entityA->GetPosition().y, entityA->GetPosition().z);
			constrainedContact.Positions[1] = glm::vec3(entityB->GetPosition().x, entityB->GetPosition().y, entityB->GetPosition().z);

			constrainedContact.Orientations[0] = glm::vec4(entityA->GetRotation().x, entityA->GetRotation().y, entityA->GetRotation().z, entityA->GetRotation().w);
			constrainedContact.Orientations[1] = glm::vec4(entityB->GetPosition().x, entityB->GetPosition().y, entityB->GetPosition().z, entityB->GetRotation().w);

			constrainedContact.ContactOrigins[0] = constrainedContact.Positions[0];
			constrainedContact.ContactOrigins[1] = constrainedContact.Positions[1];

			constrainedContact.ContactLocalPositions[0].x = manifold.ContactPoints[c].HitPoint.x - constrainedContact.ContactOrigins[0].x;
			constrainedContact.ContactLocalPositions[0].y = manifold.ContactPoints[c].HitPoint.y - constrainedContact.ContactOrigins[0].y;
			constrainedContact.ContactLocalPositions[0].z = manifold.ContactPoints[c].HitPoint.z - constrainedContact.ContactOrigins[0].z;
			constrainedContact.ContactLocalPositions[1].x = manifold.ContactPoints[c].HitPoint.x - constrainedContact.ContactOrigins[1].x;
			constrainedContact.ContactLocalPositions[1].y = manifold.ContactPoints[c].HitPoint.y - constrainedContact.ContactOrigins[1].y;
			constrainedContact.ContactLocalPositions[1].z = manifold.ContactPoints[c].HitPoint.z - constrainedContact.ContactOrigins[1].z;

			constrainedContact.InverseContactMasses[0] = rigidbodyA.GetInverseMass() * invContactPointCount;
			constrainedContact.InverseContactMasses[1] = rigidbodyB.GetInverseMass() * invContactPointCount;
			
			constrainedContact.InverseInertiaTensors[0] = glm::inverse(glm::scale(rigidbodyA.InverseInertiaTensor));
			constrainedContact.InverseInertiaTensors[1] = glm::inverse(glm::scale(rigidbodyB.InverseInertiaTensor));
	
			//todo : potentially clamp the normal and offsets?

			constraintContacts.push_back(constrainedContact);
		}
	}
}

void CollisionResolver::PrepareConstrainedContacts(std::vector<ConstrainedContact>& constrainedContacts)
{
	for (auto& contact : constrainedContacts)
	{
		contact.RelativeVelocity = ConstrainedContact::ComputeRelativeVelocity(contact);
		ConstrainedContact::ComputeContactOrthoVectors(contact.RelativeVelocity, contact.ContactNormal, contact.ContactBitangent, contact.ContactTangent);
	}
}

float CollisionResolver::DetermineVelocityConstraint(const ConstrainedContact& contact, const JacobianVector& jacobianVector)
{
	//Body A
	glm::vec3 impulseDirection = jacobianVector.v[0];
	glm::vec3 torqueDirection = jacobianVector.v[1];

	float result = glm::dot(contact.LinearVelocity[0], impulseDirection) + glm::dot(contact.AngularVelocity[0], torqueDirection);

	//Add body B
	impulseDirection = jacobianVector.v[2];
	torqueDirection = jacobianVector.v[3];
	result += glm::dot(contact.LinearVelocity[1], impulseDirection) + glm::dot(contact.AngularVelocity[1], torqueDirection);

	return result;
}

std::vector<Constraint> CollisionResolver::CalculateConstraints(const std::vector<Jacobian>& jacobians, const std::vector<ConstrainedContact>& contacts)
{
	std::vector<Constraint> constraints;

	for (size_t j = 0; j < jacobians.size(); j++)
	{
		const Jacobian& jacobian = jacobians[j];
		const ConstrainedContact& contact = contacts[j / 3];

		Constraint constraint;
		constraint.PositionalError = contact.CollisionDepth;
		constraint.VelocityError = DetermineVelocityConstraint(contact, jacobian.Vector);

		//Find effective inverse masses of each coeff (JM-1JT?)
		JacobianVector inverseJTranspose =
		{
			 jacobian.Vector.v[0] * contact.InverseContactMasses[0],
			 contact.InverseInertiaTensors[0] * jacobian.Vector.v[1],
			 jacobian.Vector.v[2] * contact.InverseContactMasses[1],
			 contact.InverseInertiaTensors[1] * jacobian.Vector.v[3],
		};

		float inverseEffectiveMass =
			glm::dot(jacobian.Vector.v[0], inverseJTranspose.v[0]) +
			glm::dot(jacobian.Vector.v[1], inverseJTranspose.v[1]) +
			glm::dot(jacobian.Vector.v[2], inverseJTranspose.v[2]) +
			glm::dot(jacobian.Vector.v[3], inverseJTranspose.v[3]);

		constraint.EffectiveMass = (1.0f / inverseEffectiveMass);
		constraint.EffectiveMass = inverseEffectiveMass;

		CalculateConstraintBounds(constraint.ImpulseBounds, jacobian.Type, contact);

		constraints.push_back(constraint);
	}

	return constraints;
}

void CollisionResolver::ResolveVelocityConstraint(const double& deltaTime, const std::vector<Jacobian>& jacobians, const InverseMassMatrix& inverseMassMatrix, std::vector<Constraint>& constraints, std::vector<ConstrainedContact>& contacts)
{
	UNREFERENCED_PARAMETER(deltaTime);

	for (size_t itr = 0; itr < RESOLUTION_ITERATIONS; itr++)
	{
		//Solving frictional constraints first.
		for (int i = 0; i < constraints.size(); i++)
		{
			Constraint& constraint = constraints[i];
			const Jacobian& jacobian = jacobians[i];
			ConstrainedContact& contact = contacts[jacobian.Contact];

			if (jacobian.Type != JACOBIAN_TYPE::FRICTION)
				continue;

			float velocityError = DetermineVelocityConstraint(contact, jacobian.Vector);

			float& impulseSum = constraint.ImpulseSum;
			float oldImpulse = constraint.ImpulseSum;

			float  impulse = constraint.EffectiveMass * -velocityError;
			impulseSum = std::clamp(impulseSum + impulse, constraint.ImpulseBounds.x, constraint.ImpulseBounds.y);
			impulse = impulseSum - oldImpulse;

			//deltaV = M-1JTtheta, J(V+DeltaV) = 0
			//J(V + (M-1JTtheta) = 0
						
			//V0 + delta V
			Rigidbody& bodyA = *contact.BodyA;
			glm::vec3 linearImpulseA = jacobian.Vector.v[0] * impulse * inverseMassMatrix[jacobian.Contact][0].inverseContactMass;
			glm::vec3 angularImpulseA{ inverseMassMatrix[jacobian.Contact][0].iTensor * jacobian.Vector.v[1] * impulse };
			bodyA.LinearVelocity.x += linearImpulseA.x;
			bodyA.LinearVelocity.y += linearImpulseA.y;
			bodyA.LinearVelocity.z += linearImpulseA.z;
			bodyA.AngularVelocity.x += angularImpulseA.x;
			bodyA.AngularVelocity.y += angularImpulseA.y;
			bodyA.AngularVelocity.z += angularImpulseA.z;

			//V1 + delta V
			Rigidbody& bodyB = *contact.BodyB;
			glm::vec3 linearImpulseB = jacobian.Vector.v[2] * impulse * inverseMassMatrix[jacobian.Contact][1].inverseContactMass;
			glm::vec3 angularImpulseB = inverseMassMatrix[jacobian.Contact][1].iTensor * jacobian.Vector.v[3] * impulse;

			bodyB.LinearVelocity.x += linearImpulseB.x;
			bodyB.LinearVelocity.y += linearImpulseB.y;
			bodyB.LinearVelocity.z += linearImpulseB.z;
			bodyB.AngularVelocity.x += angularImpulseB.x;
			bodyB.AngularVelocity.y += angularImpulseB.y;
			bodyB.AngularVelocity.z += angularImpulseB.z;

		}
	}

	for (size_t itr = 0; itr < RESOLUTION_ITERATIONS; itr++)
	{
		//Solving frictional constraints first.
		for (int i = 0; i < constraints.size(); i++)
		{
			Constraint& constraint = constraints[i];
			const Jacobian& jacobian = jacobians[i];
			ConstrainedContact& contact = contacts[jacobian.Contact];

			if (jacobian.Type == JACOBIAN_TYPE::FRICTION)
				continue;

			float velocityError = DetermineVelocityConstraint(contacts[jacobian.Contact],jacobian.Vector);

			float& impulseSum = constraint.ImpulseSum;
			float oldImpulse = constraint.ImpulseSum;

			float impulse = -constraint.EffectiveMass * velocityError;
			//impulseSum = std::clamp(impulseSum + impulse, constraint.ImpulseBounds.x, constraint.ImpulseBounds.y);
			impulse = impulseSum - oldImpulse;


			//V0 + delta V
			Rigidbody& bodyA = *contact.BodyA;
			glm::vec3 linearImpulseA = jacobian.Vector.v[0] * impulse * inverseMassMatrix[jacobian.Contact][0].inverseContactMass;
			glm::vec3 angularImpulseA = inverseMassMatrix[jacobian.Contact][0].iTensor * jacobian.Vector.v[1] * impulse;
			bodyA.LinearVelocity.x += linearImpulseA.x;
			bodyA.LinearVelocity.y += linearImpulseA.y;
			bodyA.LinearVelocity.z += linearImpulseA.z;
			bodyA.AngularVelocity.x += angularImpulseA.x;
			bodyA.AngularVelocity.y += angularImpulseA.y;
			bodyA.AngularVelocity.z += angularImpulseA.z;

			//V1 + delta V
			Rigidbody& bodyB = *contact.BodyB;
			glm::vec3 linearImpulseB = jacobian.Vector.v[2] * impulse * inverseMassMatrix[jacobian.Contact][1].inverseContactMass;
			glm::vec3 angularImpulseB = inverseMassMatrix[jacobian.Contact][1].iTensor * jacobian.Vector.v[3] * impulse;
			bodyB.LinearVelocity.x += linearImpulseB.x;
			bodyB.LinearVelocity.y += linearImpulseB.y;
			bodyB.LinearVelocity.z += linearImpulseB.z;
			bodyB.AngularVelocity.x += angularImpulseB.x;
			bodyB.AngularVelocity.y += angularImpulseB.y;
			bodyB.AngularVelocity.z += angularImpulseB.z;
		}
	}
}

void CollisionResolver::ResolvePositionConstraint(const double& deltaTime, const std::vector<Jacobian>& jacobians, const InverseMassMatrix& inverseMassMatrix, std::vector<Constraint>& constraints, std::vector<ConstrainedContact>& contacts)
{
	for (size_t itr = 0; itr < RESOLUTION_ITERATIONS; itr++)
	{
		//Solving frictional constraints first.
		for (int i = 0; i < constraints.size(); i++)
		{
			Constraint& constraint = constraints[i];
			const Jacobian& jacobian = jacobians[i];
			ConstrainedContact& contact = contacts[jacobian.Contact];

			//we do NOT need friction for our positoin
			if (jacobian.Type == JACOBIAN_TYPE::FRICTION)
				continue;

			//Calculating impulse size.
			float b = BAUMGARTE_STABILISATION_VALUE * std::fminf(0.0f, constraint.PositionalError + ALLOWED_INTERSECTION);
			float impulseScale = -(constraint.EffectiveMass * b);

			//BodyA impulses
			Rigidbody& bodyA = *contact.BodyA;
			glm::vec3 linearImpulse_A = jacobian.Vector.v[0] * impulseScale * inverseMassMatrix[jacobian.Contact][0].inverseContactMass;
			printf("%f %f %f\n", linearImpulse_A.x, linearImpulse_A.y, linearImpulse_A.z);
			glm::vec3 angularImpulse_A = inverseMassMatrix[jacobian.Contact][0].iTensor * jacobian.Vector.v[1] * impulseScale;
			bodyA.Translation.x += linearImpulse_A.x;
			bodyA.Translation.y += linearImpulse_A.y;
			bodyA.Translation.z += linearImpulse_A.z;

			//Integrating angular velocity.
			glm::quat rotation_A = { contact.BodyA->Rotation.x, contact.BodyA->Rotation.y, contact.BodyA->Rotation.z, contact.BodyA->Rotation.w };
			glm::quat angularImpulseQuat_A = glm::quat(angularImpulse_A.x, angularImpulse_A.y, angularImpulse_A.z, 0.0f);
			rotation_A = glm::normalize(rotation_A + (angularImpulseQuat_A * rotation_A * 0.5f * (float)deltaTime));
			contact.BodyA->Rotation = { rotation_A.x, rotation_A.y, rotation_A.z, rotation_A.w };

			//bodyB impulses
			Rigidbody& bodyB = *contact.BodyB;
			glm::vec3 linearImpulse_B = jacobian.Vector.v[2] * impulseScale * inverseMassMatrix[jacobian.Contact][1].inverseContactMass;
			glm::vec3 angularImpulse_B = inverseMassMatrix[jacobian.Contact][1].iTensor * jacobian.Vector.v[3] * impulseScale;
			bodyB.Translation.x += linearImpulse_B.x;
			bodyB.Translation.y += linearImpulse_B.y;
			bodyB.Translation.z += linearImpulse_B.z;

			//Integrating angular velocity.
			glm::quat rotation_B = { contact.BodyB->Rotation.x, contact.BodyB->Rotation.y, contact.BodyB->Rotation.z, contact.BodyB->Rotation.w };
			glm::quat angularImpulseQuat_B = glm::quat(angularImpulse_B.x, angularImpulse_B.y, angularImpulse_B.z, 0.0f);
			rotation_B = glm::normalize(rotation_B + (angularImpulseQuat_B * rotation_B * 0.5f * (float)deltaTime));
			contact.BodyB->Rotation = { rotation_B.x, rotation_B.y, rotation_B.z, rotation_B.w };

			constraint.PositionalError = DetermineVelocityConstraint(contact, jacobian.Vector);
		}
	}
}

void CollisionResolver::SoftResolveCollision(const CollisionManifold& manifold)
{
	for (size_t i = 0; i < manifold.ContactPoints.size(); i++)
	{
		CollisionResolver::PositionalCorrection(manifold.CollisionPair.first->GetRigidbody(), manifold.CollisionPair.second->GetRigidbody(), manifold, manifold.ContactPoints[i]);
	}
	return; 

	/*
	for (size_t i = 0; i < manifold.ContactPoints.size(); i++)
	{
		Rigidbody& objectA = manifold.CollisionPair.first->GetRigidbody();
		Rigidbody& objectB = manifold.CollisionPair.second->GetRigidbody();

		if (objectA.IsStatic && objectB.IsStatic)
			return;

		//Calculate relative directions to hitpoints
		glm::vec3 relativePositionA =
		{
			manifold.ContactPoints[i].HitPoint.x - objectA.Translation.x,
			manifold.ContactPoints[i].HitPoint.y - objectA.Translation.y,
			manifold.ContactPoints[i].HitPoint.z - objectA.Translation.z,
		};

		glm::vec3 relativePositionB =
		{
			manifold.ContactPoints[i].HitPoint.x - objectB.Translation.x,
			manifold.ContactPoints[i].HitPoint.y - objectB.Translation.y,
			manifold.ContactPoints[i].HitPoint.z - objectB.Translation.z,
		};

		//Calculate angular velocity at hitpoint.

		glm::vec3 angularVelocityA = { objectA.AngularVelocity.x, objectA.AngularVelocity.y, objectA.AngularVelocity.z };
		angularVelocityA = glm::cross(angularVelocityA, relativePositionA);

		glm::vec3 angularVelocityB = { objectB.AngularVelocity.x, objectB.AngularVelocity.y, objectB.AngularVelocity.z };
		angularVelocityB = glm::cross(angularVelocityB, relativePositionB);

		//Calculate total velocity at hitpoint.
		glm::vec3 totalVelocityA = { objectA.LinearVelocity.x + angularVelocityA.x, objectA.LinearVelocity.y + angularVelocityA.y, objectA.LinearVelocity.z + angularVelocityA.z };
		glm::vec3 totalVelocityB = { objectB.LinearVelocity.x + angularVelocityA.x, objectB.LinearVelocity.y + angularVelocityA.y, objectB.LinearVelocity.z + angularVelocityA.z };

		glm::vec3 relativeVelocity = totalVelocityB - totalVelocityA;

		glm::vec3 normal = { manifold.Normal.x, manifold.Normal.y, manifold.Normal.z };
		const glm::vec3 relativeNormal = glm::normalize(normal);

		//relative velocity applied in normal.
		float impulseForce = glm::dot(relativeVelocity, relativeNormal);

		if (impulseForce < 0)
		{
			//bodies are seperating, do not need to resolve.
			//return;
		}
		glm::vec3 rpAxN = glm::cross(relativePositionA, relativeNormal);
		glm::vec3 rpBxN = glm::cross(relativePositionB, relativeNormal);

		float inertiaA = glm::dot(rpAxN, objectA.InverseInertiaTensor * rpAxN);
		float inertiaB = glm::dot(rpBxN, objectB.InverseInertiaTensor * rpBxN);

		float totalInvMass = objectA.GetInverseMass() + objectB.GetInverseMass() + inertiaA + inertiaB;

		//float restitution = std::min(objectA.GetRestitutionCoefficient(), objectB.GetRestitutionCoefficient());
		float restitution = 1.0f;

		//Calculate normal impulse.
		float jn = -(1.0f + restitution) * impulseForce;
		jn /= totalInvMass;
		jn /= (int)manifold.ContactPoints.size();
		jn /= RESOLUTION_ITERATIONS;
		glm::vec3 impulse = relativeNormal * jn;

		//Calculate ratios to split total impulse between both objects.
		float ratioA = -1.0f * ((objectA.GetInverseMass() + inertiaA) / totalInvMass);
		//Convert to relative to CoM.
		glm::vec3 force = impulse * ratioA;
		objectA.AddForce({ force.x, force.y, force.z });
		glm::vec3 tor = glm::cross(relativePositionA, force);
		objectA.AddTorque({ tor.x, tor.y, tor.z	});


		float ratioB = +1.0f * ((objectB.GetInverseMass() + inertiaB) / totalInvMass);
		force = impulse * ratioB;
		objectB.AddForce({ force.x, force.y, force.z });
		tor = glm::cross(relativePositionB, force);
		objectB.AddTorque({tor.x, tor.y, tor.z});

		//Apply immediate positional correction.
		CollisionResolver::PositionalCorrection(objectA, objectB, manifold, manifold.ContactPoints[i]);
	}
	*/
}

//Position correction - Dividing inv mass of object by total mass to properly scale each way.
//A heavier object will move less than a lighter object.
void CollisionResolver::PositionalCorrection(Rigidbody& objectA, Rigidbody& objectB, const CollisionManifold& manifold, const Contact& contact)
{
	constexpr float CORRECTION_PERCENTAGE = 0.02f;
	constexpr float PENETRATION_SLOP = 0.02f;

	glm::vec3 normal = { manifold.Normal.x, manifold.Normal.y, manifold.Normal.z };
	const glm::vec3 relativeNormal = glm::normalize(-normal);

	float massSum = (objectA.GetInverseMass() + objectB.GetInverseMass());
	if (massSum == 0.0f)
		massSum = 1.0f;

	float numerator = std::max(fabsf(contact.Depth) + PENETRATION_SLOP, 0.0f);
	glm::vec3 correction = numerator / massSum * CORRECTION_PERCENTAGE * relativeNormal;
	correction /= (float)manifold.ContactPoints.size();

	objectA.Translation.x += (-1.0f * (objectA.GetInverseMass() * correction.x));
	objectA.Translation.y += (-1.0f * (objectA.GetInverseMass() * correction.y));
	objectA.Translation.z += (-1.0f * (objectA.GetInverseMass() * correction.z));

	objectB.Translation.x += (+1.0f * (objectB.GetInverseMass() * correction.x));
	objectB.Translation.y += (+1.0f * (objectB.GetInverseMass() * correction.y));
	objectB.Translation.z += (+1.0f * (objectB.GetInverseMass() * correction.z));
}