#include "MrpocPch.h"
#include "SceneManagement/Nodes/Model.h"

namespace mrpoc
{

	Model::Model()
	{

	}

	Model::~Model()
	{

	}

	void Model::AddMesh(std::shared_ptr<Mesh>& meshToAdd)
	{
		m_meshes.push_back(meshToAdd);
	}

}