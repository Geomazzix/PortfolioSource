#pragma once
#include <vec3.hpp>
#include <mat4x4.hpp>
#include <memory>
#include "SpacialSubdivision/AABB.h"
#include "Ray.h"
#include "Transform.h"
#include "Material.h"

namespace CRT
{
	/// <summary>
	/// Primitive abstract base class.
	/// </summary>
	class Primitive
	{
	public:
		Primitive();
		virtual ~Primitive() = default;

		virtual void Initialize(std::weak_ptr<Material> material, 
			const glm::vec3& position = glm::vec3(0.0f), 
			const glm::vec3& eulerAngles = glm::vec3(0.0f), 
			const glm::vec3& scale = glm::vec3(1.0f), 
			const AABB& aabb = AABB());
		virtual bool Intersect(const Ray& ray, HitInfo& hit, const float maxRayLength) = 0;

		void SetMaterial(std::weak_ptr<Material>& material);
		std::weak_ptr<Material> GetMaterial();
		Transform& GetTransform();

		const AABB& GetAABB() const;
		AABB GetWorldAABB();

	protected:
		Transform m_Transform;
		AABB m_AABB;
		std::weak_ptr<Material> m_Material;
	};
}