#pragma once
#include "Ray.h"

namespace CRT
{
	/// <summary>
	/// Interface used for anything that can be checked for intersections.
	/// </summary>
	class IIntersectable
	{
	public:

		IIntersectable() = default;
		virtual ~IIntersectable() = default;
		
		virtual bool Intersect(const Ray& ray, HitInfo& hit, const float maxRayLength) = 0;
	};
}