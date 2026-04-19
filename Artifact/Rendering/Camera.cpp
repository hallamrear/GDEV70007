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

	printf("Up: %f %f %f\n", m_UpVector.x, m_UpVector.y, m_UpVector.z);
	printf("Left: %f %f %f\n", m_RightVector.x, m_RightVector.y, m_RightVector.z);
	printf("Forward: % f % f % f\n", m_ForwardVector.x, m_ForwardVector.y, m_ForwardVector.z);
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

//bool Camera::HandleEvent(const float& deltaTime, const Event& event)
//{
//	switch (event.GetType())
//	{
//	case CAMERA_EVENT:
//
//		break;
//
//	case INPUT_EVENT:
//	{
//		float step = (5000.0f * deltaTime);
//		float rotation = 200.0f * deltaTime;
//
//		switch (event.Data.Input.Key)
//		{
//			/* E Key */
//		case 0x45: RotateEulerDegrees({ 0.0f, +1.0F * rotation, 0.0f }); break;
//			/* Q Key */
//		case 0x51: RotateEulerDegrees({ 0.0f, -1.0F * rotation, 0.0f }); break;
//
//			/* R Key */
//		case 0x52: RotateEulerDegrees({ -1.0F * rotation, 0.0f, 0.0f }); break;
//			/* F Key */
//		case 0x46: RotateEulerDegrees({ +1.0F * rotation, 0.0f, 0.0f }); break;
//
//			/* Up Arrow */
//		case VK_UP: Move({ m_ForwardVector.x * step, m_ForwardVector.y * step, m_ForwardVector.z * step }); break;
//			/* Down Arrow */
//		case VK_DOWN:  Move({ m_ForwardVector.x * -step, m_ForwardVector.y * -step, m_ForwardVector.z * -step }); break;
//			/* Left Arrow */
//		case VK_LEFT:  Move({ m_RightVector.x * -step, m_RightVector.y * -step, m_RightVector.z * -step }); break;
//			/* Right Arrow */
//		case VK_RIGHT: Move({ m_RightVector.x * step, m_RightVector.y * step, m_RightVector.z * step }); break;
//			/* Space Bar */
//		case VK_SPACE: Move({ m_UpVector.x * step, m_UpVector.y * step, m_UpVector.z * step }); break;
//			/* Left Shift */
//		case VK_LSHIFT: Move({ m_UpVector.x * -step, m_UpVector.y * -step, m_UpVector.z * -step }); break;
//
//		default:
//			break;
//		}
//	}
//		break;
//
//	default:
//		return false;
//	}
//
//	return true;
//}

const Matrix4x4 Camera::GetViewMatrix() const
{
	Matrix4x4 view;
	DirectX::XMStoreFloat4x4(&view, DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&m_WorldMatrix))));
	return view;
}