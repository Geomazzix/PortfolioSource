#pragma once
#include <vec3.hpp>
#include <memory>
#include "Transform.h"

namespace CRT
{
	/// <summary>
	/// The ambient light serves as a very simple replacement for global illumination.
	/// </summary>
	struct AmbientLight
	{
		float Intensity = 0.1f;
	};

	/// <summary>
	/// Used to identify the lights.
	/// </summary>
	enum class ELightType
	{
		DIRECTIONAL = 0,
		POINT
	};

	/// <summary>
	/// The light base class represents a directional light.
	/// </summary>
	class Light
	{
	public:
		Light();
		virtual ~Light() = default;

		void Initialize(ELightType type, const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& eulerAngles = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f));

		void SetIntensity(float intensity);
		float GetIntensity() const;

		ELightType GetType() const;
		Transform& GetTransform();

		//Based on the linear + quadratic lighting attentuation from Blender.
		//https://docs.blender.org/manual/fr/2.79/render/blender_render/lighting/lights/attenuation.html
		virtual float CalculateLightingAttentuation(const glm::vec3& point) = 0;

	protected:
		float m_Intensity;
		Transform m_Transform;
		ELightType m_Type;
	};

	/// <summary>
	/// The point light class represents a light position with a radius.
	/// </summary>
	class PointLight : public Light
	{
	public:
		PointLight();
		virtual ~PointLight() = default;

		void SetRange(float radius);
		float GetRange() const;

		float CalculateLightingAttentuation(const glm::vec3& point) override;

	protected:
		float m_Range;
	};
}