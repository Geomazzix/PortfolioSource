#pragma once
#include <vec3.hpp>

namespace CRT
{
	/// <summary>
	/// The material holds all the data used to calculate the appropriate lighting.
	/// </summary>
	struct Material
	{
		glm::vec3 AmbientReflectivity = glm::vec3(0.0f);
		glm::vec3 EmissionCoefficient = glm::vec3(0.0f);
		glm::vec3 AlbedoCoefficient = glm::vec3(1.0f, 0.2f, 1.0f);
		glm::vec3 SpecularCoefficient = glm::vec3(0.5f);
		int PhongExponent = 1024;

		glm::vec3 AbsorbanceCoefficient = glm::vec3(1.0f);
		float Reflectivity = 0.01f;
		float RefractiveIndex = 1.0f;

		bool IsReflective = false;
		bool IsDialetic = false;
	};
}