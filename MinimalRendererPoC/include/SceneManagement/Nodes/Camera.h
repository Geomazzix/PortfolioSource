#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "EventSystem/EventArgs.h"

namespace mrpoc
{
	/// <summary>
	/// Contains the different types of camera the renderer can provide for the renderer.
	/// </summary>
	enum class ECameraType
	{
		Perspective,
		Orthographic
	};

	/// <summary>
	/// Camera handles the project matrix and the settings that apply to it.
	/// </summary>
	class Camera
	{
	public:
		Camera();
		~Camera();

		void Initialize(int screenWidth, int screenHeight, float fovInDeg, float zNear, float zFar);

		void OnScreenResize(ResizeEventArgs& args);

		void SetFOV(float fov);
		float GetFOVInDeg() const;

		void SetZNear(float zNear);
		float GetZNear() const;

		void SetZFar(float zFar);
		float GetZFar() const;

		float GetAspectRatio();
		glm::mat4 GetProjectionMat();

	private:
		int m_screenWidth;
		int m_screenHeight;
		
		float m_fovInDeg;
		float m_zNear;
		float m_zFar;

		float m_AspectRatio;
		glm::mat4 m_projectionMat;
		bool m_isDirty;
		
		ECameraType m_cameraType;

		void CalculateProjectionMatrix();
	};
}