#pragma once
#include <vec3.hpp>
#include <mat4x4.hpp>

namespace CRT
{
	class Primitive;

	/// <summary>
	/// A ray represents a directional vector.
	/// </summary>
	struct Ray
	{
		glm::vec3 Origin = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 Direction = glm::vec3(0.0f, 0.0f, 1.0f);
	};

	/// <summary>
	/// The hit info returns from a raycast - inspired by Unity.
	/// </summary>
	struct HitInfo
	{
		Primitive* HitPrimitive = nullptr;
		glm::vec3 Point = glm::vec3(0.0f);
		glm::vec3 Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		float Distance = 0.0f;

#if _DEBUG
		glm::vec3 BVHColor = glm::vec3(0.0f);
#endif
	};
}