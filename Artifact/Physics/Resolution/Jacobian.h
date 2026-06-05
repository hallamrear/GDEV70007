#pragma once
#include <glm/glm.hpp>

/// <summary>
/// 1x12 Vector that encompasses Ji, vi0, vi1 wi1.
/// Split into 4 vec3's.
/// </summary>
struct JacobianVector
{
	glm::vec3 v[4];
};

enum JACOBIAN_TYPE
{
	NORMAL = 0,
	FRICTION = 1
};

inline static bool operator>(const JACOBIAN_TYPE& lhs, const JACOBIAN_TYPE& rhs)
{
	return (int)lhs > (int)rhs;
}

struct Jacobian
{
	int Contact;
	JACOBIAN_TYPE Type;
	JacobianVector Vector;

	Jacobian(const int& index, const JACOBIAN_TYPE& type, const JacobianVector& vector) : Contact(index), Type(type), Vector(vector) {};

	//Calculating jacobian constraint to solve C * constraint = JV + b = 0;
	static JacobianVector CalculateNormalJacobian(const glm::vec3& frictionNormal, const glm::vec3& localContactPointA, const glm::vec3& localContactPointB);

	//Calculating jacobian constraint to solve C * constraint = JV + b = 0;
	static JacobianVector CalculateFrictionalJacobian(const glm::vec3& frictionNormal, const glm::vec3& localContactPointA, const glm::vec3& localContactPointB);
};	