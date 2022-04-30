#pragma once
#include <vector>
#include <Glm/glm.hpp>

namespace TOR
{
	struct Material;
	struct Mesh;
	struct Transform;

	struct MaterialBatch
	{
		MaterialBatch(Material* mat) : SurfaceMaterial(mat), Meshes(), MeshTransforms() {}
		Material* SurfaceMaterial;

		std::vector<Mesh*> Meshes;
		std::vector<std::vector<glm::mat4x4>> MeshTransforms;
		std::vector<std::vector<glm::mat3x3>> MeshNormalTransforms;
	};
}