#pragma once

#include <memory>
#include <queue>
#include <unordered_map>
#include <glm/glm.hpp>
#include "SceneManagement/Nodes/Light.h"
#include "EventSystem/EventArgs.h"
#include "Resources/Mesh.h"
#include "Resources/Shader.h"

namespace mrpoc 
{	
	class Model;
	class Camera;
	class Scene;
	class Node;
	class ResourceHandler;
	class Shader;

	struct ModelRenderData 
	{
		glm::mat4x4 worldMatrix; // also known as model matrix
		Model& modelData;
	};

	struct MeshRenderData
	{
		glm::mat4x4 worldMatrix;
		Mesh& mesh;
	};

	struct LightData
	{
		glm::mat4x4 worldMatrix;
		Light& lightData;
	};

	struct CameraData
	{
		glm::mat4x4 worldMatrix;
		Camera& cameraData;
	};

	constexpr int amountOfFrameBuffers = 1;

	enum class FrameBufferType {
		ScenePassBuffer = 0
	};

	class Renderer 
	{
	public:
		Renderer(ResourceHandler& resourceHandler);
		~Renderer();

		void SetActiveCameraIndex(int aNewPrimaryCameraIndex);
		void Render(Scene& aSceneToRender);

		int GetActiveCameraIndex() const;

		void ParseNode(
			std::shared_ptr<Node>& aNodeToParse,
			Scene& scene,
			std::unordered_map<Material*, std::queue<MeshRenderData>>& meshesToParse,
			std::vector<LightData>& lightDataVector,
			std::vector<CameraData>& cameraDataVector);

	private:
		void RenderScenePass(Scene& sceneToRender);
		void RenderFinalResultPass();

		void SetupFrameBuffers(int ScreenWidth,int ScreenHeight);
		void OnResize(ResizeEventArgs& e);
		void SetupScreenSpaceQuadVAO();

		void SetLightingProperties(std::vector<LightData>& lightDataVector, Shader& shader, Material& material);
		void SetMaterialProperties(Shader& shader, Material& material);

		int m_activeCameraIndex;

		unsigned int m_frameBuffers[amountOfFrameBuffers];
		unsigned int m_colorTextures[amountOfFrameBuffers];
		unsigned int m_renderBuffers[amountOfFrameBuffers];
		unsigned int m_screenQuadVAO, m_screenQuadVBO;

		std::shared_ptr<Shader> m_activeShader = nullptr;
		std::weak_ptr<Shader> m_screenShader;
	};
	
}