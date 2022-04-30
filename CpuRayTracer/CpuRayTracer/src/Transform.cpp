#include "Transform.h"
#include <gtx/transform.hpp>

namespace CRT
{
	Transform::Transform() :
		m_Translation(glm::vec3(0.0f)),
		m_Orientation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
		m_Scale(glm::vec3(1.0f)),
		m_IsDirty(true),
		m_LocalTransform(glm::identity<glm::mat4>())
	{}

	glm::mat4 Transform::GetMatrix()
	{
		if (m_IsDirty)
			UpdateLocalTransform();
		return m_LocalTransform;
	}

	void Transform::LookAt(const glm::vec3& targetPoint)
	{
		glm::vec3 localUp = glm::vec3(m_LocalTransform[1].x, m_LocalTransform[1].y, m_LocalTransform[1].z);
		glm::vec3 direction = glm::normalize(targetPoint - m_Translation);
		m_Orientation = glm::quatLookAtLH(direction, localUp);
		m_IsDirty = true;
	}

	void Transform::Translate(const glm::vec3& translation)
	{
		m_Translation += translation;
		m_IsDirty = true;
	}

	void Transform::Rotate(const glm::vec3& eulerAngles)
	{
		Rotate(glm::quat(eulerAngles));
	}

	void Transform::Rotate(const glm::quat& quaternion)
	{
		m_Orientation *= quaternion;
		m_IsDirty = true;
	}

	void Transform::Scale(const glm::vec3& scale)
	{
		m_Scale *= scale;
		m_IsDirty = true;
	}

	glm::vec3 Transform::GetTranslation() const
	{
		return m_Translation;
	}

	glm::quat Transform::GetRotation() const
	{
		return m_Orientation;
	}

	glm::vec3 Transform::GetScale() const
	{
		return m_Scale;
	}

	void Transform::UpdateLocalTransform()
	{
		glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), m_Translation);
		glm::mat4 orientation = glm::toMat4(m_Orientation);
		glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), m_Scale);

		m_LocalTransform = translation * orientation * scale;
		m_IsDirty = false;
	}
}