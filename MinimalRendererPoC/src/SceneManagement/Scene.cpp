#include "MrpocPch.h"
#include "SceneManagement/Scene.h"
#include "SceneManagement/Nodes/Model.h"
#include "SceneManagement/Nodes/Camera.h"
#include "SceneManagement/Nodes/Light.h"
#include "SceneManagement/Node.h"

namespace mrpoc
{
	Scene::Scene() : m_root(std::make_unique<Node>(Node()))
	{
		m_root->InitializeNode(ENodeType::ROOT, 0);
	}

	Scene::~Scene()
	{
	}

	int Scene::AddLight(std::shared_ptr<Light> aLightToAdd)
	{
		m_lights.push_back(aLightToAdd);
		return m_lights.size() - 1;
	}

	int Scene::AddCamera(std::shared_ptr<Camera> aCameraToAdd)
	{
		m_cameras.push_back(aCameraToAdd);
		return m_cameras.size() - 1;
	}

	int Scene::AddModel(std::shared_ptr<Model> aModelToAdd)
	{
		m_models.push_back(aModelToAdd);
		return m_models.size() - 1;
	}

	void Scene::Update()
	{
		m_root->Update();
	}

	Light& Scene::GetLightData(int dataIndex)
	{
#if DEBUG
		assert(dataIndex < m_lights.size())
#endif
			return *m_lights[dataIndex].get();
	}

	size_t Scene::GetLightDataSize()
	{
		return m_lights.size();
	}

	Camera& Scene::GetCameraData(int dataIndex)
	{
#if DEBUG
		assert(dataIndex < m_cameras.size())
#endif
			return *m_cameras[dataIndex].get();
	}

	size_t Scene::GetCameraDataSize()
	{
		return m_cameras.size();
	}

	Model& Scene::GetModelData(int dataIndex)
	{
#if DEBUG
		assert(dataIndex < m_models.size())
#endif
			return *m_models[dataIndex].get();
	}

	size_t Scene::GetModelDataSize()
	{
		return m_models.size();
	}
}