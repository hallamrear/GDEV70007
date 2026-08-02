#include <pch.h>
#include <Physics/GJK/GJK.h>
#include <Physics/Colliders/Collider.h>
#include <World/Entity.h>
#include <Physics/EPA/EPA.h>
#include <System/Maths.h>

//bool Collision::CheckCentroidsInOtherCollider(const Collider& testCollider, const Collider& otherCollider, CollisionDetails* manifold)
//{
//	constexpr size_t centroidCount = 6;
//	std::array<glm::vec3, centroidCount> centroids = { glm::vec3(0.0f) };
//	const glm::vec3& extents = testCollider.GetExtents() * 0.5f;
//	const glm::vec3& position = testCollider.GetAttachedEntity().GetPosition();// +testCollider.GetOffset();
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
//				glm::vec3 testColliderPos = testCollider.GetAttachedEntity().GetPosition(); // +testCollider.GetOffset();
//				glm::vec3 otherPos = otherCollider.GetAttachedEntity().GetPosition(); //+ otherCollider.GetOffset();
//				glm::vec3 tcC = centroids[i] - testColliderPos;
//				glm::vec3 n = -glm::normalize(tcC);
//				glm::vec3 otherCentroid = otherPos + (otherCollider.GetExtents() * -n);
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
//					glm::vec3 endPoint = ray.GetStart() + (ray.GetDirection() * t);
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
bool GJK::Line(Simplex& simplex, glm::vec3& direction)
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

	glm::vec3 ab = glm::vec3(b.MinkowskiDifference.x - a.MinkowskiDifference.x, b.MinkowskiDifference.y - a.MinkowskiDifference.y, b.MinkowskiDifference.z - a.MinkowskiDifference.z);
	glm::vec3 ao = glm::vec3(a.MinkowskiDifference.x * -1.0f, a.MinkowskiDifference.y * -1.0f, a.MinkowskiDifference.z * -1.0f);

	//There only needs to be a test for which side of A that the origin is on.
	if (Maths::SameDirection(ab, ao))
	{
		//If AB is in the same direction, the origin is not behind A.
		//Determine new direction

		glm::vec3 c_ab = glm::cross(glm::vec3(ab.x, ab.y, ab.z), glm::vec3(ao.x, ao.y, ao.z));
		glm::vec3 tc_ab = glm::cross(c_ab, glm::vec3(ab.x, ab.y, ab.z));
		direction = { tc_ab.x, tc_ab.y, tc_ab.z };
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

bool GJK::Triangle(Simplex& simplex, glm::vec3& direction)
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

	glm::vec3 ab = { b.MinkowskiDifference.x - a.MinkowskiDifference.x, b.MinkowskiDifference.y - a.MinkowskiDifference.y, b.MinkowskiDifference.z - a.MinkowskiDifference.z };
	glm::vec3 ac = { c.MinkowskiDifference.x - a.MinkowskiDifference.x, c.MinkowskiDifference.y - a.MinkowskiDifference.y, c.MinkowskiDifference.z - a.MinkowskiDifference.z };
	glm::vec3 ao = { -a.MinkowskiDifference.x,-a.MinkowskiDifference.y, -a.MinkowskiDifference.z };

	//Direction of triangle
	glm::vec3 abc = glm::cross(ab, ac);
	//Direction of face edge AB
	glm::vec3 e_ab = glm::cross(ab, abc);
	//Direction of face edge AC
	glm::vec3 e_ac = glm::cross(abc, ac);

	if (Maths::SameDirection(e_ac, ao))
	{
		if (Maths::SameDirection(ac, ao))
		{
			simplex.clear();
			simplex = { a, c };

			glm::vec3 c_ac = glm::cross(ac, ao);
			direction = glm::normalize(glm::cross(c_ac, ac));
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
				direction = glm::vec3(-abc.x, -abc.y, -abc.z);
			}
		}
	}

	return false;
};

bool GJK::Tetrahedron(Simplex& simplex, glm::vec3& direction)
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
		 
	glm::vec3 ab = b.MinkowskiDifference - a.MinkowskiDifference;
	glm::vec3 ac = c.MinkowskiDifference - a.MinkowskiDifference;
	glm::vec3 ad = d.MinkowskiDifference - a.MinkowskiDifference;
	glm::vec3 ao = -a.MinkowskiDifference;
		 
	//Construct new edges for top point of tetrahedron
	glm::vec3 e_abc = glm::cross(ab, ac);
	glm::vec3 e_acd = glm::cross(ac, ad);
	glm::vec3 e_adb = glm::cross(ad, ab);

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
bool GJK::UpdateSimplex(Simplex& simplex, glm::vec3& direction)
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
	glm::vec3 crossA = glm::vec3(0.0f, 0.0f, 0.0f);
	//crossA = (colliderB.GetAttachedEntity().GetPosition() /*+ colliderB.GetOffset()*/) - (colliderA.GetTransform().GetPosition() /*+ colliderA.GetOffset()*/) + glm::vec3(FLT_EPSILON);
	crossA.x = (colliderB.GetAttachedEntity().GetPosition().x - colliderA.GetAttachedEntity().GetPosition().x) + FLT_EPSILON;
	crossA.y = (colliderB.GetAttachedEntity().GetPosition().y - colliderA.GetAttachedEntity().GetPosition().y) + FLT_EPSILON;
	crossA.z = (colliderB.GetAttachedEntity().GetPosition().z - colliderA.GetAttachedEntity().GetPosition().z) + FLT_EPSILON;

	glm::vec3 up = glm::vec3(FLT_EPSILON, 1.0f + FLT_EPSILON, FLT_EPSILON);
	glm::vec3 crossB = glm::normalize(up);
	glm::vec3 direction = glm::normalize(glm::cross(crossA, crossB));

	//Get support vertex in initial direction.
	direction = glm::normalize(direction);
	SupportVertex support = SupportVertex::GetSupportVertex(colliderA, colliderB, direction);

	Simplex simplex = Simplex();
	simplex = { support };

	//Get a new direction in the opposite direction for the first loop.
	glm::vec3 oppositeDirection = glm::vec3(support.MinkowskiDifference.x * -1.0f, support.MinkowskiDifference.y * -1.0f, support.MinkowskiDifference.z * -1.0f);
	direction = glm::normalize(support.MinkowskiDifference);

	while (true)
	{
		if (abs(direction.x) < FLT_EPSILON && abs(direction.y) < FLT_EPSILON && abs(direction.z) < FLT_EPSILON)
		{
			up = glm::vec3(FLT_EPSILON, 1.0f + FLT_EPSILON, FLT_EPSILON);
			direction = glm::normalize(up);
		}

		direction = glm::normalize(direction);

		support = SupportVertex::GetSupportVertex(colliderA, colliderB, direction);

		//returns if the new point is not in front of the search direction
		//this would exit if the direction finds a vertex that was already the furthest one along it.

		float dot = glm::dot(support.MinkowskiDifference, direction);
		if (dot <= 0)
		{
			return false;
		}

		//Add new support vertex to simplex.
		simplex.insert(simplex.begin(), support);

		//If detection in simplex, return true.
		if (UpdateSimplex(simplex, direction))
		{
			UNREFERENCED_PARAMETER(manifold);
			//Origin contained within simplex.
			//if (manifold != nullptr)
			//{
			//	//Develop collision details using EPA.
			//	EPA_Result CSO = EPA::GetCollisionDetails(simplex, colliderA, colliderB, manifold);

			//	////This is our closest point to the origin on the CSO’s boundary.
			//	glm::vec3 PointOnTri = glm::vec3(CSO.Tri.Normal.x * CSO.Depth, CSO.Tri.Normal.y * CSO.Depth, CSO.Tri.Normal.z * CSO.Depth);
			//	//
			//	//Compute the barycentric coordinates of this closest point in the CSO triangle.
			//	glm::vec3 BaryCentPointOnTri = Maths::CalculateBarycentricPositionOnTriangle(PointOnTri, CSO.Tri.Points[0].MinkowskiDifference, CSO.Tri.Points[1].MinkowskiDifference, CSO.Tri.Points[2].MinkowskiDifference);
			//	
			//	//Linearly combining the original points with the same barycentric coordinates as coefficients
			//	//Ap = x Aa + y Ab + z Ac
			//	//Bp = x Ba + y Bb + z Bc
			//	glm::vec3 Aa = BaryCentPointOnTri.x * CSO.Tri.Points[0].SupportVertexA;
			//	glm::vec3 Ab = BaryCentPointOnTri.y * CSO.Tri.Points[1].SupportVertexA;
			//	glm::vec3 Ac = BaryCentPointOnTri.z * CSO.Tri.Points[2].SupportVertexA;
			//	glm::vec3 HitPointA = glm::vec3(Aa.x + Ab.x + Ac.x, Aa.y + Ab.y + Ac.y, Aa.z + Ab.z + Ac.z);

			//	glm::vec3 Ba = BaryCentPointOnTri.x * CSO.Tri.Points[0].SupportVertexB;
			//	glm::vec3 Bb = BaryCentPointOnTri.y * CSO.Tri.Points[1].SupportVertexB;
			//	glm::vec3 Bc = BaryCentPointOnTri.z * CSO.Tri.Points[2].SupportVertexB;
			//	glm::vec3 HitPointB = glm::vec3(Ba.x + Bb.x + Bc.x, Ba.y + Bb.y + Bc.y, Ba.z + Bb.z + Bc.z);

			//	//manifold->CollisionPair = { colliderA, colliderB };
			//	manifold->Normal = CSO.Tri.Normal;

			//	Contact contactA;
			//	contactA.HitPoint = { HitPointA.x, HitPointA.y, HitPointA.z };
			//	contactA.Depth = CSO.Depth;
			//	manifold->ContactPoints.push_back(contactA);
			//}
				
			return true;
		}
	}

	return false;
}