#include "Light.h"

namespace CRT
{
	Light::Light() :
		m_Intensity(1.0f),
		m_Type(ELightType::DIRECTIONAL),
		m_Transform()
	{}

	void Light::Initialize(ELightType type, const glm::vec3& position /*= glm::vec3(0.0f)*/, const glm::vec3& eulerAngles /*= glm::vec3(0.0f)*/, const glm::vec3& scale /*= glm::vec3(1.0f)*/)
	{
		m_Transform.Translate(position);
		m_Transform.Rotate(eulerAngles);
		m_Transform.Scale(scale);
		m_Type = type;
	}

	Transform& Light::GetTransform()
	{
		return m_Transform;
	}

	void Light::SetIntensity(float intensity)
	{
		m_Intensity = intensity;
	}

	float Light::GetIntensity() const
	{
		return m_Intensity;
	}


	ELightType Light::GetType() const
	{
		return m_Type;
	}

	PointLight::PointLight() :
		Light(),
		m_Range(10.0f)
	{}

	void PointLight::SetRange(float radius)
	{
		m_Range = radius;
	}

	float PointLight::GetRange() const
	{
		return m_Range;
	}

	float PointLight::CalculateLightingAttentuation(const glm::vec3& point)
	{
		float distance = glm::length(m_Transform.GetTranslation() - point);

		float E = m_Intensity;
		float DD = m_Range * m_Range;
		float Q = 1;
		float rr = distance * distance;

		return E * (DD / (DD + Q * rr));
	}
}