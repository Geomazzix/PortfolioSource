#include "Mesh.h"

namespace CRT
{
	void Mesh::SetTriangles(std::vector<std::shared_ptr<Primitive>>&& triangles)
	{
		m_Triangles = triangles;

		BVHConfig config;
		config.BinningCount = 12;
		config.MaxPrimitiveCountInNode = 2;

		m_MeshBVH.Initialize(config, m_Triangles);
	}

	std::shared_ptr<CRT::Primitive> Mesh::GetTriangle(int index)
	{
		return m_Triangles[index];
	}

	size_t Mesh::GetTriangleCount() const
	{
		return m_Triangles.size();
	}

	void Mesh::Delete()
	{
		m_MeshBVH.Shutdown();
	}

	bool Mesh::Intersect(const Ray& ray, HitInfo& hitInfo, float maxRayLength)
	{
		//Transform the ray to object space.
		const glm::mat4 inverse = glm::inverse(m_Transform.GetMatrix());
		const Ray localSpaceRay =
		{
			inverse * glm::vec4(ray.Origin, 1.0f),
			inverse * glm::vec4(ray.Direction, 0.0f)
		};

		if(m_MeshBVH.Intersect(localSpaceRay, hitInfo, maxRayLength))
		{
			//Calculate the actual intersection point for world space in the mesh instead of the triangle, since the triangle is based on the model space.
			hitInfo.Point = ray.Origin + ray.Direction * hitInfo.Distance;
			return true;
		}

		return false;
	}
}