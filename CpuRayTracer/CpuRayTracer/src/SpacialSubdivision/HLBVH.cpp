#include "SpacialSubdivision/HLBVH.h"

namespace CRT
{
	/*
	void HLBVH::Initialize(std::shared_ptr<JobSystem> jobSystem, const BVHConfig& config, Scene& scene)
	{
		m_Config = config;
		if(scene.GetPrimitiveCount() <= 0)
		{
			printf("WARNING: No primitives found in the scene, cannot create BVH!\n");
			return;
		}

		std::vector<BVHPrimitiveInfo> primitiveInfo(scene.GetPrimitiveCount());
		for (int i = 0; i < scene.GetPrimitiveCount(); ++i)
		{
			primitiveInfo[i] = {
				i,
				scene.GetPrimitive(i).GetWorldAABB()
			};
		}

		int totalNodes = 0;
		std::vector<std::shared_ptr<Primitive>> orderedPrimitives;
		orderedPrimitives.reserve(scene.GetPrimitiveCount());

		printf("Started BVH construction!\n");
		BVHBuildNode* root = Build(scene, primitiveInfo, totalNodes, orderedPrimitives);
		printf("Finished BVH construction!\n");

		m_Primitives.swap(orderedPrimitives);
		primitiveInfo.resize(0);
	}

	CRT::BVH::BVHBuildNode* CRT::HLBVH::Build(Scene& scene, const std::vector<BVHPrimitiveInfo>& primitiveInfo, int& totalNodes, std::vector<std::shared_ptr<Primitive>>& orderedPrimitives) const
	{
		//Compute bounding box of all primitive centroids.
		AABB nodeBounds = AABB(glm::vec3(INFINITY), glm::vec3(-INFINITY));
		for(const BVHPrimitiveInfo& info : primitiveInfo)
		{
			nodeBounds = Combine(nodeBounds, info.Centroid);
		}

		//Compute Morton indicies of primitives.
		std::vector<MortonPrimitive> mortonPrimitives(primitiveInfo.size());
		for(int i = 0; i < primitiveInfo.size(); i++)
		{
			//#TODO: Multithread this for loop, by assigning a worker thread a primitive count it should calculate Morton curves for.

		}

		//Raxis sort primitive morton indices.

		//Create LBVH treelets at the bottom of the BVH.

		//Create and return SAH BVH from LBVH treelets.
		return nullptr;
	}

	bool HLBVH::Intersect(const Ray& ray, Scene& scene, HitInfo& lastHitInfo, const float maxRayLength)
	{
		return false;
	}*/
}