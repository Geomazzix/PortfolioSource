#pragma once
#include "Graphics/RenderSystem.h"
#include "IO/ModelBuilder.h"
#include "Core/ResourceBank.h"
namespace Frac
{
	class EntityRegistry;
	class Entity;

	struct ModelTag {};

	struct Renderable
	{
		TOR::Mesh* Mesh;
		TOR::Material* Material;
	};

	/// <summary>
	/// The scene manager is responsive for the creation of entities within the scenes.
	/// </summary>
	class SceneManager
	{
	public:
		SceneManager(Renderer& a_renderSystem, EntityRegistry& a_registry, ResourceManager& a_resourceManager);
		~SceneManager() = default;

		/// <summary>
		/// Creates an empty scene root for the new scene, so the entities remain grouped within the world.
		/// </summary>
		/// <returns></returns>
		Entity& CreateScene(const std::string& sceneName, bool setAsActiveScene = true);

		/// <summary>
		/// Unloads a scene from the scene hierarchy by retrieving the root component of that entity pack and destroying it.
		/// </summary>
		/// <param name="a_filePath"></param>
		void DestroyScene(const std::string& sceneName);

		/// <summary>
		/// Sets the active scene to be updated and rendered.
		/// </summary>
		/// <param name="a_filePath"></param>
		/// <returns></returns>
		Entity& SetActiveScene(const std::string& a_filePath);
		
		/// <summary>
		/// Retrieves the currently active scene. Asserts on failure.
		/// </summary>
		/// <returns></returns>
		Entity& GetActiveScene() const;

		/// <summary>
		/// Retrieves the requested. Asserts on failure.
		/// </summary>
		/// <param name="sceneName"></param>
		/// <returns></returns>
		Entity& GetScene(const std::string& sceneName);

		void LoadModelIntoMemory(EModelFileType fileExtention, const std::string& filePath);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="handle"></param>
		/// <param name="filePath"></param>
		void LoadModelOnEntity(Entity& handle, EModelFileType fileExtention, const std::string& filePath);
		void LoadModelOnEntityOnceLoaded(Entity& handle, EModelFileType fileExtention, const std::string& filePath);
		void AddPreloadModel(EModelFileType fileExtention, const std::string& modelToPreload);
		bool ProcessPreloadModels();
		void RetrievePreloadProgress(uint64_t& startPoint, uint64_t& endPoint, uint64_t& currentProgress);


	private:
		void LoadBuildNodeIntoScene(GLTFBuildNode& buildNode, Entity& parent);
		void AssignAllQuededModels();
		Renderer& m_renderSystem;
		EntityRegistry& m_entityRegistry;
		ResourceManager& m_resourceManager;

		Entity m_worldRoot;
		Entity* m_activeScene;
		std::unordered_map<std::string, Entity*> m_world;
		std::queue<std::pair<EModelFileType,std::string>> m_preloadModelQueue;

		int m_startPreloadValue = 0,m_endPreloadValue = 0,m_progress = 0;
		struct ModelQueueStruct
		{
			Entity& entity;
			EModelFileType type;
			const std::string path;
		};
		std::queue<ModelQueueStruct> m_modelQueue;
		std::unordered_map<std::string, bool> m_loadedMap;
	};
}