#pragma once
#include <math.h>

#define DEGREES_TO_RADIANS (float)(M_PI / 180.0f)
#define RADIANS_TO_DEGREES (float)(180.0f / M_PI)

namespace Maths
{
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
		Vector3 normal = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 AB = Vector3(B.x - A.x, B.y - A.y, B.z - A.z);
		Vector3 AC = Vector3(C.x - A.x, C.y - A.y, C.z - A.z);
		DirectX::XMStoreFloat3(&normal, DirectX::XMVector3Normalize(
			DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&AB), DirectX::XMLoadFloat3(&AC))));
		return normal;
	}

	inline static bool SameDirection(const Vector3& A, const Vector3& B)
	{
		Vector3 dot;
		DirectX::XMStoreFloat3(&dot, DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&A), DirectX::XMLoadFloat3(&B)));
		return dot.x > 0;
	};

	inline Vector3 MultiplyScalar(const float& scalar, const Vector3& vector)
	{
		return Vector3(vector.x * scalar, vector.y * scalar, vector.z * scalar);
	}

	//Compute barycentric coordinates for pointOnTri with respect to triangle (a, b, c)
	//Calculation implementation taken from 'Real Time Collision Detection' by Christer Ericson.
	inline static Vector3 CalculateBarycentricPositionOnTriangle(const Vector3& pointOnTri, const Vector3& triA, const Vector3& triB, const Vector3& triC)
	{
		Vector3 uvw = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 v0 = Vector3(triB.x - triA.x, triB.y - triA.y, triB.z - triA.z);
		Vector3 v1 = Vector3(triC.x - triA.x, triC.y - triA.y, triC.z - triA.z);
		Vector3 v2 = Vector3(pointOnTri.x - triA.x, pointOnTri.y - triA.y, pointOnTri.z - triA.z);

		float d00;
		DirectX::XMStoreFloat(&d00, DirectX::XMVector3Dot(XMLoadFloat3(&v0), XMLoadFloat3(&v0)));
		float d01;
		DirectX::XMStoreFloat(&d01, DirectX::XMVector3Dot(XMLoadFloat3(&v0), XMLoadFloat3(&v1)));
		float d11;
		DirectX::XMStoreFloat(&d11, DirectX::XMVector3Dot(XMLoadFloat3(&v1), XMLoadFloat3(&v1)));
		float d20;
		DirectX::XMStoreFloat(&d20, DirectX::XMVector3Dot(XMLoadFloat3(&v2), XMLoadFloat3(&v0)));
		float d21;
		DirectX::XMStoreFloat(&d21, DirectX::XMVector3Dot(XMLoadFloat3(&v2), XMLoadFloat3(&v1)));

		float denom = d00 * d11 - d01 * d01;
		uvw.y = (d11 * d20 - d01 * d21) / denom;
		uvw.z = (d00 * d21 - d01 * d20) / denom;
		uvw.x = 1.0f - uvw.y - uvw.z;
		return uvw;
	}

	inline static float Dot(const Vector3& A, const Vector3& B)
	{
		float d = FLT_MAX;
		DirectX::XMStoreFloat(&d, DirectX::XMVector3Dot(XMLoadFloat3(&A), XMLoadFloat3(&B)));
		return d;
	}

	inline static Vector3 Cross(const Vector3& A, const Vector3& B)
	{
		Vector3 cross = Vector3(0.0f, 0.0f, 0.0f);
		DirectX::XMStoreFloat3(&cross, DirectX::XMVector3Cross(XMLoadFloat3(&A), XMLoadFloat3(&B)));
		return cross;
	}
};

