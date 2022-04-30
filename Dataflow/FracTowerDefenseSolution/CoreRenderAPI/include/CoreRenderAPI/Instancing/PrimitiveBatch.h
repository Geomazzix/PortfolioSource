#pragma once
#include <vector>
#include <Glm/glm.hpp>
#include "CoreRenderAPI/Instancing/PrimitiveCache.h"

namespace TOR
{
	struct Mesh;
	struct Primitive;
	struct Transform;

	struct PrimitiveBatch
	{
		PrimitiveBatch(PrimitiveShape in_Primitive) : PrimitiveToRender(in_Primitive), PrimitiveTransforms(), PrimitiveColors() {}
		PrimitiveShape PrimitiveToRender;

		// this matrix vector is used for more than only (R)otation (P)osition and (S)cale instancing:
		// | S | R | R | P
		// | R | S | R | P
		// | R | R | S | P
		// | X | Y | Z | 1
		// X Y and Z are 3 floats free to use for any special effects on particles
		// X: 0 is not emmissive, 1 is emissive
		std::vector<glm::mat4x4> PrimitiveTransforms;
		//std::vector<glm::mat3x3> PrimitiveNormalTransforms;
		std::vector<glm::vec4> PrimitiveColors;
	};
}