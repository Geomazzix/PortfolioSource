#include "Camera.h"

namespace CRT
{
	Camera::Camera() :
		m_FovInRad(glm::radians<float>(70.0f)),
		m_ViewPortPixelWidth(0),
		m_ViewPortPixelHeight(0),
		m_Type(ECameraType::PERSPECTIVE),
		m_ZNear(0.1f),
		m_ZFar(1.0f),
		m_AspectRatio(1.2f),
		m_CameraSize(10.0f),
		m_Aperture(0.0f)
	{}

	void Camera::Initialize(int pixelWidth, int pixelHeight, ECameraType type, float zNear, float zFar)
	{
		m_ViewPortPixelWidth = pixelWidth;
		m_ViewPortPixelHeight = pixelHeight;
		m_Type = type;
		m_ZNear = zNear;
		m_ZFar = zFar;
		m_AspectRatio = static_cast<float>(pixelWidth) / static_cast<float>(pixelHeight);
	}

	void Camera::SetFieldOfView(float fovInDeg)
	{
		m_FovInRad = glm::radians<float>(fovInDeg);
	}

	float Camera::GetAspectRatio() const
	{
		return m_AspectRatio;
	}

	float Camera::GetFovInRads() const
	{
		return m_FovInRad;
	}

	ECameraType Camera::GetCameraType() const
	{
		return m_Type;
	}

	float Camera::GetZNear() const
	{
		return m_ZNear;
	}

	float Camera::GetZFar() const
	{
		return m_ZFar;
	}

	void Camera::SetAperture(float aperture)
	{
		m_Aperture = aperture;
	}

	float Camera::GetAperture() const
	{
		return m_Aperture;
	}

	void Camera::SetFocalLength(float focalLength)
	{
		m_FocalLength = focalLength;
	}

	float Camera::GetFocalLength() const
	{
		return m_FocalLength;
	}

	void Camera::SetCameraSize(float size)
	{
		m_CameraSize = size;
	}

	float Camera::GetCameraSize() const
	{
		return m_CameraSize;
	}

	int Camera::GetViewPortPixelWidth() const
	{
		return m_ViewPortPixelWidth;
	}

	int Camera::GetViewPortPixelHeight() const
	{
		return m_ViewPortPixelHeight;
	}

	CRT::Transform& Camera::GetTransform()
	{
		return m_Transform;
	}
}