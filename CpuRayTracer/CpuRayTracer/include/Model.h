#pragma once
#include "Mesh.h"
#include "Transform.h"
#include "Ray.h"
#include "Primitives/Triangle.h"

namespace CRT
{
	/// <summary>
	/// A model represents a group of meshes. It therefor defines a group of surfaces.
	/// </summary>
	class Model
	{
	public:
		Model() = default;
		~Model() = default;

		void Initialize(std::vector<std::shared_ptr<Mesh>>&& meshes, const glm::vec3& position, const glm::vec3& eulerRotation, const glm::vec3& scale);
		void Delete();

		bool Intersect(const Ray& ray, HitInfo& hitInfo, float maxRayLength);
		Transform& GetTransform();

	private:
		Transform m_Transform;
		BVH m_ModelBVH;
		std::vector<std::shared_ptr<Mesh>> m_Meshes;
	};
}