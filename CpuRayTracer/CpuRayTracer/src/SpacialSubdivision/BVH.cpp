#include "SpacialSubdivision/BVH.h"
#include <gtx/transform.hpp>
#include <glm.hpp>
#include <algorithm>
#include <geometric.hpp>
#include "Memory.h"

namespace CRT
{
	BVH::BVH() 
		: Primitive()
	{}

	void BVH::Initialize(const BVHConfig& config, const std::vector<std::shared_ptr<Primitive>>& primitives)
	{
		if (primitives.size() <= 0)
		{
			printf("WARNING: No primitives found in the scene, cannot create BVH!\n");
			return;
		}

		m_Primitives = primitives;
		m_Config = config;

		std::vector<BVHPrimitiveInfo> primitiveInfo(m_Primitives.size());
		for(int i = 0; i < m_Primitives.size(); ++i)
		{
			primitiveInfo[i] = {
				i,
				m_Primitives[i]->GetWorldAABB()
			};
		}

		int totalNodes = 0;
		std::vector<std::shared_ptr<Primitive>> orderedPrimitives;
		orderedPrimitives.reserve(m_Primitives.size());

		printf("Started BVH construction!\n");
		BVHBuildNode* root = Build(primitiveInfo, 0, primitiveInfo.size(), totalNodes, orderedPrimitives);
		printf("Finished BVH construction!\n");

		m_Primitives.swap(orderedPrimitives);
		primitiveInfo.resize(0);

		m_Nodes = AllocAligned<LinearBVHNode>(totalNodes);

		int offset = 0;
		FlattenBVHTree(root, offset);
		if(totalNodes != offset)
		{
			printf("The BVH tree failed to flatten!");
			return;
		}
	}

	BVH::BVHBuildNode* BVH::Build(std::vector<BVH::BVHPrimitiveInfo>& primitiveInfo, int start, int end, int& totalNodes, std::vector<std::shared_ptr<Primitive>>& orderedPrimitives)
	{
		BVHBuildNode* node = new BVHBuildNode();
		int numPrimitives = end - start;
		totalNodes++;

		//Calculate the outer bounds of this node.
		AABB nodeBounds = AABB(glm::vec3(INFINITY), glm::vec3(-INFINITY));
		for (int i = start; i < end; i++)
		{
			nodeBounds = AABB::Combine(nodeBounds, primitiveInfo[i].Bounds);
		}

		//Check whether this node is a leaf or branch node.
		if(numPrimitives == 1)
		{
			//When only one primitive is available, create a leaf node with the given properties.
			int firstPrimitiveOffset = orderedPrimitives.size();
			for(int i = start; i < end; ++i)
			{
				orderedPrimitives.push_back(m_Primitives[primitiveInfo[i].PrimitiveIndex]);
			}
			InitializeAsLeafNode(node, firstPrimitiveOffset, numPrimitives, nodeBounds);
			return node;
		}
		else
		{
			//When more than 1 primitive is found in the AABB confinement, create a branch node.
			AABB centroidBounds = AABB(glm::vec3(INFINITY), glm::vec3(-INFINITY));;
			for (int i = start; i < end; ++i)
			{
				centroidBounds = AABB::Combine(centroidBounds, primitiveInfo[i].Centroid);
			}
			ESortAxis sortAxis = GetSortAxis(centroidBounds, start, end);
			int mid = (start + end) / 2;

			//Check if the centroid points fall on the same position (i.e. the centroid bounds have 0 volume).
			if(centroidBounds.GetMax()[sortAxis] == centroidBounds.GetMin()[sortAxis])
			{
				int firstPrimitiveOffset = orderedPrimitives.size();
				for (int i = start; i < end; ++i)
				{
					orderedPrimitives.push_back(m_Primitives[primitiveInfo[i].PrimitiveIndex]);
				}
				InitializeAsLeafNode(node, firstPrimitiveOffset, numPrimitives, nodeBounds);
				return node;
			}
			else
			{
				switch (m_Config.PartitionType)
				{
				case EPartitionType::MIDDLE:
				{
					//Partition through the node's midpoint.
					float pmid = (centroidBounds.GetMin()[sortAxis] + centroidBounds.GetMax()[sortAxis]) * 0.5f;
					BVHPrimitiveInfo* midPtr = std::partition(
						&primitiveInfo[start],
						&primitiveInfo[end - 1] + 1,
						[sortAxis, pmid](const BVHPrimitiveInfo& info)
						{
							return info.Centroid[sortAxis] < pmid;
						});

					int mid = midPtr - &primitiveInfo[0];

					//In case overlapping AABBs seem to fail, try the equal counts approach.
					if (mid != start && mid != end)
						break;
				}
				case EPartitionType::EQUALCOUNTS:
				{
					//Split the node into 2 equal sized leafs.
					int mid = (start - end) / 2;
					std::nth_element(&primitiveInfo[start], &primitiveInfo[mid], &primitiveInfo[end - 1] + 1,
						[sortAxis](const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b)
						{
							return a.Centroid[sortAxis] < b.Centroid[sortAxis];
						});
					break;
				}
				case EPartitionType::SAH:
				{
					//Create the buckets for the sorting axis partition line.
					std::vector<BucketInfo> buckets(m_Config.BinningCount);
					for (int i = start; i < end; ++i)
					{
						int b = buckets.size() * centroidBounds.GetOffset(primitiveInfo[i].Centroid)[sortAxis];
						if (b == m_Config.BinningCount)
						{
							b = m_Config.BinningCount - 1;
						}

						buckets[b].Count++;
						buckets[b].Bounds = AABB::Combine(buckets[b].Bounds, primitiveInfo[i].Bounds);
					}

					//Calculate the Surface Area Heuristic.
					std::vector<float> sahCosts(m_Config.BinningCount - 1);
					for (int i = 0; i < m_Config.BinningCount - 1; ++i)
					{
						AABB ba = AABB(glm::vec3(INFINITY), glm::vec3(-INFINITY));
						AABB bb = AABB(glm::vec3(INFINITY), glm::vec3(-INFINITY));
						int countA = 0;
						int countB = 0;

						for (int j = 0; j <= i; ++j)
						{
							ba = AABB::Combine(ba, buckets[j].Bounds);
							countA += buckets[j].Count;
						}

						for (int j = i + 1; j < m_Config.BinningCount; ++j)
						{
							bb = AABB::Combine(bb, buckets[j].Bounds);
							countB += buckets[j].Count;
						}

						sahCosts[i] = m_Config.TraversalCost + (countA * ba.GetSurfaceArea() + countB * bb.GetSurfaceArea()) / nodeBounds.GetSurfaceArea();
					}

					//Find the optimal SAH cost and utilize it.
					float minCost = sahCosts[0];
					int minCostSplitBucket = 0;
					for (int i = 1; i < m_Config.BinningCount - 1; ++i)
					{
						if (sahCosts[i] < minCost)
						{
							minCost = sahCosts[i];
							minCostSplitBucket = i;
						}
					}

					//Either create a leaf node or split the primitives at the selected SAH bucket.
					int numPrimitives = end - start;
					if (numPrimitives > m_Config.MaxPrimitiveCountInNode || minCost < numPrimitives)
					{
						BVHPrimitiveInfo* primitiveMid = std::partition(&primitiveInfo[start], &primitiveInfo[end - 1] + 1,
							[=](const BVHPrimitiveInfo& infoItem)
							{
								int b = m_Config.BinningCount * centroidBounds.GetOffset(infoItem.Centroid)[sortAxis];
								if (b == m_Config.BinningCount)
								{
									b = m_Config.BinningCount - 1;
								}
								return b <= minCostSplitBucket;
							});

						mid = primitiveMid - &primitiveInfo[0];
					}
					else
					{
						//When only one primitive is available, create a leaf node with the given properties.
						int firstPrimitiveOffset = orderedPrimitives.size();
						for (int i = start; i < end; ++i)
						{
							orderedPrimitives.push_back(m_Primitives[primitiveInfo[i].PrimitiveIndex]);
						}
						InitializeAsLeafNode(node, firstPrimitiveOffset, numPrimitives, nodeBounds);
						return node;
					}
					break;
				}
				default:
					printf("Undefined partition method!\n");
					break;
				}

				BVHBuildNode* leftChild = Build(primitiveInfo, start, mid, totalNodes, orderedPrimitives);
				BVHBuildNode* rightChild = Build(primitiveInfo, mid, end, totalNodes, orderedPrimitives);
				InitializeAsInteriorNode(node, sortAxis, leftChild, rightChild);
			}
		}

		return node;
	}

	CRT::BVH::ESortAxis BVH::GetSortAxis(const AABB& centroidBounds, int start, int end)
	{
		glm::vec3 diagonal = centroidBounds.GetMax() - centroidBounds.GetMin();
		if (diagonal.x > diagonal.y && diagonal.x > diagonal.z)
		{
			return ESortAxis::X;
		}
		else if (diagonal.y > diagonal.z)
		{
			return ESortAxis::Y;
		}
		else
		{
			return ESortAxis::Z;
		}
	}

	void BVH::InitializeAsLeafNode(BVHBuildNode* buildNode, int first, int numPrimitives, const AABB& bounds)
	{
		buildNode->Children[0] = nullptr;
		buildNode->Children[1] = nullptr;
		buildNode->FirstPrimitiveOffset = first;
		buildNode->Bounds = bounds;
		buildNode->NumPrimitives = numPrimitives;
	}

	void BVH::InitializeAsInteriorNode(BVHBuildNode* buildNode, ESortAxis axis, BVHBuildNode* left, BVHBuildNode* right)
	{
		buildNode->Children[0] = left;
		buildNode->Children[1] = right;
		buildNode->Bounds = AABB::Combine(left->Bounds, right->Bounds);
		buildNode->SplitAxis = axis;
		buildNode->NumPrimitives = 0;
	}

	int BVH::FlattenBVHTree(BVHBuildNode* node, int& offset)
	{
		LinearBVHNode* linearNode = &m_Nodes[offset];
		linearNode->Bounds = node->Bounds;
		int linearNodeOffset = offset++;

		if(node->NumPrimitives > 0)
		{
			//Leaf node.
			linearNode->PrimitiveOffset = node->FirstPrimitiveOffset;
			linearNode->NumPrimitives = node->NumPrimitives;
		}
		else
		{
			//Interior node.
			linearNode->Axis = node->SplitAxis;
			linearNode->NumPrimitives = 0;
			FlattenBVHTree(node->Children[0], offset);
			linearNode->SecondChildOffset = FlattenBVHTree(node->Children[1], offset);
		}

		return linearNodeOffset;
	}

	bool BVH::Intersect(const Ray& ray, HitInfo& lastHitInfo, const float maxRayLength)
	{
		bool hit = false;
		glm::vec3 inverseDirection = glm::vec3(
			1.0f / ray.Direction.x,
			1.0f / ray.Direction.y,
			1.0f / ray.Direction.z
		);

		int directionIsNegative[3] = {
			inverseDirection.x < 0,
			inverseDirection.y < 0,
			inverseDirection.z < 0
		};

		int toVisitOffset = 0;
		int currentNodeIndex = 0;
		int nodesToVisit[64];

#if _DEBUG
		int currentLayer = 0;
		glm::vec3 bvhEdgeLayerColor = glm::vec3(0.0f);
#endif

		lastHitInfo.Distance = maxRayLength;
		HitInfo hitInfo;

		while(true)
		{
			const LinearBVHNode* node = &m_Nodes[currentNodeIndex];
#if _DEBUG
			float debugTmin = 0.0f;
			float debugTmax = 0.0f;
			bool debugHit = currentLayer == m_DebugLayer
				? node->Bounds.Intersect(ray, debugTmax, debugTmin, bvhEdgeLayerColor, inverseDirection, directionIsNegative, maxRayLength)
				: node->Bounds.Intersect(ray, inverseDirection, directionIsNegative, maxRayLength);

			if (debugHit)
			{
				if (currentLayer == m_DebugLayer && bvhEdgeLayerColor != glm::vec3(0.0f))
				{
					lastHitInfo.BVHColor = bvhEdgeLayerColor;
				}
#else
			if (node->Bounds.Intersect(ray, inverseDirection, directionIsNegative, maxRayLength))
			{
#endif
				if(node->NumPrimitives > 0)
				{
					for(int i = 0; i < node->NumPrimitives; ++i)
					{
						if (m_Primitives[node->PrimitiveOffset + i]->Intersect(ray, hitInfo, maxRayLength) && hitInfo.Distance < lastHitInfo.Distance)
						{
							lastHitInfo = hitInfo;
#if _DEBUG
							lastHitInfo.BVHColor = bvhEdgeLayerColor;
#endif
						}
					}

					if (toVisitOffset == 0)
					{
						break;
					}

					currentNodeIndex = nodesToVisit[--toVisitOffset];
				}
				else
				{
					if (directionIsNegative[node->Axis])
					{
						nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
						currentNodeIndex = node->SecondChildOffset;
					}
					else
					{
						nodesToVisit[toVisitOffset++] = node->SecondChildOffset;
						currentNodeIndex = currentNodeIndex + 1;
					}
#if _DEBUG
					++currentLayer;
#endif
				}
			}
			else
			{
				if(toVisitOffset == 0)
				{
					break;
				}
				currentNodeIndex = nodesToVisit[--toVisitOffset];
			}
		}

		return lastHitInfo.Distance < maxRayLength;
	}

	void BVH::Shutdown()
	{
		FreeAligned(m_Nodes);
	}

#if defined(_DEBUG)
	void BVH::SetDebugLayer(int index)
	{
		m_DebugLayer = index;
	}
#endif
}