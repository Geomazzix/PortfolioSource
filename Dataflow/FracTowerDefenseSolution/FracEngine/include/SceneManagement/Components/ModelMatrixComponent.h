/*!
 * \struct Model Matrix Component
 *
 * \brief Component that is used to store rotation values.
 *
 * \author Olivier
 * \date March 2021
 */

#pragma once
#include "Glm/mat4x4.hpp"
namespace Frac
{
	struct ModelMatrixComponent
	{
	public:
		union
		{
			glm::mat4x4 modelTransformData;
			struct
			{
				glm::vec4 xColumn;
				glm::vec4 yColumn;
				glm::vec4 zColumn;
				glm::vec4 wColumn;
			};
		};
	};
}