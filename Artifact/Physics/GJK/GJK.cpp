#include <pch.h>
#include <Physics/GJK/GJK.h>
#include <Physics/Colliders/Collider.h>
#include <World/Entity.h>
#include <Physics/EPA/EPA.h>
#include <System/Maths.h>

using namespace glm;

#define MAX_GJK_ITERATIONS 64
#define a 0
#define b 1
#define c 2
#define d 3

bool GJK::Triangle(Simplex& simplex, int& simplexDimensions, vec3& direction)
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

	const vec3& A = simplex[a].MinkDiff;
	const vec3& B = simplex[b].MinkDiff;
	const vec3& C = simplex[c].MinkDiff;

	//triangles normal
	vec3 normal = cross(B - A, C - A);
	//to origin
	vec3 ao = -A;

	//Closest to edge AB
	if (Maths::SameDirection(cross(B - A, normal), ao))
	{
		simplex[c] = simplex[a];
		direction = cross(cross(B - A, ao), B - A);
		simplexDimensions = 2;
		return false;
	}

	//Closest to edge AC
	if (Maths::SameDirection(cross(normal, C - A), ao))
	{
		simplex[b] = simplex[a];
		direction = cross(cross(C - A, ao), C - A);
		simplexDimensions = 2;
		return false;
	}

	//Check above triangle
	if (Maths::SameDirection(normal, ao))
	{
		simplex[d] = simplex[c];
		simplex[c] = simplex[b];
		simplex[b] = simplex[a];
		direction = normal;
		simplexDimensions = 3;
		return false;
	}

	//else below triangle
	simplexDimensions = 3;
	simplex[d] = simplex[b];
	simplex[b] = simplex[a];
	direction = -normal;
	return false;
};

bool GJK::Tetrahedron(Simplex& simplex, int& simplexDimensions, vec3& direction)
{
	/*
									[A]
								,|,
								,7``\'VA,
							,7`   |, `'VA,
							,7`     `\    `'VA,
						,7`        |,      `'VA,
						,7`          `\         `'VA,
					,7`             |,           `'VA,
					,7`               `\       ,..ooOOTK` [D]
				,7`                  |,.ooOOT''`    AV
				,7`            ,..ooOOT`\`           /7
			,7`      ,..ooOOT''`      |,          AV
			,T,..ooOOT''`              `\         /7
		[B] `'TTs.,                      |,       AV
				`'TTs.,                 `\      /7
					`'TTs.,             |,    AV
							`'TTs.,        `\   /7
								`'TTs.,    |, AV
									`'TTs.,\/7
										`'T`
											[C]
		*/

	const vec3& A = simplex[a].MinkDiff;
	const vec3& B = simplex[b].MinkDiff;
	const vec3& C = simplex[c].MinkDiff;
	const vec3& D = simplex[d].MinkDiff;
		 
	//Construct new edges for top point of tetrahedron
	vec3 e_abc = cross(B-A, C-A);
	vec3 e_acd = cross(C-A, D-A);
	vec3 e_adb = cross(D-A, B-A);
	vec3 ao = -A;

	simplexDimensions = 3;

	//Check each triangle.
	if (Maths::SameDirection(e_abc, ao)) //In front of face ABC
	{
		simplex[d] = simplex[c];
		simplex[c] = simplex[b];
		simplex[b] = simplex[a];
		direction = e_abc;
		return false;
	}

	if (Maths::SameDirection(e_acd, ao)) //In front of face ACD
	{
		simplex[b] = simplex[a];
		direction = e_acd;
		return false;
	}

	if (Maths::SameDirection(e_adb, ao)) //In front of face ADB
	{
		simplex[c] = simplex[d];
		simplex[d] = simplex[b];
		simplex[b] = simplex[a];
		direction = e_adb;
		return false;
	}

	return true;
};

//Determines for a simplex, which part is closest to the origin.
//Similar to determining which voronoi region of each feature is nearest the origin.
bool GJK::UpdateSimplex(Simplex& simplex, int& simplexDimensions, vec3& direction)
{
	switch (simplexDimensions)
	{
	//Simplex& is triangle ABC
	case 3: { return Triangle(simplex, simplexDimensions, direction); } break;

	//Simplex& is formed of 3 triangles, ABC, ABD, ACD, BCD
	case 4: { return Tetrahedron(simplex, simplexDimensions, direction); } break;

	default:
		throw new std::exception("GJK Simplex& updating should not have reached here.\n");
		break;
	}
}

bool GJK::CheckCollision(const Collider& colliderA, const Collider& colliderB, CollisionManifold* manifold)
{
	SupportVertex points[4] = {};
	Simplex simplex = points;

	vec3 direction = { 0.0f, 0.0f, 0.0f };
	direction.x = (colliderA.GetAttachedEntity().GetPosition().x - colliderB.GetAttachedEntity().GetPosition().x);
	direction.y = (colliderA.GetAttachedEntity().GetPosition().y - colliderB.GetAttachedEntity().GetPosition().y);
	direction.z = (colliderA.GetAttachedEntity().GetPosition().z - colliderB.GetAttachedEntity().GetPosition().z);

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

	//Get support vertex in initial direction.
	simplex[c] = SupportVertex::GetSupportVertex(colliderA, colliderB, direction);

	direction = -simplex[c].MinkDiff;

	//Get a new direction in the opposite direction for the first loop.
	simplex[b] = SupportVertex::GetSupportVertex(colliderA, colliderB, direction);

	//returns if the new point is not in front of the search direction
	//this would exit if the direction finds a vertex that was already the furthest one along it.
	float z = Maths::Dot(simplex[b].MinkDiff, direction);
	if (z <= 0)
	{
		return false;
	}

	//Determine perpendicular direction to the line segment with triple cross
	direction = cross(cross(
		simplex[c].MinkDiff - simplex[b].MinkDiff, -simplex[b].MinkDiff),
		simplex[c].MinkDiff - simplex[b].MinkDiff);

	//If origin exists on the segment
	if (fabsf(direction.x) < FLT_EPSILON &&
		fabsf(direction.y) < FLT_EPSILON &&
		fabsf(direction.z) < FLT_EPSILON)
	{
		//Check again against a basis axis
		direction = cross(simplex[c].MinkDiff - simplex[b].MinkDiff, vec3(1.0f, 0.0f, 0.0f));

		//Check again against another basis axis
		if (fabsf(direction.x) < FLT_EPSILON &&
			fabsf(direction.y) < FLT_EPSILON &&
			fabsf(direction.z) < FLT_EPSILON)
		{
			direction = cross(simplex[c].MinkDiff - simplex[b].MinkDiff, vec3(0.0f, 0.0f, -1.0f));
		}
	}

	int simplexDimensions = 2;

	for (size_t itr = 0; itr < MAX_GJK_ITERATIONS; itr++)
	{
		simplex[a] = SupportVertex::GetSupportVertex(colliderA, colliderB, direction);

		//returns if the new point is not in front of the search direction
		//this would exit if the direction finds a vertex that was already the furthest one along it.
		z = Maths::Dot(simplex[a].MinkDiff, direction);
		if (z < 0)
		{
			return false;
		}

		simplexDimensions++;

		//If detection in simplex, return true.
		if (UpdateSimplex(simplex, simplexDimensions, direction))
		{
			//Origin contained within simplex.
			if (manifold != nullptr)
			{
				//Develop collision details using EPA.
				EPA::ConstructManifold(colliderA, colliderB, simplex, *manifold);

				printf("1 %f %f %f - %f\n", manifold->Normal.x, manifold->Normal.y, manifold->Normal.z, manifold->ContactPoints[0].Depth);

				std::vector<SupportVertex> s = { simplex[a], simplex[b], simplex[c], simplex[d] };

				EPA::GetCollisionDetails(s, colliderA, colliderB, manifold);

				printf("2 %f %f %f - %f\n", manifold->Normal.x, manifold->Normal.y, manifold->Normal.z, manifold->ContactPoints[0].Depth);
			}

			return true;
		}

		//if (simplexDimensions == 3)
		//{
		//	Triangle(simplex, simplexDimensions, direction);
		//}
		//else if(Tetrahedron(simplex, simplexDimensions, direction))
		//{
		//	//Origin contained within simplex.
		//	if (manifold != nullptr)
		//	{
		//		//Develop collision details using EPA.
		//		EPA::ConstructManifold(colliderA, colliderB, simplex, *manifold);

		//		printf("1 %f %f %f - %f\n", manifold->Normal.x, manifold->Normal.y, manifold->Normal.z, manifold->ContactPoints[0].Depth);

		//		std::vector<SupportVertex> s = { simplex[a], simplex[b], simplex[c], simplex[d] };

		//		EPA::GetCollisionDetails(s, colliderA, colliderB, manifold);

		//		printf("2 %f %f %f - %f\n", manifold->Normal.x, manifold->Normal.y, manifold->Normal.z, manifold->ContactPoints[0].Depth);
		//	}

		//	return true;
		//}
	}

	return false;
}