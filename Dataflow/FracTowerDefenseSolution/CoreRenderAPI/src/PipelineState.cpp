#include "CoreRenderAPI/pch.h"
#include "CoreRenderAPI/PipelineState.h"
#include "CoreRenderAPI/Renderpasses/ShadowPass.h"
#include "CoreRenderAPI/Renderpasses/PBRRenderPass.h"
#include "CoreRenderAPI/Renderpasses/PostProcessingPass.h"

namespace TOR
{
	int PipelineState::s_id = 0;

	PipelineState::PipelineState(const std::string& name)
	{
		m_name = name;
		m_id = s_id++;
		m_renderPasses =
		{
			std::make_unique<ShadowPass>(ShadowPass(name + "_ShadowPass",ERenderPass::SHADOW)),
			std::make_unique<PBRRenderPass>(PBRRenderPass(name + "_PBRPass",ERenderPass::PBR)),
			std::make_unique<PostProcessingPass>(PostProcessingPass(name + "PostProcessingPass",ERenderPass::POSTPROCESSING))
		};
	}

	PipelineState::~PipelineState()
	{
		m_name = "";
		m_id = -1;
	}

	void PipelineState::SetRenderPassState(ERenderPass renderPass, bool state)
	{
		m_renderPasses[static_cast<int>(renderPass)]->SetState(state);
	}

	const std::string& PipelineState::GetName() const
	{
		return m_name;
	}

	const int PipelineState::GetId() const
	{
		return m_id;
	}

	IRenderPass& TOR::PipelineState::GetRenderPass(ERenderPass renderPass)
	{
		return *m_renderPasses[static_cast<int>(renderPass)].get();
	}
}