#pragma once

#include <memory>
#include <vector>

namespace mrpoc
{

	class Mesh;

	/// <summary>
	/// Represents a collection of data required to render a model.
	/// Based on a node in the scene graph hierarchy.
	/// </summary>
	class Model
	{
	public:
		Model();
		~Model();

		std::vector<std::shared_ptr<Mesh>>& GetMeshes() {
			return m_meshes;
		}

		void AddMesh(std::shared_ptr<Mesh>& meshToAdd);

	private:

		std::vector<std::shared_ptr<Mesh>> m_meshes;
	};
}