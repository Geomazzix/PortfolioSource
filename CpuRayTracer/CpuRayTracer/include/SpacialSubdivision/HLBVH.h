#pragma once
#include "SpacialSubdivision/BVH.h"

namespace CRT
{
	/*
	/// <summary>
	/// Hierarchical Linear Bounding Volume hierarchy, this BVH resembles an optimized version of the normal SAH BVH.
	/// It constructs an upper BVH using the SAH calculation and uses the Middle divide at the bottom of the BVH, since there are
	/// not much primitives there.
	/// </summary>
	class HLBVH final : public BVH
	{
	public:
		HLBVH() = default;
		~HLBVH() = default;
	
		void Initialize(std::shared_ptr<JobSystem> jobSystem, const BVHConfig& config, Scene& scene) override;
		bool Intersect(const Ray& ray, Scene& scene, HitInfo& lastHitInfo, const float maxRayLength) override;

	private:
		/// <summary>
		/// Used to identify the position of the AABB of the primitive in morton code.
		/// </summary>
		struct MortonPrimitive
		{
			int PrimitiveIndex;
			uint32_t MortonCode;
		};

		std::shared_ptr<JobSystem> m_Jobsystem;
		BVHConfig m_Config;
		BVH::BVHBuildNode* Build(Scene& scene, const std::vector<BVHPrimitiveInfo>& primitiveInfo, int& totalNodes, std::vector<std::shared_ptr<Primitive>>& orderedPrimitives) const;
	};*/
}