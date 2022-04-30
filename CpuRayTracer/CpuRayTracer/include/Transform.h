#pragma once
#include <glm.hpp>
#include <gtx/quaternion.hpp>
#include <memory>
#include <vector>
#include <string>

namespace CRT
{
	/// <summary>
	/// The transform class is part of the scene hierarchy and contains therefor a reference to it's parent and it's children.
	/// </summary>
	class Transform
	{
	public:
		Transform();
		~Transform() = default;
	
		glm::mat4 GetMatrix();
	
		void LookAt(const glm::vec3& targetPoint);

		void Translate(const glm::vec3& translation);
		void Rotate(const glm::vec3& eulerAngles);
		void Rotate(const glm::quat& quaternion);
		void Scale(const glm::vec3& scale);

		glm::vec3 GetTranslation() const;
		glm::quat GetRotation() const;
		glm::vec3 GetScale() const;

	private:
		bool m_IsDirty;

		glm::vec3 m_Translation;
		glm::quat m_Orientation;
		glm::vec3 m_Scale;

		glm::mat4 m_LocalTransform;

		void UpdateLocalTransform();
	};
}