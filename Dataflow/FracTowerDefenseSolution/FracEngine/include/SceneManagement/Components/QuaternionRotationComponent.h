/*!
 * \class Quaternion Rotation Component
 *
 * \brief A rotation component that describes rotation data using quaternion floats.
 *
 * \author Olivier
 * \date March 2021
 */
#pragma once
#include "glm/gtx/quaternion.hpp"
namespace Frac
{
	struct QuaternionRotationComponent
	{
	public:
		union
		{
			glm::quat quaternionRotationData = glm::quat{glm::vec3{0.f,0.f,0.f}};
			struct
			{
				float x, y, z, w;
			};
		};
	};
}