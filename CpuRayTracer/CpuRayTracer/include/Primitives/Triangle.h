#pragma once
#include "Ray.h"
#include "Primitives/Primitive.h"

namespace CRT
{
	/// <summary>
	/// The triangle primitive doesn't actually inherit from the primitive base class, since that would lead to a lot of memory issues.
	/// </summary>
	class Triangle : public Primitive
	{
	public:
		Triangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& normal);
		~Triangle() = default;

		//Implemented using the Möller-Trumbone ray-triangle intersection algorithm.
		//https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm#:~:text=The%20M%C3%B6ller%E2%80%93Trumbore%20ray%2Dtriangle,the%20plane%20containing%20the%20triangle.
		bool Intersect(const Ray& ray, HitInfo& hitInfo, const float maxRayLength);

	private:
		glm::vec3 m_TrianglePoints[3];
		glm::vec3 m_Normal;
	};
}