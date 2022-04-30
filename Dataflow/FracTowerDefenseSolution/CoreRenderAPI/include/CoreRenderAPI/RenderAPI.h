#pragma once

#include "CoreRenderAPI/PipelineState.h"
#include "CoreRenderAPI/LowLevelAPI.h"
#include "CoreRenderAPI/Instancing/PrimitiveCache.h"

namespace TOR
{
	struct Material;
	struct Camera;
	struct Mesh;
	struct Light;
	struct MaterialBatch;
	struct PrimitiveBatch;

	/// <summary>
	/// The RenderAPi is used to load and manage the low-end graphics API, which also handles the graphics API specific
	/// methods.
	/// </summary>
	class RenderAPI
	{
	public:
		static RenderAPI* CreateRenderAPI(const char* dllName);

		virtual void Initialize(const std::string& assetDirectory, unsigned viewportWidth, unsigned viewportHeight);
		virtual void Shutdown();

		//Mesh methods.
		virtual void CreateMesh(Mesh& meshStructToFill);
		virtual void DestroyMesh(unsigned meshId);
		virtual void BindMeshForRecording(unsigned meshId);
		virtual void UnbindMesh(unsigned meshId);
		virtual void BufferAttributeData(unsigned meshId, EVertexAttributes& attrType, uint32_t byteLength, uint32_t ByteStride, bool normalized, int attrElementCount, const void* bufferStart, EValueTypes DataType = EValueTypes::FLOAT, EDrawTypes DrawMode = EDrawTypes::STATIC_DRAW);
		virtual void BufferElementData(unsigned meshId, uint32_t byteLength, void* bufferStart, unsigned int numberOfElements, EDrawTypes DrawMode);

		//Texture methods.
		virtual void CreateTexture(Texture& texture);
		virtual void LoadTexture(unsigned textureId, const Image& image, unsigned char* source, int sourceCountInElements, const Sampler& sampler, int format);
		virtual void DestroyTexture(unsigned textureId);
		virtual void BindTexture(unsigned textureId, unsigned int slot);
		virtual void UnbindTexture(unsigned textureId);

		virtual void AssignShaderToMesh(unsigned meshId, EShaderType type);

		void SetActiveCamera(const TOR::Camera* newCamera);
		void SetActiveCameraPosition(const glm::mat4x4& activeCameraPosition);

		void AddNewPointLight(const TOR::PointLight* newLight, glm::mat4x4 lightTransform);
		void RemovePointLight(const TOR::PointLight* oldLight);

		void AddNewDirectionalLight(const TOR::DirectionalLight* newLight, const glm::mat4x4& lightTransform);
		void RemoveDirectionalLight(const TOR::DirectionalLight* oldLight);

		void AddNewSpotLight(const TOR::SpotLight* newLight, glm::mat4x4 lightTransform);
		void RemoveSpotLight(const TOR::SpotLight* oldLight);

		//PipelineStage methods.
		TOR::PipelineState& CreatePipelineStage(const std::string& name, bool setActive = false);
		void DeletePipelineStage(const std::string& name);
		TOR::PipelineState& SetActivePipelineState(const std::string& name);
		PipelineState& GetPipelineState(const std::string& name);

		//Rendering
		virtual void ResizeViewport(unsigned viewportWidth, unsigned viewportHeight);
		virtual void Render();

		//Utility.
		SystemStats GetSystemStats();
		glm::mat4x4 tempToAvoidErrors = glm::identity<glm::mat4x4>();

		// Render API Batch accessors
		void InsertMaterialBatch(MaterialBatch newBatch);
		void InsertPrimitiveBatch(PrimitiveBatch newBatch);
		void ClearLights();

	private:
		static LowLevelAPI* s_Renderer;

		const TOR::Camera* m_activeCamera;
		glm::mat4x4 m_activeCameraWorldMatrix;

		PrimitiveCache m_primitiveCache;

		std::vector<std::pair<const TOR::PointLight*, glm::mat4x4>> m_activePointLights;
		std::vector<std::pair<const TOR::DirectionalLight*, glm::mat4x4>> m_activeDirectionalLights;
		std::vector<std::pair<const TOR::SpotLight*, glm::mat4x4>> m_activeSpotLights;

		std::forward_list<MaterialBatch> m_renderBatches;
		std::forward_list<PrimitiveBatch> m_particleBatches;

		TOR::PipelineState* m_activePipelineState = nullptr;
		std::unordered_map<std::string, std::unique_ptr<TOR::PipelineState>> m_pipelineStates;
	};
}