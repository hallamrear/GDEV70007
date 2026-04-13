#pragma once
#include <DirectXMath.h>

class Camera
{
private:
	Vector3 m_Translation;
	Vector3 m_Rotation;
	DirectX::XMFLOAT4X4 m_RotationMatrix;
	DirectX::XMFLOAT4X4 m_WorldMatrix;
	void UpdateTransformMatrix();

	DirectX::XMFLOAT3 m_ForwardVector;
	DirectX::XMFLOAT3 m_RightVector;
	DirectX::XMFLOAT3 m_UpVector;

public:
	Camera();
	~Camera();

	void Move(const Vector3& movement);
	void RotateEulerRadians(const Vector3& rotationEulerRadians);
	void RotateEulerDegrees(const Vector3& rotationEulerDegrees);
	void RotateQuat(const Quaternion& rotationQuat);

	const DirectX::XMFLOAT4X4 GetViewMatrix() const;
};

