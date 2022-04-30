/*!
 * \struct Scale Component
 *
 * \brief Describes the data inside a scale component.
 *
 * \author Olivier
 * \date March 2021
 */
#pragma once
#include "glm/vec3.hpp"
namespace Frac
{
	struct ScaleComponent
	{
	public:
		union
		{
			glm::vec3 scale;
			struct
			{
				float x, y, z;
			};
		};
	};
}