#include "pch.h"
#include <Physics/SAT/SeparatingAxisTheorem.h>
#include <Physics/Colliders/Collider.h>
#include <Physics/Structures.h>
#include <World/Entity.h>

using namespace Maths;

Interval SeparatingAxisTheorem::GetInterval(const Collider& collider, const Vector3& axis)
{
	std::vector<Vector3> points = std::vector<Vector3>();
	collider.GetPoints(points);

	//Vector3 maxPoint = collider.GetFurthestPointInDirection(axis);
	//Vector3 minPoint = collider.GetFurthestPointInDirection(MultiplyScalar(-1.0f, axis));
	//Interval interval;
	//interval.min = Dot(axis, minPoint);
	//interval.max = Dot(axis, maxPoint);
	//interval.axis = axis;
	//interval.delta = abs(interval.max) - abs(interval.min);


	Interval interval;
	interval.axis = axis;
	size_t pointCount = points.size();
	float dot = 0.0f;
	for (size_t i = 0; i < pointCount; i++)
	{
		dot = Dot(axis, points[i]);
		interval.min = std::min(interval.min, dot);
		interval.max = std::max(interval.max, dot);
	}

	return interval;
}

bool SeparatingAxisTheorem::CheckCollision(const Collider& colliderA, const Collider& colliderB, CollisionManifold* manifold)
{
	UNREFERENCED_PARAMETER(manifold);
	assert(colliderA.GetType() == COLLIDER_TYPE_AABB);
	assert(colliderB.GetType() == COLLIDER_TYPE_AABB);

	Vector3 axisToCheck[15] =
	{
		//Three normals of colliderA (AABB)
		colliderA.GetAttachedEntity().GetForwardVector(),
		colliderA.GetAttachedEntity().GetRightVector(),
		colliderA.GetAttachedEntity().GetUpVector(),
		//Three normals of colliderB (AABB)
		colliderB.GetAttachedEntity().GetForwardVector(),
		colliderB.GetAttachedEntity().GetRightVector(),
		colliderB.GetAttachedEntity().GetUpVector(),
		//Cross Product of 1 of each
		Cross(axisToCheck[0], axisToCheck[3]),
		Cross(axisToCheck[0], axisToCheck[4]),
		Cross(axisToCheck[0], axisToCheck[5]),
		Cross(axisToCheck[1], axisToCheck[3]),
		Cross(axisToCheck[1], axisToCheck[4]),
		Cross(axisToCheck[1], axisToCheck[5]),
		Cross(axisToCheck[2], axisToCheck[3]),
		Cross(axisToCheck[2], axisToCheck[4]),
		Cross(axisToCheck[2], axisToCheck[5]),
	};

	//float minimumOverlap = INFINITY;

	//if occurs on one of the first 6 axis' its an corner face collision.
		//if it happens in the first 3, its a face on colliderA, corner on B.
			//One of the two faces along the normal used for the axis, whichever is closest to the other collider.
			//corner is whichever corner is closest along the axis.

	//if it happens in axis 3-6, its the same but with the colliders reversed.


	//if mimimum overlap occurs on a cross axis, then its an edge edge cllison
	//determine edges as 
		//colliderA one of the edges parallel to the normal used for the cross product, closest one to colliderB. same for b.

	int intervalAxisIndex = -1;
	float maxA = -INFINITY;
	float minA = INFINITY;
	float maxB = -INFINITY;
	float minB = INFINITY;

	for (int i = 0; i < 15; i++)
	{
		if (MagnitudeSqr(axisToCheck[i]) <= FLT_EPSILON)
			continue;

		Vector3 normalisedAxis = Normalised(axisToCheck[i]);
		Interval intervalA = GetInterval(colliderA, normalisedAxis);
		minA = std::min(minA, intervalA.min);
		maxA = std::max(maxA, intervalA.max);

		Interval intervalB = GetInterval(colliderB, normalisedAxis);
		minB = std::min(minB, intervalB.min);
		maxB = std::max(maxB, intervalB.max);

		bool overlap = (minB <= maxA) && (minA <= maxB);

		if (!overlap)
		{
			return false;
		}

		intervalAxisIndex = i;
	}

	assert(intervalAxisIndex != -1);

	//https://www.youtube.com/watch?v=snqmUCu-_4o

	if (manifold != nullptr)
	{
		float depth = minB - maxA;
		Vector3 axis = Normalised(axisToCheck[intervalAxisIndex]);
		manifold->Depth = depth;
		manifold->ContactPoints.resize(2);
		manifold->ContactPoints[0].HitPoint = Add(colliderA.GetAttachedEntity().GetPosition(), MultiplyScalar(depth, axis));
		manifold->ContactPoints[0].Normal = axis;
		manifold->ContactPoints[0].SubjectA = &colliderA;
		manifold->ContactPoints[0].SubjectB = &colliderB;

		manifold->ContactPoints[1].HitPoint = Add(colliderB.GetAttachedEntity().GetPosition(), MultiplyScalar(-depth, axis));
		manifold->ContactPoints[1].Normal = MultiplyScalar(-1.0f, axis);
		manifold->ContactPoints[1].SubjectA = &colliderB;
		manifold->ContactPoints[1].SubjectB = &colliderA;
	}

	return true;
}
