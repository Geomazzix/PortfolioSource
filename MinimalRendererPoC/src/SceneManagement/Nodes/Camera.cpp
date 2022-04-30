#include "MrpocPch.h"
#include <SceneManagement/Nodes/Camera.h>
#include "EventSystem/EventMessenger.h"

namespace mrpoc
{
	Camera::Camera() :
		m_screenWidth(0),
		m_screenHeight(0),
		m_fovInDeg(0),
		m_zNear(0),
		m_zFar(0),
		m_AspectRatio(0),
		m_projectionMat(glm::mat4(0.0f)),
		m_isDirty(false)
	{}

	Camera::~Camera()
	{
		EventMessenger::GetInstance().DisconnectListener<ResizeEventArgs&>("OnWindowResize", &Camera::OnScreenResize, this);
	}

	void Camera::Initialize(int screenWidth, int screenHeight, float fovInDeg, float zNear, float zFar)
	{
		m_screenWidth = screenWidth;
		m_screenHeight = screenHeight;

		m_fovInDeg = fovInDeg;
		m_zNear = zNear;
		m_zFar = zFar;

		EventMessenger::GetInstance().ConnectListener<ResizeEventArgs&>("OnWindowResize", &Camera::OnScreenResize, this);

		CalculateProjectionMatrix();
	}

	void Camera::OnScreenResize(ResizeEventArgs& args)
	{
		m_screenWidth = args.Width;
		m_screenHeight = args.Height;
		m_isDirty = true;
	}

	float Camera::GetAspectRatio()
	{
		if (m_isDirty)
			CalculateProjectionMatrix();
		return m_AspectRatio;
	}

	void Camera::SetFOV(float fovInDeg)
	{
		m_fovInDeg = fovInDeg;
		m_isDirty = true;
	}

	float Camera::GetFOVInDeg() const
	{
		return m_fovInDeg;
	}

	void Camera::SetZNear(float zNear)
	{
		m_zNear = zNear;
		m_isDirty = true;
	}

	float Camera::GetZNear() const
	{
		return m_zNear;
	}

	void Camera::SetZFar(float zFar)
	{
		m_zFar = zFar;
		m_isDirty = true;
	}

	float Camera::GetZFar() const
	{
		return m_zFar;
	}

	glm::mat4 Camera::GetProjectionMat()
	{
		if (m_isDirty)
			CalculateProjectionMatrix();
		return m_projectionMat;
	}

	void Camera::CalculateProjectionMatrix()
	{
		float screenWidth = static_cast<float>(m_screenWidth);
		float screenHeight = static_cast<float>(m_screenHeight);

		m_AspectRatio = screenWidth / screenHeight;
		m_projectionMat = glm::perspectiveFovRH(glm::radians<float>(m_fovInDeg), screenWidth, screenHeight, m_zNear, m_zFar);

		m_isDirty = false;
	}
}