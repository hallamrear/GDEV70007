#include "pch.h"
#include "Camera.h"

Camera::Camera()
{
	DirectX::XMStoreFloat4x4(&m_WorldMatrix, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&m_RotationMatrix, DirectX::XMMatrixIdentity());
	m_RightVector = { 1.0f, 0.0f, 0.0f };
	m_UpVector = { 0.0f, 1.0f, 0.0f };
	m_ForwardVector = { 0.0f, 0.0f, 1.0f };
	m_Translation = { 0.0f, 0.0f, 0.0f };
	UpdateTransformMatrix();
}

Camera::~Camera()
{

}

const Vector3& Camera::GetForwardVector() const
{
	return m_ForwardVector;
}

const Vector3& Camera::GetRightVector() const
{
	return m_RightVector;
}

const Vector3& Camera::GetUpVector() const
{
	return m_UpVector;
}

void Camera::UpdateTransformMatrix()
{
	m_Rotation.x = fmodf(m_Rotation.x, 360.0f);
	m_Rotation.y = fmodf(m_Rotation.y, 360.0f);
	m_Rotation.z = fmodf(m_Rotation.z, 360.0f);

	DirectX::XMStoreFloat4x4(&m_RotationMatrix, DirectX::XMMatrixIdentity() * DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z));
	DirectX::XMStoreFloat4x4(&m_WorldMatrix, DirectX::XMMatrixIdentity() * DirectX::XMLoadFloat4x4(&m_RotationMatrix) * DirectX::XMMatrixTranslation(m_Translation.x, m_Translation.y, m_Translation.z));
	
	DirectX::XMStoreFloat3(&m_RightVector, DirectX::XMVector3Normalize(DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&BASIS_RIGHT_VECTOR), DirectX::XMLoadFloat4x4(&m_RotationMatrix))));
	DirectX::XMStoreFloat3(&m_UpVector, DirectX::XMVector3Normalize(DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&BASIS_UP_VECTOR), DirectX::XMLoadFloat4x4(&m_RotationMatrix))));
	DirectX::XMStoreFloat3(&m_ForwardVector, DirectX::XMVector3Normalize(DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&BASIS_FORWARD_VECTOR), DirectX::XMLoadFloat4x4(&m_RotationMatrix))));
}

void Camera::Move(const Vector3& movement)
{
	m_Translation.x += movement.x;
	m_Translation.y += movement.y;
	m_Translation.z += movement.z;
	UpdateTransformMatrix();
}

void Camera::RotateEulerRadians(const Vector3& rotationEulerRadians)
{
	m_Rotation.x += (RADIANS_TO_DEGREES * rotationEulerRadians.x);
	m_Rotation.y += (RADIANS_TO_DEGREES * rotationEulerRadians.y);
	m_Rotation.z += (RADIANS_TO_DEGREES * rotationEulerRadians.z);
	UpdateTransformMatrix();
}

void Camera::RotateEulerDegrees(const Vector3& rotationEulerDegrees)
{
	m_Rotation.x += (rotationEulerDegrees.x);
	m_Rotation.y += (rotationEulerDegrees.y);
	m_Rotation.z += (rotationEulerDegrees.z);
	UpdateTransformMatrix();
}

void Camera::RotateQuat(const Vector4& rotationQuat)
{
	DirectX::XMStoreFloat4x4(&m_RotationMatrix, DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&m_RotationMatrix), DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotationQuat))));
	UpdateTransformMatrix();
}

const Vector3& Camera::GetPosition() const
{
	return m_Translation;
}

const Matrix4x4 Camera::GetViewMatrix() const
{
	Matrix4x4 view;
	DirectX::XMStoreFloat4x4(&view, DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&m_WorldMatrix))));
	return view;
}