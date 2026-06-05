#include "pch.h"
#include "Jacobian.h"

JacobianVector Jacobian::CalculateNormalJacobian(const glm::vec3& frictionNormal, const glm::vec3& localContactPointA, const glm::vec3& localContactPointB)
{
	JacobianVector vector;
	vector.v[0] = -glm::normalize(frictionNormal);
	vector.v[1] = glm::cross(frictionNormal, localContactPointA); //Torque point of contactA
	vector.v[2] = glm::normalize(frictionNormal);
	vector.v[3] = glm::cross(localContactPointB, frictionNormal); //Torque point of contactB
	return vector;
}

JacobianVector Jacobian::CalculateFrictionalJacobian(const glm::vec3& frictionNormal, const glm::vec3& localContactPointA, const glm::vec3& localContactPointB)
{
	JacobianVector vector;
	vector.v[0] = -glm::normalize(frictionNormal);
	vector.v[1] = glm::cross(frictionNormal, localContactPointA); //Calulate slip direction
	vector.v[2] = glm::normalize(frictionNormal);
	vector.v[3] = glm::cross(localContactPointB, frictionNormal); //Calculate slip direction
	return vector;
}
