#include "pch.h"
#include "ContraintContact.h"
#include <glm/gtx/norm.hpp>

void GramSchmidt(glm::vec3& A, glm::vec3& B, glm::vec3& C)
{
	double a[3][3] = 
	{
	   {A.x, A.y, A.z},
	   {B.x, B.y, B.z},
	   {C.x, C.y, C.z}
	};

	double r[3][3], q[3][3];
	int k, i, j;
	for (k = 0; k < 3; k++)
	{
		r[k][k] = 0; // equivalent to sum = 0
		for (i = 0; i < 3; i++)
		{
			r[k][k] = r[k][k] + a[i][k] * a[i][k]; // rkk = sqr(a0k) + sqr(a1k) + sqr(a2k) 
		}

		r[k][k] = sqrt(r[k][k]);  // ||a||

		for (i = 0; i < 3; i++)
		{
			q[i][k] = a[i][k] / r[k][k];
		}

		for (j = k + 1; j < 3; j++)
		{
			r[k][j] = 0;

			for (i = 0; i < 3; i++)
				r[k][j] += q[i][k] * a[i][j];

			for (i = 0; i < 3; i++)
				a[i][j] = a[i][j] - r[k][j] * q[i][k];
		}
	}

	A.x = (float)a[0][0];
	A.y = (float)a[0][1];
	A.z = (float)a[0][2];
	B.x = (float)a[1][0];
	B.y = (float)a[1][1];
	B.z = (float)a[1][2];
	C.x = (float)a[2][0];
	C.y = (float)a[2][1];
	C.z = (float)a[2][2];
	printf("");
};

glm::vec3 ConstrainedContact::ComputeRelativeVelocity(const ConstrainedContact& contact)
{
	glm::vec3 contactDelta = { contact.ContactNormal * (contact.CollisionDepth * 0.5f) };

	glm::vec3 bodyAContactPointLocal = contact.ContactLocalPositions[0] - contactDelta;
	glm::vec3 bodyBContactPointLocal = contact.ContactLocalPositions[1] + contactDelta;

	glm::vec3 angularVelocityAtPointA = glm::cross(bodyAContactPointLocal, contact.AngularVelocity[0]);
	glm::vec3 angularVelocityAtPointB = glm::cross(bodyBContactPointLocal, contact.AngularVelocity[1]);

	glm::vec3 relativeVelocityA = -(contact.LinearVelocity[0] - angularVelocityAtPointA);
	glm::vec3 relativeVelocityB = contact.LinearVelocity[1] + angularVelocityAtPointB;

	glm::vec3 velocitySum = relativeVelocityA + relativeVelocityB;

	//Velocity reduction if near zero.
	if (std::fabsf(velocitySum.x) < 0.0001f)
		velocitySum.x = 0.0f;

	if (std::fabsf(velocitySum.y) < 0.0001f)
		velocitySum.y = 0.0f;

	if (std::fabsf(velocitySum.z) < 0.0001f)
		velocitySum.z = 0.0f;

	return velocitySum;
}

void ConstrainedContact::ComputeContactOrthoVectors(const glm::vec3& relativeVelocityAtContact, glm::vec3& normal, glm::vec3& binormal, glm::vec3& tangent)
{
	tangent = relativeVelocityAtContact - normal * glm::dot(relativeVelocityAtContact, normal);

	float tangentSqr = glm::length2(tangent);

	if (tangentSqr > 0.001f)
	{
		float mag = (1.0f / std::sqrtf(tangentSqr));

		tangent *= mag;
		binormal = glm::normalize(glm::cross(tangent, normal));

		tangent = glm::normalize(tangent);
		binormal= glm::normalize(binormal);
	}
	else
	{
		//Make our own basis vectors;
		float x = abs(normal.x);
		float y = abs(normal.y);
		float z = abs(normal.z);

		//0 0 1
		if (x > y && x > z)
		{
			tangent = { 0.0f, 1.0f, 0.0f };
			binormal = { 0.0f, 0.0f, 1.0f };
		}
		else if (y > z) //0 1 0
		{
			tangent = { 0.0f, 0.0f, 1.0f };
			binormal = { 1.0f, 0.0f, 0.0f };
		}
		else
		{
			tangent = { 1.0f, 0.0f, 0.0f };
			binormal = { 0.0f, 1.0f, 0.0f };
		}
	}
	
	normal = glm::normalize(normal);
	tangent = glm::normalize(tangent);
	binormal = glm::normalize(binormal);

	GramSchmidt(normal, tangent, binormal);
	//todo : Reorthoganise?
	//GranSchmidt?
}
