#pragma once
#include <vec3.hpp>
#include "Primitive.h"

namespace CRT
{
	/// <summary>
	/// The box primitive represents 2 corner points in space, which checks if a ray intersects in between.
	/// </summary>
	class Box : public Primitive
	{
	public:
		Box();
		~Box() = default;

		bool Intersect(const Ray& ray, HitInfo& hit, const float maxRayLength) override;
		glm::vec3 GetNormal(const glm::vec3& point);

	private:
		glm::vec3 m_Bounds[2];
	};
}