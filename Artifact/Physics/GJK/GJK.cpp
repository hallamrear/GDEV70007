#include <pch.h>
#include <Physics/GJK/GJK.h>
#include <Physics/Colliders/Collider.h>
#include <World/Entity.h>
#include <Physics/EPA/EPA.h>
#include <System/Maths.h>

using namespace DirectX;

//bool Collision::CheckCentroidsInOtherCollider(const Collider& testCollider, const Collider& otherCollider, CollisionDetails* manifold)
//{
//	constexpr size_t centroidCount = 6;
//	std::array<Vector3, centroidCount> centroids = { Vector3(0.0f) };
//	const Vector3& extents = testCollider.GetExtents() * 0.5f;
//	const Vector3& position = testCollider.GetAttachedEntity().GetPosition();// +testCollider.GetOffset();
//
//	switch (testCollider.GetType())
//	{
//	case COLLIDER_TYPE::COLLIDER_TYPE_AABB:
//	{
//		centroids[0] = position + (extents * BASIS_LEFT_VECTOR);
//		centroids[1] = position + (extents * BASIS_UP_VECTOR);
//		centroids[2] = position + (extents * BASIS_FORWARD_VECTOR);
//		centroids[3] = position + (extents * BASIS_RIGHT_VECTOR);
//		centroids[4] = position + (extents * BASIS_DOWN_VECTOR);
//		centroids[5] = position + (extents * BASIS_BACK_VECTOR);
//	}
//	break;
//
//	case COLLIDER_TYPE::COLLIDER_TYPE_OBB:
//	{
//		const Transform& transform = testCollider.GetTransform();
//		centroids[0] = position + (extents * transform.GetLeftVector());
//		centroids[1] = position + (extents * transform.GetUpVector());
//		centroids[2] = position + (extents * transform.GetForwardVector());
//		centroids[3] = position + (extents * transform.GetLeftVector() * -1.0f);
//		centroids[4] = position + (extents * transform.GetUpVector() * -1.0f);
//		centroids[5] = position + (extents * transform.GetForwardVector() * -1.0f);
//	}
//	break;
//
//	case COLLIDER_TYPE::COLLIDER_TYPE_SPHERE:
//	{
//		float radius = extents.x;
//		centroids[0] = position + (radius * BENNETT_LEFT_VECTOR);
//		centroids[1] = position + (radius * BENNETT_UP_VECTOR);
//		centroids[2] = position + (radius * BENNETT_FORWARD_VECTOR);
//		centroids[3] = position + (radius * BENNETT_RIGHT_VECTOR);
//		centroids[4] = position + (radius * BENNETT_DOWN_VECTOR);
//		centroids[5] = position + (radius * BENNETT_BACK_VECTOR);
//	}
//	break;
//
//	default:
//		break;
//	}
//
//	for (size_t i = 0; i < centroidCount; i++)
//	{
//		if (Collision::CheckCollision(centroids[i], otherCollider))
//		{
//			if (manifold != nullptr)
//			{
//				Vector3 testColliderPos = testCollider.GetAttachedEntity().GetPosition(); // +testCollider.GetOffset();
//				Vector3 otherPos = otherCollider.GetAttachedEntity().GetPosition(); //+ otherCollider.GetOffset();
//				Vector3 tcC = centroids[i] - testColliderPos;
//				Vector3 n = -glm::normalize(tcC);
//				Vector3 otherCentroid = otherPos + (otherCollider.GetExtents() * -n);
//
//				manifold->ContactPoints[0].Subject = &testCollider;
//				manifold->ContactPoints[0].Normal = n;
//				manifold->ContactPoints[0].HitPoint = centroids[i];
//				manifold->Count++;
//
//				float t = 0;
//				Ray ray = Ray(otherPos, n);
// 				if (Collision::RayPlaneIntersection(ray, otherCentroid, -n, t))
//				{
//					Vector3 endPoint = ray.GetStart() + (ray.GetDirection() * t);
//					manifold->ContactPoints[1].Subject = &otherCollider;
//					manifold->ContactPoints[1].Normal = -n;
//					manifold->ContactPoints[1].HitPoint = endPoint;
//					manifold->Count++;
//				}
//
//				manifold->Depth = 1.0f;
//			}
//			return true;
//		}
//	}
//
//	return false;
//};
//	
bool GJK::Line(Simplex& simplex, Vector3& direction)
{
	//	far	B  /
	//		  /		
	//		B/
	//		 \  between AB
	//		  \
	//		   \	/
	//			\  /	far A
	//			 A/
	//

	//For a line, does the origin fall the far side of A or B or somewhere in AB
	//However, it cant be in the direction of AB (far side of B), as adding A (B->A is already in the direction of the origin)
	//means that going past B from A is going further from the origin.
	SupportVertex a = simplex[1];
	SupportVertex b = simplex[0];

	Vector3 ab = Vector3(b.MinkowskiDifference.x - a.MinkowskiDifference.x, b.MinkowskiDifference.y - a.MinkowskiDifference.y, b.MinkowskiDifference.z - a.MinkowskiDifference.z);
	Vector3 ao = Vector3(a.MinkowskiDifference.x * -1.0f, a.MinkowskiDifference.y * -1.0f, a.MinkowskiDifference.z * -1.0f);

	//There only needs to be a test for which side of A that the origin is on.
	if (Maths::SameDirection(ab, ao))
	{
		//If AB is in the same direction, the origin is not behind A.
		//Determine new direction

		Vector3 c_ab;
		XMStoreFloat3(&c_ab, XMVector3Cross(XMLoadFloat3(&ab), XMLoadFloat3(&ao)));
		XMStoreFloat3(&direction, XMVector3Cross(XMLoadFloat3(&c_ab), XMLoadFloat3(&ab)));
	}
	else
	{
		//Otherwise, A must be the closest.
		simplex.clear();
		simplex = { a };
		direction = ao;
	}

	return false;
};

bool GJK::Triangle(Simplex& simplex, Vector3& direction)
{
	/*
	////////////////////////////////////////////////@@#####################
	//////////////////////////////////////////////////@@####   ############
	////////////////////////////////////////////////////@%## A ############
	//////////////////////////////////////////////////////@################
	///////////////////////////////////////////////////////@@//////////////
	///////////////////////////////////////////////////@@&X@@//////////////
	@@/////////////////////////////////////////////@@@XXXXX@@//////////////
	##@////////////////////////////////////////@@@XXXXXXXXX@@//////////////
	###&@//////////////////////////////////@@&XXXXXXXXXXXXX%@//////////////
	#####@@////////////////////////////@@@XXXXXXXXXXXXXXXXX%@//////////////
	#######@#//////////////////////@@&XXXXXXXXXXXXXXXXXXXXX%@//////////////
	########%@/////////////////@@%XXXXXXXXXXXXXXXXXXXXXXXXX%@//////////////
	##########@@///////////@@@XXXXXXXXXX            XXXXXXX%@/////      ///
	########   #@&/////@@&XXXXXXXXXXXXXX     abc    XXXXXXX%@///// e_ac ///
	######## B ###@@@%XXXXXXXXXXXXXXXXXX            XXXXXXXX@/////      ///
	########   ##@%/&@@%XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX@//////////////
	###########&@////////X@@@XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX@//////////////
	##########@&///////////////@@@&XXXXXXXXXXXXXXXXXXXXXXXXX@//////////////
	########%@//////////////////////&@@@XXXXXXXXXXXXXXXXXXXX@//////////////
	#######@@/////////////////////////////@@@XXXXXXXXXXXXXXX@//////////////
	######@////////////////////////////////////(@@@XXXXXXXXX@//////////////
	####@@/////////////////////////////////////////@@@&XXXXX@//////////////
	##%@////////////////////      ////////////////////%@@@@X@@/////////////
	#@@///////////////////// e_bc ////////////////////////@@@##############
	@///////////////////////      ////////////////////////@###   ##########
	////////////////////////////////////////////////////@@#### C ##########
	///////////////////////////////////////////////////@######   ##########
	/////////////////////////////////////////////////@@####################
	////////////////////////////////////////////////@######################
	*/

	//Could be nearest to 8 possible spaces,
	//	- 3 Points (a, b, c)
	//	- 3 Edges (ab, ac, bc)
	//	- Above/Below triangle (abc) itself.

	//Can early reject a few though as it wouldve been implicitly rejected in the line function as we wouldve tested that direction already.
	//This removes B, C and edge BC.

	const SupportVertex& a = simplex[2];
	const SupportVertex& b = simplex[1];
	const SupportVertex& c = simplex[0];

	Vector3 ab, ac, ao;
	XMStoreFloat3(&ab, XMVectorSubtract(XMLoadFloat3(&b.MinkowskiDifference), XMLoadFloat3(&a.MinkowskiDifference)));
	XMStoreFloat3(&ac, XMVectorSubtract(XMLoadFloat3(&c.MinkowskiDifference), XMLoadFloat3(&a.MinkowskiDifference)));
	XMStoreFloat3(&ao, XMVectorScale(XMLoadFloat3(&a.MinkowskiDifference), -1.0f));

	//Direction of triangle
	Vector3 abc;
	XMStoreFloat3(&abc, XMVector3Cross(XMLoadFloat3(&ab), XMLoadFloat3(&ac)));
	//Direction of face edge AB
	Vector3 e_ab;
	XMStoreFloat3(&e_ab, XMVector3Cross(XMLoadFloat3(&ab), XMLoadFloat3(&abc)));
	//Direction of face edge AC
	Vector3 e_ac;
	XMStoreFloat3(&e_ac, XMVector3Cross(XMLoadFloat3(&abc), XMLoadFloat3(&ac)));

	if (Maths::SameDirection(e_ac, ao))
	{
		if (Maths::SameDirection(ac, ao))
		{
			simplex.clear();
			simplex = { a, c };

			Vector3 c_ac;
			XMStoreFloat3(&c_ac, XMVector3Cross(XMLoadFloat3(&ac), XMLoadFloat3(&ao)));
			XMStoreFloat3(&direction, XMVector3Cross(XMLoadFloat3(&c_ac), XMLoadFloat3(&ac)));
		}
		else
		{
			simplex.clear();
			simplex = { a, b };
			return false;
		}
	}
	else
	{
		if (Maths::SameDirection(e_ab, ao))
		{
			simplex.clear();
			simplex = { a, b };
			return false;
		}
		else
		{
			//Is the origin above the triangle?
			if (Maths::SameDirection(abc, ao))
			{
				simplex.clear();
				simplex = { a, b, c };
				direction = abc;
			}
			else
			{
				simplex.clear();
				simplex = { a, c, b };
				direction = Vector3(-abc.x, -abc.y, -abc.z);
			}
		}
	}

	return false;
};

bool GJK::Tetrahedron(Simplex& simplex, Vector3& direction)
{
	/*
									[D]
								,|,
								,7``\'VA,
							,7`   |, `'VA,
							,7`     `\    `'VA,
						,7`        |,      `'VA,
						,7`          `\         `'VA,
					,7`             |,           `'VA,
					,7`               `\       ,..ooOOTK` [C]
				,7`                  |,.ooOOT''`    AV
				,7`            ,..ooOOT`\`           /7
			,7`      ,..ooOOT''`      |,          AV
			,T,..ooOOT''`              `\         /7
		[A] `'TTs.,                      |,       AV
				`'TTs.,                 `\      /7
					`'TTs.,             |,    AV
							`'TTs.,        `\   /7
								`'TTs.,    |, AV
									`'TTs.,\/7
										`'T`
											[B]
		*/

	const SupportVertex& a = simplex[3];
	const SupportVertex& b = simplex[2];
	const SupportVertex& c = simplex[1];
	const SupportVertex& d = simplex[0];
		 
	Vector3 ab, ac, ad, ao;
	XMStoreFloat3(&ab, XMVectorSubtract(XMLoadFloat3(&b.MinkowskiDifference), XMLoadFloat3(&a.MinkowskiDifference)));
	XMStoreFloat3(&ac, XMVectorSubtract(XMLoadFloat3(&c.MinkowskiDifference), XMLoadFloat3(&a.MinkowskiDifference)));
	XMStoreFloat3(&ad, XMVectorSubtract(XMLoadFloat3(&d.MinkowskiDifference), XMLoadFloat3(&a.MinkowskiDifference)));
	XMStoreFloat3(&ao, XMVectorScale(XMLoadFloat3(&a.MinkowskiDifference), -1.0f));
		 
	//Construct new edges for top point of tetrahedron
	Vector3 e_abc, e_acd, e_adb;
	XMStoreFloat3(&e_abc, XMVector3Cross(XMLoadFloat3(&ab), XMLoadFloat3(&ac)));
	XMStoreFloat3(&e_acd, XMVector3Cross(XMLoadFloat3(&ac), XMLoadFloat3(&ad)));
	XMStoreFloat3(&e_adb, XMVector3Cross(XMLoadFloat3(&ad), XMLoadFloat3(&ab)));

	//Check each triangle.
	if (Maths::SameDirection(e_abc, ao))
	{
		simplex.clear();
		simplex = { a, b, c };
		direction = e_abc;
		return false;
	}

	if (Maths::SameDirection(e_acd, ao))
	{
		simplex.clear();
		simplex = { a, c, d };
		direction = e_acd;
		return false;
	}

	if (Maths::SameDirection(e_adb, ao))
	{
		simplex.clear();
		simplex = { a, d, b };
		direction = e_adb;
		return false;
	}

	return true;
};
	
//Determines for a simplex, which part is closest to the origin.
//Similar to determining which voronoi region of each feature is nearest the origin.
bool GJK::UpdateSimplex(Simplex& simplex, Vector3& direction)
{
	switch (simplex.size())
	{
		//Simplex is line AB
	case 2: { return Line(simplex, direction); } break;

		//Simplex is triangle ABC
	case 3: { return Triangle(simplex, direction); } break;

		//Simplex is formed of 3 triangles, ABC, ABD, ACD, BCD
	case 4: { return Tetrahedron(simplex, direction); } break;

	default:
		throw new std::exception("GJK Simplex updating should not have reached here.\n");
		break;
	}
}

bool GJK::CheckCollision(const Collider& colliderA, const Collider& colliderB, CollisionManifold* manifold)
{
	Vector3 crossA = Vector3(0.0f, 0.0f, 0.0f);
	//crossA = (colliderB.GetAttachedEntity().GetPosition() /*+ colliderB.GetOffset()*/) - (colliderA.GetTransform().GetPosition() /*+ colliderA.GetOffset()*/) + Vector3(FLT_EPSILON);
	crossA.x = (colliderB.GetAttachedEntity().GetPosition().x - colliderA.GetAttachedEntity().GetPosition().x) + FLT_EPSILON;
	crossA.y = (colliderB.GetAttachedEntity().GetPosition().y - colliderA.GetAttachedEntity().GetPosition().y) + FLT_EPSILON;
	crossA.z = (colliderB.GetAttachedEntity().GetPosition().z - colliderA.GetAttachedEntity().GetPosition().z) + FLT_EPSILON;

	Vector3 up = Vector3(FLT_EPSILON, 1.0f + FLT_EPSILON, FLT_EPSILON);
	Vector3 crossB;
	XMStoreFloat3(&crossB, XMVector3Normalize(XMLoadFloat3(&up)));
	Vector3 direction;
	XMStoreFloat3(&direction, XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&crossA), XMLoadFloat3(&crossB))));

	//Get support vertex in initial direction.
	XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&direction)));
	SupportVertex support = SupportVertex::GetSupportVertex(colliderA, colliderB, direction);

	Simplex simplex = Simplex();
	simplex = { support };

	//Get a new direction in the opposite direction for the first loop.
	Vector3 oppositeDirection = Vector3(support.MinkowskiDifference.x * -1.0f, support.MinkowskiDifference.y * -1.0f, support.MinkowskiDifference.z * -1.0f);
	XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&support.MinkowskiDifference)));

	while (true)
	{
		if (abs(direction.x) < FLT_EPSILON && abs(direction.y) < FLT_EPSILON && abs(direction.z) < FLT_EPSILON)
		{
			up = Vector3(FLT_EPSILON, 1.0f + FLT_EPSILON, FLT_EPSILON);
			XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&up)));
		}

		XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&direction)));

		support = SupportVertex::GetSupportVertex(colliderA, colliderB, direction);

		//returns if the new point is not in front of the search direction
		//this would exit if the direction finds a vertex that was already the furthest one along it.

		Vector3 dot;
		XMStoreFloat3(&dot, XMVector3Dot(XMLoadFloat3(&support.MinkowskiDifference), XMLoadFloat3(&direction)));
		if (dot.x <= 0)
		{
			return false;
		}

		//Add new support vertex to simplex.
		simplex.insert(simplex.begin(), support);

		//If detection in simplex, return true.
		if (UpdateSimplex(simplex, direction))
		{
			//Origin contained within simplex.
			if (manifold != nullptr)
			{
				//Develop collision details using EPA.
				EPA_Result CSO = EPA::GetCollisionDetails(simplex, colliderA, colliderB, manifold);

				////This is our closest point to the origin on the CSO’s boundary.
				Vector3 PointOnTri = Vector3(CSO.Tri.Normal.x * manifold->Depth, CSO.Tri.Normal.y * manifold->Depth, CSO.Tri.Normal.z * manifold->Depth);
				//
				//Compute the barycentric coordinates of this closest point in the CSO triangle.
				Vector3 BaryCentPointOnTri = Maths::CalculateBarycentricPositionOnTriangle(PointOnTri, CSO.Tri.Points[0].MinkowskiDifference, CSO.Tri.Points[1].MinkowskiDifference, CSO.Tri.Points[2].MinkowskiDifference);
				
				//Linearly combining the original points with the same barycentric coordinates as coefficients
				//Ap = x Aa + y Ab + z Ac
				//Bp = x Ba + y Bb + z Bc
				Vector3 Aa = Maths::MultiplyScalar(BaryCentPointOnTri.x, CSO.Tri.Points[0].SupportVertexA);
				Vector3 Ab = Maths::MultiplyScalar(BaryCentPointOnTri.y, CSO.Tri.Points[1].SupportVertexA);
				Vector3 Ac = Maths::MultiplyScalar(BaryCentPointOnTri.z, CSO.Tri.Points[2].SupportVertexA);
				Vector3 HitPointA = Vector3(Aa.x + Ab.x + Ac.x, Aa.y + Ab.y + Ac.y, Aa.z + Ab.z + Ac.z);

				Vector3 Ba = Maths::MultiplyScalar(BaryCentPointOnTri.x, CSO.Tri.Points[0].SupportVertexB);
				Vector3 Bb = Maths::MultiplyScalar(BaryCentPointOnTri.y, CSO.Tri.Points[1].SupportVertexB);
				Vector3 Bc = Maths::MultiplyScalar(BaryCentPointOnTri.z, CSO.Tri.Points[2].SupportVertexB);
				Vector3 HitPointB = Vector3(Ba.x + Bb.x + Bc.x, Ba.y + Bb.y + Bc.y, Ba.z + Bb.z + Bc.z);

				//manifold->ContactPoints[0].HitPoint = HitPointA;
				manifold->ContactPoints.push_back(Contact());
				manifold->ContactPoints[0].SubjectA = &colliderA;
				manifold->ContactPoints[0].SubjectB = &colliderB;
				manifold->ContactPoints[0].Normal = CSO.Tri.Normal;

				manifold->ContactPoints.push_back(Contact());
				manifold->ContactPoints[1].SubjectA = &colliderB;
				manifold->ContactPoints[1].SubjectB = &colliderA;
				manifold->ContactPoints[1].Normal = Vector3(-CSO.Tri.Normal.x, -CSO.Tri.Normal.y, -CSO.Tri.Normal.z);
			}
				
			return true;
		}
	}

	return false;
}