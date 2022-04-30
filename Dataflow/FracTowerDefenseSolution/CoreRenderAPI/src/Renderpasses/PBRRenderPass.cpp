#include "CoreRenderAPI/pch.h"
#include "CoreRenderAPI/Renderpasses/PBRRenderPass.h"
#include "CoreRenderAPI/Instancing/MaterialBatch.h"

namespace TOR
{
	PBRRenderPass::PBRRenderPass(const std::string& name, TOR::ERenderPass pass) : IRenderPass(name, pass)
	{
		m_lowLevelRenderingCore = nullptr;
	}

	PBRRenderPass::~PBRRenderPass()
	{
	}

	void PBRRenderPass::OnStateEnter(OnStateEnterArgs* args)
	{
		TOR_ASSERT(args != nullptr);
		OnStateEnterArgsPBRPass& PassData = static_cast<OnStateEnterArgsPBRPass&>(*args);

		m_lowLevelRenderingCore->SetupScenePass();

		for (auto j = PassData.RenderBatches.begin(); j != PassData.RenderBatches.end(); ++j)
		{
			const MaterialBatch& batch = *j;

			// Set Material Data for each mesh within a material & Activate Shader
			// We should do this outside of the mesh loop as batches are setup per material.
			m_lowLevelRenderingCore->SetMaterialProperties(batch.Meshes[0]->Id, *batch.SurfaceMaterial);

			// Set new Camera properties
			// Should also only be done once?
			m_lowLevelRenderingCore->SetCameraProperties(batch.Meshes[0]->Id, PassData.ActiveCameraPosition, PassData.ActiveCamera);

			// Set light data

			for (auto& lightPair : PassData.DirectionalLightPairs)
			{
				m_lowLevelRenderingCore->SetDirectionalLightProperties(batch.Meshes[0]->Id, lightPair.second, *lightPair.first);
			}

			for (auto& lightPair : PassData.PointLightPairs)
			{
				m_lowLevelRenderingCore->SetPointLightProperties(batch.Meshes[0]->Id, lightPair.second, *lightPair.first);
			}

			/*for (auto& lightPair : PassData.SpotLightPairs) // NOT IMPLEMENTED
			{
			}*/

			for (int i = 0; i < batch.Meshes.size(); i++)
			{
				// bind VAO, Set attrib for instanced matrix
				m_lowLevelRenderingCore->BufferInstanceAttributeData(batch.Meshes[i]->Id, batch.MeshTransforms[i].size() * sizeof(glm::mat4), batch.MeshTransforms[i].data());
				m_lowLevelRenderingCore->BufferInstanceNormalAttributeData(batch.Meshes[i]->Id, batch.MeshNormalTransforms[i].size() * sizeof(glm::mat3), batch.MeshNormalTransforms[i].data());

				// Render
				m_lowLevelRenderingCore->RenderInstanced(batch.Meshes[i]->Id, batch.MeshTransforms[i].size());
			}
		}

		for (auto j = PassData.PrimitiveBatches.begin(); j != PassData.PrimitiveBatches.end(); ++j)
		{
			const PrimitiveBatch& batch = *j;
			const Primitive& primToRender = PassData.PrimCache.GetPrimitive(batch.PrimitiveToRender);

			m_lowLevelRenderingCore->ActivateShader(primToRender.data.Id);
			m_lowLevelRenderingCore->SetCameraProperties(primToRender.data.Id, PassData.ActiveCameraPosition, PassData.ActiveCamera);

			// Set light data
			for (auto& lightPair : PassData.DirectionalLightPairs)
			{
				m_lowLevelRenderingCore->SetDirectionalLightProperties(primToRender.data.Id, lightPair.second, *lightPair.first);
			}

			for (auto& lightPair : PassData.PointLightPairs)
			{
				m_lowLevelRenderingCore->SetPointLightProperties(primToRender.data.Id, lightPair.second, *lightPair.first);
			}

			// bind VAO, Set attrib for instanced matrix
			m_lowLevelRenderingCore->BufferInstanceAttributeData(primToRender.data.Id, batch.PrimitiveTransforms.size() * sizeof(glm::mat4), batch.PrimitiveTransforms.data());
			//m_lowLevelRenderingCore->BufferInstanceNormalAttributeData(primToRender.data.Id, batch.MeshNormalTransforms[i].size() * sizeof(glm::mat3), batch.MeshNormalTransforms[i].data());
			m_lowLevelRenderingCore->BufferAnyInstancedAttributeData(primToRender.data.Id, batch.PrimitiveColors.size() * sizeof(glm::vec4), batch.PrimitiveColors.data(), 6, 4);
			// Render
			m_lowLevelRenderingCore->RenderInstanced(primToRender.data.Id, batch.PrimitiveTransforms.size());
		}
	}

	void PBRRenderPass::OnStateExit(OnStateExitArgs* args)
	{
		TOR_ASSERT(args != nullptr);
		m_lowLevelRenderingCore->EndScenePass();
	}

	void PBRRenderPass::Initialize(OnPassConstructionArgs* args)
	{
		TOR_ASSERT(args != nullptr);
		m_lowLevelRenderingCore = args->Renderer;
	}

	void PBRRenderPass::Destroy(OnPassDestructionArgs* args)
	{
		TOR_ASSERT(args != nullptr);
	}
}