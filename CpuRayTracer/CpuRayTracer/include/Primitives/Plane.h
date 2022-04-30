#pragma once
#include "Ray.h"
#include "Primitive.h"

namespace CRT
{
	/// <summary>
	/// A finite plane primitive.
	/// </summary>
	class Plane : public Primitive
	{
	public:
		Plane() = default;
		~Plane() = default;

		bool Intersect(const Ray& ray, HitInfo& hitInfo, const float maxRayLength) override;
		glm::vec3 GetNormal();
	};
}