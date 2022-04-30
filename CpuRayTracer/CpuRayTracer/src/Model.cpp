#include "Model.h"

namespace CRT
{
	void Model::Initialize(std::vector<std::shared_ptr<Mesh>>&& meshes, const glm::vec3& position, const glm::vec3& eulerRotation, const glm::vec3& scale)
	{
		m_Meshes = meshes;

		BVHConfig config;
		config.BinningCount = 4;
		config.MaxPrimitiveCountInNode = 2;

		std::vector<std::shared_ptr<Primitive>> triangles;
		for(auto& mesh : m_Meshes)
		{
			for(int i = 0; i < mesh->GetTriangleCount(); ++i)
			{
				triangles.push_back(mesh->GetTriangle(i));
			}
		}

		m_ModelBVH.Initialize(config, triangles);
	}

	void Model::Delete()
	{
		m_ModelBVH.Shutdown();
	}

	bool Model::Intersect(const Ray& ray, HitInfo& hitInfo, float maxRayLength)
	{
		return m_ModelBVH.Intersect(ray, hitInfo, maxRayLength);
	}

	CRT::Transform& Model::GetTransform()
	{
		return m_Transform;
	}

}