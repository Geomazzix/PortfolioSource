#include "CoreRenderAPI/pch.h"
#include "CoreRenderAPI/Renderpasses/PostProcessingPass.h"

namespace TOR
{

	PostProcessingPass::PostProcessingPass(const std::string& name, ERenderPass renderPass) : 
		IRenderPass(name, renderPass)
	{

	}

	void PostProcessingPass::Initialize(OnPassConstructionArgs* args)
	{
		TOR_ASSERT(args != nullptr);
		m_lowLevelRenderingCore = args->Renderer;
		//#TODO: Abstract the framebuffers and renderbuffer objects from opengl-so the initialization can be done in the correct places.
	}

	void PostProcessingPass::Destroy(OnPassDestructionArgs* args)
	{
		TOR_ASSERT(args != nullptr);
	}

	void PostProcessingPass::OnStateEnter(OnStateEnterArgs* args)
	{
		TOR_ASSERT(args != nullptr);
		m_lowLevelRenderingCore->SetupPostProcessingPass();
		m_lowLevelRenderingCore->ApplyBloomPass();
	}

	void PostProcessingPass::OnStateExit(OnStateExitArgs* args)
	{
		TOR_ASSERT(args != nullptr);
		m_lowLevelRenderingCore->EndPostProcessingPass();
	}
}