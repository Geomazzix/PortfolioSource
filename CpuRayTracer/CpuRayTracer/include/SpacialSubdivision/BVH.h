#pragma once
#include "SpacialSubdivision/AABB.h"
#include "Ray.h"
#include <vec3.hpp>
#include <memory>
#include "Primitives/Primitive.h"

namespace CRT
{
	/// <summary>
	/// Used to identify how to partition the BVH during construction.
	/// </summary>
	enum class EPartitionType
	{
		MIDDLE = 0,
		EQUALCOUNTS,
		SAH
	};

	/// <summary>
	/// The BVH can be configured for different results, there are default values at present.
	/// </summary>
	struct BVHConfig
	{
		float TraversalCost = 0.35f;
		float IntersectionCost = 1;
		int BinningCount = 16;
		int MaxPrimitiveCountInNode = 1;
		EPartitionType PartitionType = EPartitionType::SAH;

		void operator=(const BVHConfig& config)
		{
			TraversalCost = config.TraversalCost;
			IntersectionCost = config.IntersectionCost;
			BinningCount = config.BinningCount;
			MaxPrimitiveCountInNode = config.MaxPrimitiveCountInNode;
			PartitionType = config.PartitionType;
		}
	};

	/// <summary>
	/// Represents a Boundary Volume Hierarchy based on the pbr-book.org implementation, this serves as a base class and constructs only through the means of SAH/middle/equal count.
	/// http://www.pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies.html#fragment-CreateleafmonoBVHBuildNode-0
	/// </summary>
	class BVH : public Primitive
	{
	public:
		BVH();
		~BVH() = default;

		virtual void Initialize(const BVHConfig& config, const std::vector<std::shared_ptr<Primitive>>& primitives);
		virtual bool Intersect(const Ray& ray, HitInfo& lastHitInfo, const float maxRayLength) override;
		virtual void Shutdown();

#if defined(_DEBUG)
		void SetDebugLayer(int index);
#endif

	protected:
		/// <summary>
		/// Used during BVH construction.
		/// </summary>
		enum ESortAxis
		{
			X = 0,
			Y, Z
		};

		/// <summary>
		/// Used to identify the primitive nodes in the BVH tree.
		/// </summary>
		struct BVHPrimitiveInfo
		{
			int PrimitiveIndex;
			AABB Bounds;
			glm::vec3 Centroid;

			BVHPrimitiveInfo() : PrimitiveIndex(-1), Bounds(), Centroid(0.0f) {};
			BVHPrimitiveInfo(int primitiveNumber, const AABB& bounds) :
				PrimitiveIndex(primitiveNumber),
				Bounds(bounds),
				Centroid(bounds.GetCenter())
			{}
		};

		/// <summary>
		/// Used to identify the type of nodes in the BVH tree.
		/// </summary>
		struct BVHBuildNode
		{
			AABB Bounds;
			BVHBuildNode* Children[2];
			ESortAxis SplitAxis;
			int FirstPrimitiveOffset, NumPrimitives;
		};

		/// <summary>
		/// The Linear BVH node is used to traverse over the tree in a memory cache friendly way.
		/// </summary>
		struct LinearBVHNode
		{
			AABB Bounds;
			union
			{
				int PrimitiveOffset;	//Leaf.
				int SecondChildOffset;	//Interior.
			};

			uint16_t NumPrimitives;
			uint16_t Axis;
		};

		const int NUMBER_OF_PRIMITIVES_PER_NODE = 2;
		BVHConfig m_Config;


		void InitializeAsLeafNode(BVHBuildNode* node, int first, int numPrimitives, const AABB& bounds);
		void InitializeAsInteriorNode(BVHBuildNode* buildNode, ESortAxis axis, BVHBuildNode* left, BVHBuildNode* right);

		int FlattenBVHTree(BVHBuildNode* node, int& offset);

		LinearBVHNode* m_Nodes;
		std::vector<std::shared_ptr<Primitive>> m_Primitives;

	private:
#if defined(_DEBUG)
		int m_DebugLayer;
#endif

		/// <summary>
		/// Used when constructing an SAH tree.
		/// </summary>
		struct BucketInfo
		{
			int Count = 0;
			AABB Bounds = AABB(glm::vec3(INFINITY), glm::vec3(-INFINITY));
		};

		BVH::BVHBuildNode* Build(std::vector<BVH::BVHPrimitiveInfo>& primitiveInfo, int start, int end, int& totalNodes, std::vector<std::shared_ptr<Primitive>>& orderedPrimitives);
		ESortAxis GetSortAxis(const AABB& centroidBounds, int start, int end);
	};
}