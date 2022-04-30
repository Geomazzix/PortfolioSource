/*!
 * \struct Euler Rotation Component
 *
 * \brief Component that is used to store rotation values.
 *
 * \author Olivier
 * \date March 2021
 */

#pragma once
#include "glm/vec3.hpp"
namespace Frac
{
	struct EulerRotationComponent
	{
	public:
		union
		{
			glm::vec3 eulerRotationData;
			struct
			{
				float x, y, z;
			};
		};
	};
}