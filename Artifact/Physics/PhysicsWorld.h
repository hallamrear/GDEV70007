#pragma once
class Rigidbody;

class PhysicsWorld
{
private:
	static constexpr float c_RestSpeed = 0.00000001f;
	static constexpr Vector3 c_Gravity = Vector3(0.0f, -9.81f, 0.0f);
	static constexpr float c_LinearDamping = 0.98f;
	static constexpr float c_AngularDamping = 0.98f;
	static constexpr int c_MaxRigidbodyCount = 1000;

	Rigidbody* m_RigidbodyList;
	int m_ActiveRigidbodyCount;

	void IntegrateVelocities();
	void CollectCollisionPairs();
	//Sequential Impulse Constaint Solver
	void SolveConstaints();
	void IntergratePositions();
	void UpdateSleepers();
	void CleanupPhysicsObjects();

public:
	PhysicsWorld();
	~PhysicsWorld();

	void FixedUpdate();
	void Update(const float& deltaTime);

	Rigidbody& GetFreshRigidbody();
};

