#pragma once
#include "SpacialSubdivision/BVH.h"
#include "Primitives/Triangle.h"

namespace CRT
{
	/// <summary>
	/// The mesh represents a subgroup of vertices that has a material assigned to them.
	/// It by itself is not capable of being rendered, but using the model class it is.
	/// </summary>
	class Mesh : public Primitive
	{
	public:
		Mesh() = default;
		~Mesh() = default;

		void SetTriangles(std::vector<std::shared_ptr<Primitive>>&& triangles);
		void Delete();

		std::shared_ptr<Primitive> GetTriangle(int index);
		size_t GetTriangleCount() const;

		bool Intersect(const Ray& ray, HitInfo& hitInfo, float maxRayLength);

	private:
		std::vector<std::shared_ptr<Primitive>> m_Triangles;
		BVH m_MeshBVH;
	};
}