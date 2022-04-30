#include "CoreRenderAPI/pch.h"
#include "CoreRenderAPI/Renderpasses/ShadowPass.h"
#include "CoreRenderAPI/Renderpasses/IRenderPass.h"
#include "CoreRenderAPI/Components/Light.h"

namespace TOR
{
	ShadowPass::ShadowPass(const std::string& name, ERenderPass renderPass) : IRenderPass(name, renderPass)
	{
	}

	ShadowPass::~ShadowPass()
	{
	}

	void ShadowPass::Initialize(OnPassConstructionArgs* args)
	{
		TOR_ASSERT(args != nullptr);
		m_lowLevelRenderingCore = args->Renderer;
	}

	void ShadowPass::Destroy(OnPassDestructionArgs* args)
	{

	}

	void ShadowPass::OnStateEnter(OnStateEnterArgs* args)
	{
		TOR_ASSERT(args != nullptr);
		OnStateEnterArgsShadowPass& PassData = static_cast<OnStateEnterArgsShadowPass&>(*args);

		if (PassData.SunLight.first == nullptr)
		{
			return;
		}

		const DirectionalLight& dirLight = *PassData.SunLight.first;
		glm::mat4x4 transformOfLight = PassData.SunLight.second;

		const glm::vec4 Direction = transformOfLight[2];
		float nearPlane = glm::length(glm::vec3(transformOfLight[3])) - 60.f;
		float farPlane = glm::length(glm::vec3(transformOfLight[3])) + 50.0f; //+30 because the map is located on the 0,0,0
		float sizeOrtho = 65.f;

		glm::mat4 lightProjectionMatrix = glm::orthoRH(sizeOrtho * -1, sizeOrtho * 1, sizeOrtho * -1, sizeOrtho * 1, nearPlane, farPlane);
		glm::mat4 vp = lightProjectionMatrix * glm::inverse(transformOfLight);

		// Set in shader
		m_lowLevelRenderingCore->SetupShadowPass(vp);
		for (auto j = PassData.RenderBatches.begin(); j != PassData.RenderBatches.end(); ++j)
		{
			const MaterialBatch& batch = *j;
			for (int i = 0; i < batch.Meshes.size(); i++)
			{
				// bind VAO, Set attrib for instanced matrix
				m_lowLevelRenderingCore->BufferInstanceAttributeData(batch.Meshes[i]->Id, batch.MeshTransforms[i].size() * sizeof(glm::mat4), batch.MeshTransforms[i].data());

				// Render
				m_lowLevelRenderingCore->RenderInstanced(batch.Meshes[i]->Id, batch.MeshTransforms[i].size());
			}
		}

		//for (auto k = PassData.PrimitiveBatches.begin(); k != PassData.PrimitiveBatches.end(); ++k)
		//{
		//	const PrimitiveBatch& batch = *k;
		//	// bind VAO, Set attrib for instanced matrix
		//	const Primitive& primitiveToRender = PassData.PrimCache.GetPrimitive(batch.PrimitiveToRender);
		//	m_lowLevelRenderingCore->BufferInstanceAttributeData(primitiveToRender.data.Id, batch.PrimitiveTransforms.size() * sizeof(glm::mat4), batch.PrimitiveTransforms.data());
		//	m_lowLevelRenderingCore->BufferAnyInstancedAttributeData(primitiveToRender.data.Id, batch.PrimitiveColors.size() * sizeof(glm::vec3), batch.PrimitiveColors.data(), 6, 3);

		//	// Render
		//	m_lowLevelRenderingCore->RenderInstanced(primitiveToRender.data.Id, batch.PrimitiveTransforms.size());
		//}
	}

	void ShadowPass::OnStateExit(OnStateExitArgs* args)
	{
		//throw std::logic_error("The method or operation is not implemented.");
		m_lowLevelRenderingCore->EndShadowPass();
	}
}