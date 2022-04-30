/*!
 * \class Position Component
 *
 * \brief Component that describes the position data.
 *
 * \author Olivier
 * \date March 2021
 */
#pragma once
#include "glm/vec3.hpp"
namespace Frac
{
	struct PositionComponent
	{
	public:
		union
		{
			glm::vec3 position;
			struct
			{
				float x, y, z;
			};
		};
	};
}
