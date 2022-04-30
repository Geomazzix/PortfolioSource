#include "CoreRenderAPI/pch.h"
#include "CoreRenderAPI/RenderAPI.h"
#include "CoreRenderAPI/Instancing/MaterialBatch.h"
#include "CoreRenderAPI/Renderpasses/ShadowPass.h"
#include "CoreRenderAPI/Renderpasses/PBRRenderPass.h"
#include "CoreRenderAPI/Renderpasses/PostProcessingPass.h"
#include "CoreRenderAPI/Instancing/PrimitiveBatch.h"

namespace TOR
{
	LowLevelAPI* RenderAPI::s_Renderer = nullptr;
	static RenderAPI HighLevelRenderAPI;

	/// <summary>
	/// If internal rendering system is not made yet, make new Renderer, init low level API from dll.
	/// </summary>
	/// <param name="dllName">Name of dll file that contains low level rendering. Use "OpenGLRenderer" for now.</param>
	/// <returns> pointer to high level API</returns>
	RenderAPI* RenderAPI::CreateRenderAPI(const char* dllName)
	{
		if (!s_Renderer)
		{
			s_Renderer = LowLevelAPI::CreateCoreAPI(dllName);
		}
		return &HighLevelRenderAPI;
	}

	void RenderAPI::Initialize(const std::string& assetDirectory, unsigned viewportWidth, unsigned viewportHeight)
	{
		s_Renderer->Init(assetDirectory.c_str(), viewportWidth, viewportHeight);
		m_primitiveCache.Initialize(s_Renderer);
	}

	void RenderAPI::Shutdown()
	{
		s_Renderer->Shutdown();
	}

	void RenderAPI::CreateMesh(Mesh& meshStructToFill)
	{
		s_Renderer->CreateMesh(meshStructToFill);
	}

	void RenderAPI::DestroyMesh(unsigned meshId)
	{
		s_Renderer->DestroyMesh(meshId);
	}

	void RenderAPI::BindMeshForRecording(unsigned meshId)
	{
		s_Renderer->BindMeshForRecording(meshId);
	}

	void RenderAPI::UnbindMesh(unsigned meshId)
	{
		s_Renderer->UnbindMesh(meshId);
	}

	void RenderAPI::BufferAttributeData(unsigned meshId, EVertexAttributes& attrType, uint32_t byteLength, uint32_t ByteStride, bool normalized, int attrElementCount, const void* bufferStart, EValueTypes DataType, EDrawTypes DrawMode)
	{
		s_Renderer->BufferAttributeData(meshId, attrType, byteLength, ByteStride, normalized, attrElementCount, bufferStart, -1, DataType, DrawMode);
	}

	void RenderAPI::BufferElementData(unsigned meshId, uint32_t byteLength, void* bufferStart, unsigned int numberOfElements, EDrawTypes DrawMode)
	{
		s_Renderer->BufferElementData(meshId, byteLength, bufferStart, numberOfElements, DrawMode);
	}

	void RenderAPI::CreateTexture(Texture& texture)
	{
		s_Renderer->CreateTexture(texture);
	}

	void RenderAPI::LoadTexture(unsigned textureId, const Image& image, unsigned char* source, int sourceCountInElements, const Sampler& sampler, int format)
	{
		s_Renderer->LoadTexture(textureId, image, source, sourceCountInElements, sampler, format);
	}

	void RenderAPI::DestroyTexture(unsigned textureId)
	{
		s_Renderer->DestroyTexture(textureId);
	}

	void RenderAPI::BindTexture(unsigned textureId, unsigned int slot)
	{
		s_Renderer->BindTexture(textureId, slot);
	}

	void RenderAPI::UnbindTexture(unsigned textureId)
	{
		s_Renderer->UnbindTexture(textureId);
	}

	void RenderAPI::SetActiveCamera(const TOR::Camera* newCamera)
	{
		m_activeCamera = newCamera;
	}

	void RenderAPI::SetActiveCameraPosition(const glm::mat4x4& activeCameraPosition)
	{
		m_activeCameraWorldMatrix = activeCameraPosition;
	}

	void RenderAPI::AddNewPointLight(const TOR::PointLight* newLight, glm::mat4x4 lightTransform)
	{
		m_activePointLights.emplace_back(std::make_pair(newLight, lightTransform));
	}

	void RenderAPI::RemovePointLight(const TOR::PointLight* oldLight)
	{
		TOR_ASSERT(false); // TODO
	}

	void RenderAPI::AddNewDirectionalLight(const TOR::DirectionalLight* newLight, const glm::mat4x4& lightTransform)
	{
		m_activeDirectionalLights.emplace_back(std::make_pair(newLight, lightTransform));
	}

	void RenderAPI::RemoveDirectionalLight(const TOR::DirectionalLight* oldLight)
	{
		TOR_ASSERT(false); // TODO
	}

	void RenderAPI::AddNewSpotLight(const TOR::SpotLight* newLight, glm::mat4x4 lightTransform)
	{
		m_activeSpotLights.emplace_back(std::make_pair(newLight, lightTransform));
	}

	void RenderAPI::RemoveSpotLight(const TOR::SpotLight* oldLight)
	{
		TOR_ASSERT(false); // TODO
	}

	void RenderAPI::AssignShaderToMesh(unsigned meshId, EShaderType type)
	{
		s_Renderer->AssignShaderToMesh(meshId, type);
	}

	void RenderAPI::ResizeViewport(unsigned viewportWidth, unsigned viewportHeight)
	{
		s_Renderer->ResizeViewport(viewportWidth, viewportHeight);
	}

	void RenderAPI::Render()
	{
		// Shadowpass
		{
			if (!m_activeDirectionalLights.empty())
			{
				OnStateEnterArgsShadowPass args(m_activeDirectionalLights[0], m_renderBatches, m_particleBatches, m_primitiveCache);
				m_activePipelineState->GetRenderPass(ERenderPass::SHADOW).OnStateEnter(&args);
				OnStateExitArgs exitArgs;
				m_activePipelineState->GetRenderPass(ERenderPass::SHADOW).OnStateExit(&exitArgs);
			}
		}

		// PBRpass
		{
			OnStateEnterArgsPBRPass args(m_activeDirectionalLights, m_activePointLights, m_activeSpotLights, *m_activeCamera, m_activeCameraWorldMatrix, m_renderBatches, m_particleBatches, m_primitiveCache);
			m_activePipelineState->GetRenderPass(ERenderPass::PBR).OnStateEnter(&args);
			OnStateExitArgsPBRPass exitArgs;
			m_activePipelineState->GetRenderPass(ERenderPass::PBR).OnStateExit(&exitArgs);
		}

		// Post processing pass
		{
			OnStateEnterArgsPostProcessingPass args;
			m_activePipelineState->GetRenderPass(ERenderPass::POSTPROCESSING).OnStateEnter(&args);
			OnStateExitArgsPostProcessingPass exitArgs;
			m_activePipelineState->GetRenderPass(ERenderPass::POSTPROCESSING).OnStateExit(&exitArgs);
		}

		s_Renderer->Present();
		m_renderBatches.clear();
		m_particleBatches.clear();
	}

	TOR::PipelineState& RenderAPI::CreatePipelineStage(const std::string& name, bool setActive /*= false*/)
	{
		TOR_ASSERT(m_pipelineStates.find(name) == m_pipelineStates.end());
		m_pipelineStates.emplace(name, std::make_unique<TOR::PipelineState>(name));

		if (setActive)
		{
			SetActivePipelineState(name);
		}

		return *m_pipelineStates[name].get();
	}

	void RenderAPI::DeletePipelineStage(const std::string& name)
	{
		TOR_ASSERT(m_pipelineStates.find(name) != m_pipelineStates.end());
		m_pipelineStates.erase(name);
	}

	TOR::PipelineState& RenderAPI::SetActivePipelineState(const std::string& name)
	{
		TOR_ASSERT(m_pipelineStates.find(name) != m_pipelineStates.end());
		m_activePipelineState = m_pipelineStates[name].get();
		if (!m_activePipelineState->GetInitialized())
		{
			// initialize render passes

			// init Shadow pass if set
			{
				OnConstructionArgsShadowPass constructionArgs;
				constructionArgs.Renderer = s_Renderer;
				m_activePipelineState->GetRenderPass(ERenderPass::SHADOW).Initialize(&constructionArgs);
			}
			// init PBR pass if set
			{
				OnConstructionArgsPBRPass constructionArgs;
				constructionArgs.Renderer = s_Renderer;
				m_activePipelineState->GetRenderPass(ERenderPass::PBR).Initialize(&constructionArgs);
			}

			// init Post processing pass if set
			{
				OnConstructionArgsPostProcessingPass constructionArgs;
				constructionArgs.Renderer = s_Renderer;
				m_activePipelineState->GetRenderPass(ERenderPass::POSTPROCESSING).Initialize(&constructionArgs);
			}
			m_activePipelineState->SetInitialized(true);
		}
		return *m_activePipelineState;
	}

	TOR::PipelineState& RenderAPI::GetPipelineState(const std::string& name)
	{
		TOR_ASSERT(m_pipelineStates.size() <= 0);
		TOR_ASSERT(m_pipelineStates.find(name) == m_pipelineStates.end());
		return *m_pipelineStates[name].get();
	}

	TOR::SystemStats RenderAPI::GetSystemStats()
	{
		return TOR::SystemStats{};
	}

	void RenderAPI::InsertMaterialBatch(MaterialBatch newBatch)
	{
		m_renderBatches.push_front(newBatch);
	}

	void RenderAPI::ClearLights()
	{
		m_activePointLights.clear();
		m_activeDirectionalLights.clear();
		m_activeSpotLights.clear();
	}

	void RenderAPI::InsertPrimitiveBatch(PrimitiveBatch newBatch)
	{
		m_particleBatches.push_front(newBatch);
	}
}