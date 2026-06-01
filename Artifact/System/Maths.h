#pragma once
#include <math.h>

#include <System/Maths/Plane.h>

#define DEGREES_TO_RADIANS (float)(M_PI / 180.0f)
#define RADIANS_TO_DEGREES (float)(180.0f / M_PI)

struct Triangle
{
	Vector3 Vertices[3] = { Vector3(0.0f, 0.0f, 0.0f) };
};

// Source - https://stackoverflow.com/a/61862608
// Posted by bolov, modified by community. See post 'Timeline' for change history
// Retrieved 2026-05-30, License - CC BY-SA 4.0
template <class T = void>
struct equal_to {
	constexpr bool operator()(const T& lhs, const T& rhs) const
	{
		return lhs == rhs;
	}
};

inline static bool operator<(const Vector3& lhs, const Vector3& rhs)
{
	if (lhs.x != rhs.x)
		return lhs.x < rhs.x;
	if (lhs.y != rhs.y)
		return lhs.y < rhs.y;
	return lhs.z < rhs.z;
};

inline static bool operator==(const Vector3& lhs, const Vector3& rhs)
{
	if ((lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z))
		return true;
	else
		return false;
}

namespace Maths
{
	inline Vector3 MultiplyScalar(const float& scalar, const Vector3& vector)
	{
		return Vector3(vector.x * scalar, vector.y * scalar, vector.z * scalar);
	};

	inline static Vector3 Add(const Vector3& A, const Vector3& B)
	{
		return Vector3(A.x + B.x, A.y + B.y, A.z + B.z);
	}

	inline static float MagnitudeSqr(const Vector3& vec)
	{
		return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
	}

	inline static float Magnitude(const Vector3& vec)
	{
		float mag = 0.0f;
		DirectX::XMStoreFloat(&mag, DirectX::XMVector3Length(DirectX::XMLoadFloat3(&vec)));
		return mag;
	}

	inline static float Dot(const Vector3& A, const Vector3& B)
	{
		float d = FLT_MAX;
		DirectX::XMStoreFloat(&d, DirectX::XMVector3Dot(XMLoadFloat3(&A), XMLoadFloat3(&B)));
		return d;
	}
	
	inline static bool SameDirection(const Vector3& A, const Vector3& B)
	{
		return Dot(A, B) > 0;
	}

	inline static void Normalise(Vector3& vec)
	{
		DirectX::XMStoreFloat3(&vec, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&vec)));
	}

	inline static Vector3 Normalised(const Vector3& vec)
	{
		Vector3 normal = Vector3(0.0f, 0.0f, 0.0f);
		DirectX::XMStoreFloat3(&normal, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&vec)));
		return normal;
	}

	inline static Vector3 Cross(const Vector3& A, const Vector3& B)
	{
		Vector3 cross = Vector3(0.0f, 0.0f, 0.0f);
		DirectX::XMStoreFloat3(&cross, DirectX::XMVector3Cross(XMLoadFloat3(&A), XMLoadFloat3(&B)));
		return cross;

		//Vector3 cross;
		//cross.x = A.y * B.z - A.z * B.y;
		//cross.y = A.z * B.x - A.x * B.z;
		//cross.z = A.x * B.y - A.y * B.x;
		//return cross;
	}

	inline static Vector3 CrossNormalised(const Vector3& A, const Vector3& B)
	{
		return Normalised(Cross(A, B));
	}

	static inline unsigned int RoundToNearestBaseTwo(const unsigned int& integer)
	{
		unsigned int value = integer;
		value--;
		value |= value >> 1;
		value |= value >> 2;
		value |= value >> 4;
		value |= value >> 8;
		value |= value >> 16;
		value++;
		return value;
	}

	static inline float DistanceFromPlane(const float& planeOffset, const Vector3& direction, const Vector3& point)
	{
		return direction.x * point.x + direction.y * point.y + direction.z * point.z - planeOffset;
	}; 
	
	inline static Vector3 GetNormalOfTriangle(const Vector3& A, const Vector3& B, const Vector3& C)
	{
		Vector3 AB = Vector3(B.x - A.x, B.y - A.y, B.z - A.z);
		Vector3 AC = Vector3(C.x - A.x, C.y - A.y, C.z - A.z);
		return Cross(AB, AC);
	}

	inline static Vector3 GetNormalOfTriangle(const Triangle& triangle)
	{
		return GetNormalOfTriangle(triangle.Vertices[0], triangle.Vertices[1], triangle.Vertices[2]);
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

	inline static Plane GetPlaneFromTriangle(const Triangle& triangle)
	{
		//halved in the example?
		Vector3 normal = GetNormalOfTriangle(triangle);
		normal.x *= 0.5f;
		normal.y *= 0.5f;
		normal.z *= 0.5f;

		Vector3 invertedNormal = normal;
		invertedNormal.x *= -1.0f;
		invertedNormal.y *= -1.0f;
		invertedNormal.z *= -1.0f;
		
		float offset = Dot(invertedNormal, triangle.Vertices[0]);

		Plane plane = Plane(normal, offset);
		return plane;
	}

	//Compute barycentric coordinates for pointOnTri with respect to triangle (a, b, c)
	//Calculation implementation taken from 'Real Time Collision Detection' by Christer Ericson.
	inline static Vector3 CalculateBarycentricPositionOnTriangle(const Vector3& pointOnTri, const Vector3& triA, const Vector3& triB, const Vector3& triC)
	{
		Vector3 uvw = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 v0 = Vector3(triB.x - triA.x, triB.y - triA.y, triB.z - triA.z);
		Vector3 v1 = Vector3(triC.x - triA.x, triC.y - triA.y, triC.z - triA.z);
		Vector3 v2 = Vector3(pointOnTri.x - triA.x, pointOnTri.y - triA.y, pointOnTri.z - triA.z);

		float d00 = Dot(v0, v0);
		float d01 = Dot(v0, v1);
		float d11 = Dot(v1, v1);
		float d20 = Dot(v2, v0);
		float d21 = Dot(v2, v1);

		float denom = d00 * d11 - d01 * d01;
		uvw.y = (d11 * d20 - d01 * d21) / denom;
		uvw.z = (d00 * d21 - d01 * d20) / denom;
		uvw.x = 1.0f - uvw.y - uvw.z;
		return uvw;
	}
};

