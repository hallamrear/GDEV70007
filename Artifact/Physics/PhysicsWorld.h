#pragma once
#include <Rendering/IMGUIRenderable.h>
#include <Physics/Structures.h>
#include <Physics/Resolution/CollisionResolver.h>
#include <Physics/Resolution/ContraintContact.h>
#include <Physics/Resolution/Constraint.h>

class Rigidbody;

enum WORLD_EXAMPLE_SCENE : int;

class PhysicsWorld : public IIMGUIRenderable
{
private:
	static constexpr float c_RestSpeed = 0.00000001f;
	static constexpr Vector3 c_Gravity = Vector3(0.0f, -9.81f, 0.0f);
	static constexpr float c_LinearDamping = 0.98f;
	static constexpr float c_AngularDamping = 0.98f;
	static constexpr int c_MaxRigidbodyCount = 1000;

	std::vector<CollisionManifold> m_FrameCollisionManifolds;
	std::vector<ConstrainedContact> m_FrameConstraintPoints;
	std::vector<Jacobian> m_FrameJacobians;
	std::vector<Constraint> m_FrameConstraints;

	Rigidbody* m_RigidbodyList;
	int m_ActiveRigidbodyCount;

	void IntegrateAccelerationAndVelocities();
	void CollectCollisionPairs();
	//Sequential Impulse Constaint Solver
	void SolveConstaints();
	void IntergratePositions();
	void UpdateSleepers();
	void CleanupPhysicsObjects();

	int m_ResolutionType;

	class OctreeNode* m_OctreeRoot;
	class SweepAndPrune* m_SweepAndPrune;
	class SpatialGrid* m_SpatialGrid;

public:
	PhysicsWorld();
	~PhysicsWorld();

	void Initialise(const WORLD_EXAMPLE_SCENE& exampleScene);

	void FixedUpdate();
	void Update(const float& deltaTime);

	Rigidbody& GetFreshRigidbody();

	void AddToBroadPhase(const WORLD_EXAMPLE_SCENE& exampleScene, Entity* entity);

	void Render(Renderer& renderer);

	// Inherited via IIMGUIRenderable
	void OnIMGUIRender() override;
};

