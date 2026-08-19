#include <pch.h>
#include <Physics/GJK/GJK.h>
#include <Physics/Colliders/Collider.h>
#include <World/Entity.h>
#include <Physics/EPA/EPA.h>
#include <System/Maths.h>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/norm.hpp>

using namespace glm;

#define MAX_GJK_ITERATIONS 64
#define a 0
#define b 1
#define c 2
#define d 3

bool GJK::Line(Simplex& simplex, vec3& direction)
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

	glm::vec3 A = simplex[a].Diff;
	glm::vec3 B = simplex[b].Diff;

	glm::vec3 AB = B - A;
	glm::vec3 AO = -A;

	if (Maths::SameDirection(AB, AO))
	{
		direction = cross(cross(AB, AO), AB);
	}
	else
	{
		simplex = { simplex[a] };
		direction = AO;
	}

	return false;
}

bool GJK::Triangle(Simplex& simplex, vec3& direction)
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

	vec3 A = simplex[a].Diff;
	vec3 B = simplex[b].Diff;
	vec3 C = simplex[c].Diff;

	vec3 AB = B - A;
	vec3 AC = C - A;
	vec3 AO = -A;

	vec3 ABC = cross(AB, AC);

	if (Maths::SameDirection(cross(ABC, AC), AO))
	{
		if (Maths::SameDirection(AC, AO))
		{
			simplex = { simplex[a], simplex[c] };
			direction = cross(cross(AC, AO), AC);
		}
		else
		{
			
			simplex = { simplex[a], simplex[b] };
			return Line(simplex, direction);
		}
	}
	else
	{
		if (Maths::SameDirection(cross(AB, ABC), AO))
		{
			
			simplex = { simplex[a], simplex[b] };
			return Line(simplex, direction);
		}
		else
		{
			if (Maths::SameDirection(ABC, AO))
			{
				direction = ABC;
			}
			else
			{
				
				simplex = { simplex[a], simplex[c], simplex[b] };
				direction = -ABC;
			}
		}
	}

	return false;
};

bool GJK::Tetrahedron(Simplex& simplex, vec3& direction)
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

	vec3 A = simplex[0].Diff;
	vec3 B = simplex[1].Diff;
	vec3 C = simplex[2].Diff;
	vec3 D = simplex[3].Diff;

	vec3 AB = B - A;
	vec3 AC = C - A;
	vec3 AD = D - A;
	vec3 AO = -A;

	vec3 ABC = cross(AB, AC);
	vec3 ACD = cross(AC, AD);
	vec3 ADB = cross(AD, AB);

	if (Maths::SameDirection(ABC, AO))
	{
		
		simplex = { simplex[a], simplex[b], simplex[c] };
		return Triangle(simplex, direction);
	}

	if (Maths::SameDirection(ACD, AO))
	{
		
		simplex = { simplex[a], simplex[c], simplex[d] };
		return Triangle(simplex, direction);
	}

	if (Maths::SameDirection(ADB, AO))
	{
		
		simplex = { simplex[a], simplex[d], simplex[b] };
		return Triangle(simplex, direction);
	}

	return true;
};

//Determines for a simplex, which part is closest to the origin.
//Similar to determining which voronoi region of each feature is nearest the origin.
bool GJK::UpdateSimplex(Simplex& simplex, vec3& direction)
{
	switch (simplex.size())
	{
	case 2: { return Line(simplex, direction); } break;
	//Simplex& is triangle ABC
	case 3: { return Triangle(simplex, direction); } break;

	//Simplex& is formed of 3 triangles, ABC, ABD, ACD, BCD
	case 4: { return Tetrahedron(simplex, direction); } break;

	default:
		printf("GJK Simplex& updating should not have reached here.\n");
		break;
	}

	return false;
}

SupportVertex GJK::GetSupportVertex(const Collider& colliderA, const Collider& colliderB, const glm::vec3& direction)
{
	SupportVertex support;
	support.SupportVertexA = colliderA.GetSupportPoint(direction);
	support.SupportVertexB = colliderB.GetSupportPoint(-direction);
	support.Diff = support.SupportVertexA - support.SupportVertexB;
	return support;
}

bool GJK::CheckCollision(const Collider& colliderA, const Collider& colliderB, CollisionManifold* manifold)
{
	glm::vec3 AB = 
	{
		colliderB.GetAttachedEntity().GetPosition().x - colliderA.GetAttachedEntity().GetPosition().x,
		colliderB.GetAttachedEntity().GetPosition().y - colliderA.GetAttachedEntity().GetPosition().y,
		colliderB.GetAttachedEntity().GetPosition().z - colliderA.GetAttachedEntity().GetPosition().z
	};

	SupportVertex support = GetSupportVertex(colliderA, colliderB, AB);

	Simplex simplex = { support };	

	vec3 direction = -support.Diff;

	for (size_t i = 0; i < MAX_GJK_ITERATIONS; i++)
	{
		support = GetSupportVertex(colliderA, colliderB, direction);

		if (dot(support.Diff, direction) <= 0.0f)
		{
			return false;
		}

		simplex.push_front(support);

		if (UpdateSimplex(simplex, direction))
		{
			//Origin contained within simplex.
			if (manifold != nullptr)
			{
				//Develop collision details using EPA.
				//EPA::ConstructManifold(colliderA, colliderB, simplex, *manifold);
				//printf("1 %f %f %f - %f\n", manifold->Normal.x, manifold->Normal.y, manifold->Normal.z, manifold->ContactPoints[0].Depth);

				//std::vector<SupportVertex> s = { simplex[0], simplex[1], simplex[2], simplex[3] };
				
				manifold->Reset();

				EPA_Result result = EPA::GetCollisionDetails(simplex, colliderA, colliderB, manifold);
			}

			return true;
		}

	}

	printf("Max GJK iterations reached. Exitting with no collision.\n");
	return false;
}