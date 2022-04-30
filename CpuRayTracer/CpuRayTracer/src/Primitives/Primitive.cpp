#include "Primitives/Primitive.h"

namespace CRT
{
	Primitive::Primitive() :
		m_Transform(),
		m_Material()
	{}

	void Primitive::Initialize(std::weak_ptr<Material> material, const glm::vec3& position /*= glm::vec3(0.0f)*/, const glm::vec3& eulerAngles /*= glm::vec3(0.0f)*/, const glm::vec3& scale /*= glm::vec3(1.0f)*/, const AABB& aabb)
	{
		m_Material = material;
		m_Transform.Translate(position);
		m_Transform.Rotate(eulerAngles);
		m_Transform.Scale(scale);
		m_AABB = aabb;
	}

	void Primitive::SetMaterial(std::weak_ptr<Material>& material)
	{
		m_Material = material;
	}

	CRT::Transform& Primitive::GetTransform()
	{
		return m_Transform;
	}

	std::weak_ptr<Material> Primitive::GetMaterial()
	{
		return m_Material;
	}

	const CRT::AABB& Primitive::GetAABB() const
	{
		return m_AABB;
	}

	CRT::AABB Primitive::GetWorldAABB()
	{
		const glm::mat4 matrix = m_Transform.GetMatrix();
		const glm::vec3 xa = glm::vec3(matrix[0]) * m_AABB.GetMin().x;
		const glm::vec3 xb = glm::vec3(matrix[0]) * m_AABB.GetMax().x;

		const glm::vec3 ya = glm::vec3(matrix[1]) * m_AABB.GetMin().y;
		const glm::vec3 yb = glm::vec3(matrix[1]) * m_AABB.GetMax().y;

		const glm::vec3 za = glm::vec3(matrix[2]) * m_AABB.GetMin().z;
		const glm::vec3 zb = glm::vec3(matrix[2]) * m_AABB.GetMax().z;

		return AABB(
			glm::min(xa, xb) + glm::min(ya, yb) + glm::min(za, zb) + m_Transform.GetTranslation(),
			glm::max(xa, xb) + glm::max(ya, yb) + glm::max(za, zb) + m_Transform.GetTranslation()
		);
	}
}