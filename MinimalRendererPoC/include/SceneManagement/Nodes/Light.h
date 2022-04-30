#pragma once
#include "glm/vec3.hpp"

namespace mrpoc 
{
	/// <summary>
	/// Represents different light type.s
	/// </summary>
	enum class ELightType
	{
		POINT,
		DIRECTIONAL,
		SPOT
	};

	/// <summary>
	/// The light represents a directional light by default, though the direction is passed in through the use of the worldmatrix.
	/// </summary>
	struct Light
	{
		glm::vec3 Color;
		float Intensity;
		ELightType Type;
	};

	/// <summary>
	/// The point light represents a light source that shines in all directions constraint by range.
	/// </summary>
	struct PointLight : public Light
	{
		float Radius;
	};

	/// <summary>
	/// A spot light defines a light with a direction and a specified angle within the inner and outer cone.
	/// *note that the direction is derived from the rotation of the world matrix.
	/// </summary>
	struct SpotLight : public Light
	{
		float InnerCone;
		float OuterCone;
	};
}
