#include "pch.h"
#include "SceneManagement/SceneManager.h"
#include "IO/ModelDirector.h"
#include "IO/GLTFModelBuilder.h"
#include "Core/EntityRegistry.h"
#include "Core/Entity.h"
#include "IO/GLTFModelBuilder.h"
#include <tuple>

namespace Frac
{
	SceneManager::SceneManager(Renderer& a_renderSystem, EntityRegistry& a_registry, ResourceManager& a_resourceManager) :
		m_renderSystem(a_renderSystem),
		m_entityRegistry(a_registry),
		m_resourceManager(a_resourceManager),
		m_activeScene(nullptr),
		m_worldRoot(a_registry, "WorldRoot")
	{
		m_entityRegistry.AddComponent<TOR::Transform>(m_worldRoot, TOR::Transform());
	}

	Frac::Entity& SceneManager::CreateScene(const std::string& sceneName, bool setAsActiveScene)
	{
		if (m_resourceManager.IsSceneLoaded(sceneName))
		{
			LOGWARNING("This scene has already been created and will be reused.");
			if (setAsActiveScene)
			{
				m_activeScene = m_world[sceneName];
				return *m_activeScene;
			}

			return *m_world[sceneName];
		}

		Entity* sceneEntity = new Entity(m_entityRegistry, sceneName);
		TOR::Scene sceneData = m_resourceManager.LoadScene(sceneName);
		m_entityRegistry.AddComponent<TOR::Scene>(*sceneEntity, sceneData);
		m_worldRoot.AddChild(*sceneEntity);
		m_world.insert(std::make_pair(sceneName, sceneEntity));

		if (setAsActiveScene)
		{
			m_activeScene = m_world[sceneName];
			return *m_activeScene;
		}

		return *m_world[sceneName];
	}

	void SceneManager::DestroyScene(const std::string& sceneName)
	{
		FRAC_ASSERT(m_world.find(sceneName) != m_world.end(), "ASSERT: Destroy Scene, scene not existing");
		FRAC_ASSERT(m_world[sceneName] != m_activeScene, "ASSERT: Destroy Scene, scene not active");

		m_worldRoot.RemoveChild(*m_world[sceneName]);												//Removes from the hierarchy
		m_world.erase(sceneName);																	//Removes from the scene cache.
		m_entityRegistry.GetEnTTRegistry().destroy(m_world[sceneName]->GetHandle());				//Actually destroys the entities.
	}

	Frac::Entity& SceneManager::SetActiveScene(const std::string& a_filePath)
	{
		FRAC_ASSERT(m_world.find(a_filePath) != m_world.end(), "ASSERT: SetActiveScene, scene not existing");
		m_activeScene = m_world[a_filePath];
		return *m_activeScene;
	}

	Frac::Entity& SceneManager::GetActiveScene() const
	{
		FRAC_ASSERT(m_activeScene != nullptr, "ASSERT: GetActiveScene, Activescene not existing");
		return *m_activeScene;
	}

	Frac::Entity& SceneManager::GetScene(const std::string& sceneName)
	{
		FRAC_ASSERT(m_world.find(sceneName) != m_world.end(), "ASSERT: GetScene, sceneName does not exist");
		return *m_world[sceneName];
	}

	void SceneManager::LoadModelIntoMemory(EModelFileType fileExtention, const std::string& filePath)
	{
		if (!m_resourceManager.IsGltfFileLoaded(filePath))
		{
			ModelDirector director;
			switch (fileExtention)
			{
			case EModelFileType::GLTF:
			{
				GLTF_ModelBuilder builder(m_renderSystem, m_resourceManager, filePath, false);
				director.SetBuilder(&builder);
				director.Construct();
				break;
			}
			case EModelFileType::GLB:
			{
				GLTF_ModelBuilder builder(m_renderSystem, m_resourceManager, filePath, true);
				director.SetBuilder(&builder);
				director.Construct();
				break;
			}
			}
		}
	}

	void SceneManager::LoadModelOnEntity(Entity& handle, EModelFileType fileExtention, const std::string& filePath)
	{
		LoadModelIntoMemory(fileExtention, filePath);

		if (!m_entityRegistry.HasComponent<TOR::Transform>(handle))
		{
			LOGINFO("Created transform, since this wasn't already attached to the %s handle", handle.GetEntityName().c_str());
			m_entityRegistry.AddComponent<TOR::Transform>(handle, TOR::Transform());
		}

		GLTFBuildNode& rootNode = m_resourceManager.GetGltfModelNode(filePath);
		Entity& entity = *new Entity(m_entityRegistry, rootNode.Name);
		handle.AddChild(entity);
		m_entityRegistry.AddComponent<TOR::Transform>(entity, rootNode.Transform);

		LoadBuildNodeIntoScene(rootNode, entity);
	}

	void SceneManager::LoadModelOnEntityOnceLoaded(Entity& handle, EModelFileType fileExtention,
		const std::string& filePath)
	{
		if (m_loadedMap.find(filePath) != m_loadedMap.end())
		{
			LoadModelOnEntity(handle, fileExtention, filePath);
		}
		m_modelQueue.push({ handle,fileExtention,filePath });
	}

	void SceneManager::AddPreloadModel(EModelFileType fileExtention, const std::string& modelToPreload)
	{
		m_preloadModelQueue.push(std::make_pair(fileExtention, modelToPreload));
		++m_endPreloadValue;
	}

	bool SceneManager::ProcessPreloadModels()
	{
		if (!m_preloadModelQueue.empty())
		{
			std::pair<EModelFileType, std::string> modelToPreload = m_preloadModelQueue.front();
			m_loadedMap.insert(std::make_pair(modelToPreload.second, true));
			LoadModelIntoMemory(modelToPreload.first, modelToPreload.second);
			m_preloadModelQueue.pop();
			++m_progress;
			return true;
		}
		AssignAllQuededModels();
		return false;
	}

	void SceneManager::RetrievePreloadProgress(uint64_t& startPoint, uint64_t& endPoint, uint64_t& currentProgress)
	{
		startPoint = m_startPreloadValue;
		endPoint = m_endPreloadValue;
		currentProgress = m_progress;
	}

	void SceneManager::LoadBuildNodeIntoScene(GLTFBuildNode& buildNode, Entity& parent)
	{
		for (int i = 0; i < buildNode.Children.size(); i++)
		{
			GLTFBuildNode& child = buildNode.Children[i];
			Entity& childEntity = *new Entity(m_entityRegistry, child.Name);
			parent.AddChild(childEntity);
			m_entityRegistry.AddComponent<TOR::Transform>(childEntity, child.Transform);

			switch (child.Type)
			{
			case EGLTFModelNode::MESH:
			{
				if (!m_entityRegistry.HasComponent<Renderable>(childEntity))
				{
					m_entityRegistry.AddComponent<Renderable>(childEntity, Renderable());
				}

				Renderable& renderable = m_entityRegistry.GetComponent<Renderable>(childEntity);
				renderable.Mesh = reinterpret_cast<TOR::Mesh*>(buildNode.Children[i].Data);
				renderable.Material = reinterpret_cast<TOR::Material*>(buildNode.Children[++i].Data);
				break;
			}
			case EGLTFModelNode::CAMERA:
				LOGWARNING("Using camera from gtlf file, therefor ignoring it!");
				break;
			case EGLTFModelNode::SCENE:
				m_entityRegistry.AddComponent<TOR::Scene>(childEntity, *reinterpret_cast<TOR::Scene*>(child.Data));
				break;
			case EGLTFModelNode::POINTLIGHT:
				m_entityRegistry.AddComponent<TOR::PointLight>(childEntity, *reinterpret_cast<TOR::PointLight*>(child.Data));
				break;
			case EGLTFModelNode::DIRECTIONALLIGHT:
				m_entityRegistry.AddComponent<TOR::DirectionalLight>(childEntity, *reinterpret_cast<TOR::DirectionalLight*>(child.Data));
				break;
			case EGLTFModelNode::SPOTLIGHT:
				m_entityRegistry.AddComponent<TOR::SpotLight>(childEntity, *reinterpret_cast<TOR::SpotLight*>(child.Data));
				break;
			}

			LoadBuildNodeIntoScene(child, childEntity);
		}
	}

	void SceneManager::AssignAllQuededModels()
	{
		while (!m_modelQueue.empty())
		{
			ModelQueueStruct& elem{ m_modelQueue.front() };
			LoadModelOnEntity(elem.entity, elem.type, elem.path);
			m_modelQueue.pop();
		}
	}
}