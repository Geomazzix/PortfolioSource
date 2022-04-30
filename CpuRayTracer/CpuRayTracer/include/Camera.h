#pragma once
#include <glm.hpp>
#include <gtx/transform.hpp>
#include "Transform.h"

namespace CRT
{
	///<summary>Used to select the type rendering the camera will perform.</summary>
	enum class ECameraType
	{
		PERSPECTIVE = 0,
		ORTHOGRAPHIC,
		OBLIQUE
	};

	/// <summary>
	/// The camera class takes care of camera specific properties.
	/// </summary>
	class Camera
	{
	public:
		Camera();
		~Camera() = default;

		void Initialize(int pixelWidth, int pixelHeight, ECameraType type = ECameraType::PERSPECTIVE, float zNear = 0.1f, float zFar = 1000.0f);
		
		void SetFieldOfView(float fovInDeg);
		float GetFovInRads() const;

		ECameraType GetCameraType() const;
		float GetAspectRatio() const;

		float GetZNear() const;
		float GetZFar() const;

		void SetAperture(float aperture);
		float GetAperture() const;

		void SetFocalLength(float focalLength);
		float GetFocalLength() const;

		void SetCameraSize(float size);
		float GetCameraSize() const;

		int GetViewPortPixelWidth() const;
		int GetViewPortPixelHeight() const;

		Transform& GetTransform();

	private:
		Transform m_Transform;

		int m_ViewPortPixelWidth;
		int m_ViewPortPixelHeight;
		float m_AspectRatio;
		float m_Aperture;

		ECameraType m_Type;

		float m_CameraSize;
		float m_FovInRad;
		float m_FocalLength;

		float m_ZNear;
		float m_ZFar;
	};
}