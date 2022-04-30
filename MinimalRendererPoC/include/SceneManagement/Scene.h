#pragma once
#include <memory>
#include <vector>
#include "SceneManagement/ENodeType.h"

namespace mrpoc
{
	class Camera;
	class Model;
	class Light;
	class Node;

	class Scene
	{
	public:
		Scene();
		~Scene();

		int AddLight(std::shared_ptr<Light> aLightToAdd);
		int AddCamera(std::shared_ptr<Camera> aCameraToAdd);
		int AddModel(std::shared_ptr<Model> aModelToAdd);

		void Update();

		std::shared_ptr<Node>& GetRoot() { return m_root; }

		Light& GetLightData(int dataIndex);
		size_t GetLightDataSize();

		Camera& GetCameraData(int dataIndex);
		size_t GetCameraDataSize();

		Model& GetModelData(int dataIndex);
		size_t GetModelDataSize();

	private:

		std::shared_ptr<Node> m_root;
		std::vector<std::shared_ptr<Light>> m_lights;
		std::vector<std::shared_ptr<Camera>> m_cameras;
		std::vector<std::shared_ptr<Model>> m_models;
	};
}