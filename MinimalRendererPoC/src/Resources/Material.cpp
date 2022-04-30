#include "MrpocPch.h"
#include <Resources/Material.h>

namespace mrpoc
{
	Material::Material() :
		AlbedoFactor(glm::vec4(0.5f, 0.5, 0.5f, 1.0f)),
		MetallicFactor(1.0f),
		RoughnessFactor(1.0f),
		AlphaMode(""),
		AlphaCutoff(0.0f),
		DoubleSided(false),
		HasOcclusionMap(false)
	{}
}