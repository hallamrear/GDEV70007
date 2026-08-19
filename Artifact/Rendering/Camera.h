#pragma once
#include <DirectXMath.h>
#include <Rendering/IMGUIRenderable.h>

class Camera : public IIMGUIRenderable
{
private:
	Vector3 m_Translation;
	Vector3 m_Rotation;
	Matrix4x4 m_RotationMatrix;
	Matrix4x4 m_WorldMatrix;
	void UpdateTransformMatrix();

	Vector3 m_ForwardVector;
	Vector3 m_RightVector;
	Vector3 m_UpVector;

public:
	Camera();
	~Camera();

	const Vector3& GetForwardVector() const;
	const Vector3& GetRightVector() const;
	const Vector3& GetUpVector() const;

	void Move(const Vector3& movement);
	void RotateEulerRadians(const Vector3& rotationEulerRadians);
	void RotateEulerDegrees(const Vector3& rotationEulerDegrees);
	void RotateQuat(const Quaternion& rotationQuat);

	const Vector3& GetPosition() const;
	void SetPosition(const Vector3& position);

	const DirectX::XMFLOAT4X4 GetViewMatrix() const;

	// Inherited via IIMGUIRenderable
	void OnIMGUIRender() override;
};

