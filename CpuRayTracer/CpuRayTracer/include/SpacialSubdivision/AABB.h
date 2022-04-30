#pragma once
#include <vec3.hpp>
#include "Ray.h"

namespace CRT
{
	/// <summary>
	/// Axis-Aligned-Bounding-Box used by the acceleration structures, contained by the primitive base class.
	/// </summary>
	class AABB
	{
	public:
		AABB(const glm::vec3& min = glm::vec3(-1.0f), const glm::vec3& max = glm::vec3(1.0f));
		~AABB() = default;

		bool Intersect(const Ray& ray, const glm::vec3& inverseDirection, const int directionIsNegative[3], const float maxRayLength) const;

#if _DEBUG
		bool Intersect(const Ray& ray, float& debugTmax, float& debugTmin, glm::vec3& bvhColor, const glm::vec3& inverseDirection, const int directionIsNegative[3], const float maxRayLength) const;
#endif

		const glm::vec3& GetMin() const;
		const glm::vec3& GetMax() const;
		
		glm::vec3 GetCenter() const;
		glm::vec3 GetOffset(const glm::vec3& point) const;
		float GetSurfaceArea() const;

		static AABB Combine(const AABB& a, const AABB& b);
		static AABB Combine(const AABB& a, const glm::vec3& b);

	private:
		glm::vec3 m_Bounds[2];
	};
}