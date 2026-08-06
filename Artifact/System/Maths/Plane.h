#pragma once
#include <system/Types.h>

#include <System/Maths.h>
#include <System/Maths/Triangle.h>

//a plane can be defined as
//f[n|d], such that n represents a normal vector to some point p on the plane
//and w represents the point at which the normal values intersect the plane f
struct Plane
{
	Plane() = default;
	//takes in normal vector of plane
	Plane(const Vector3& n, const float d) : x(n.x), y(n.y), z(n.z), w(d)
	{

	};

	//takes in normal components of plane
	Plane(const float nx, const float ny, const float nz, const float d) : x(nx), y(ny), z(nz), w(d)
	{

	};

	//returns the normal vector of the plane
	inline Vector3 Normal()
	{
		return Vector3(x, y, z);
	};

	//sets the plane given a normal and offset from the origin
	void Set(const Vector3& normal, const float& d)
	{
		*reinterpret_cast<Vector3*>(&x) = normal;
		w = d;
	}

	void Set(const float& _x, const float& _y, const float& _z, const float& _d)
	{
		x = _x;
		y = _y;
		z = _z;
		w = _d;
	}

	Vector4 GetVector4()
	{
		return Vector4(x, y, z, w);
	}

	float& operator[](const int& i) { return ((&x)[i]); };
	const float& operator[](const int& i) const { return ((&x)[i]); };

	//normal vector of plane
	float x;
	float y;
	float z;
	//distance from origin, f*O = O
	float w;//also known as d
};

namespace Maths
{
	inline static Plane TransformPlane(const Plane& plane, const Matrix4x4& transformMatrix)
	{
		return Plane
		{
			plane.x * transformMatrix._11 + plane.y * transformMatrix._12 + plane.z * transformMatrix._13,
			plane.x * transformMatrix._21 + plane.y * transformMatrix._22 + plane.z * transformMatrix._23,
			plane.x * transformMatrix._31 + plane.y * transformMatrix._32 + plane.z * transformMatrix._33,
			plane.x * transformMatrix._41 + plane.y * transformMatrix._42 + plane.z * transformMatrix._43 + plane.w
		};


	//	Vector3 O = { plane.x * plane.w, plane.y * plane.w, plane.z * plane.w };
	//	Vector3 N = { plane.x, plane.y, plane.z };

	//	//O = M * O
	//	DirectX::XMStoreFloat3(&O, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&O), DirectX::XMLoadFloat4x4(&transformMatrix)));

	//	//N = transpose(invert(M)) * N
	//	Matrix4x4 invTranspose = Inverse(transformMatrix);
	//	DirectX::XMStoreFloat3(&N, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&N), DirectX::XMLoadFloat4x4(&invTranspose)));

	//	Plane result;
	//	//d = dot(O.xyz, N.xyz)
	//	result.w = Dot(O, N);
	//	Normalise(N);
	//	result.x = N.x;
	//	result.y = N.y;
	//	result.z = N.z;

	//	return result;
	}

	//returns a normalized plane such that ||n||=1
	inline static Plane Normalised(const Plane& plane)
	{
		const auto surfaceNormal{ reinterpret_cast<const Vector3&>(plane.x) };
		const auto iNormalMag{ 1.0f / Maths::Magnitude(surfaceNormal) };

		Plane out = plane;
		out.x *= iNormalMag;
		out.y *= iNormalMag;
		out.z *= iNormalMag;
		out.w *= iNormalMag;
		return out;
	}

	//returns the signed distance orthogonal to the plane from the point
	inline static const float DotPoint(const Plane& f, const Vector3& p)
	{
		return f.x * p.x + f.y * p.y + f.z * p.z + f.w;//*1.0f;
	}

	inline static Plane CreatePlaneFromTriangle(const Triangle& triangle)
	{
		//halved in the example?
		Vector3 normal = GetNormalOfTriangle(triangle);

		Vector3 invertedNormal = normal;
		invertedNormal.x *= -1.0f;
		invertedNormal.y *= -1.0f;
		invertedNormal.z *= -1.0f;

		float offset = Dot(invertedNormal, triangle.Vertices[0]);

		Plane plane = Plane(normal, offset);
		return plane;
	}

	inline static Plane CreatePlaneFromTriangle(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C)
	{
		glm::vec3 AB = B - A;
		glm::vec3 AC = C - A;

		glm::vec3 normal = glm::normalize(glm::cross(AB, AC));
		float distance = -glm::dot(A, normal);

		Plane plane({ normal.x, normal.y, normal.z }, distance);
		return plane;
	}

	inline static Plane CreatePlaneFromTriangle(const Vector3& A, const Vector3& B, const Vector3& C)
	{
		return CreatePlaneFromTriangle(glm::vec3(A.x, A.y, A.z), glm::vec3(B.x, B.y, B.z), glm::vec3(C.x, C.y, C.z));
	}

	inline static float DistanceFromPlane(const Plane& plane, const glm::vec3& point)
	{
		return glm::dot(point, {plane.x, plane.y, plane.z}) + plane.w;
	}

	inline static glm::vec3 ProjectPointOntoPlane(const Plane& plane, const glm::vec3& point)
	{
		float distanceFromPlane = DistanceFromPlane(plane, point);
		return point - (glm::vec3(plane.x, plane.y, plane.z) * distanceFromPlane);
	}
}
