#pragma once
#include "Ray.h"
#include "Primitive.h"
#include <vec3.hpp>

namespace CRT
{
	/// <summary>
	/// Represents a sphere, though can be transformed into an ellipsoid.
	/// </summary>
	class Sphere : public Primitive
	{
	public:
		Sphere() = default;
		~Sphere() = default;

		bool Intersect(const Ray& ray, HitInfo& hitInfo, const float maxRayLength) override;
		glm::vec3 GetNormal(const glm::vec3& point);
	};
}