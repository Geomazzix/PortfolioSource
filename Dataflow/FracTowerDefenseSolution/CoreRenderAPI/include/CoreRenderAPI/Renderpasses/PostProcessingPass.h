#pragma once

#include "CoreRenderAPI/Renderpasses/IRenderPass.h"

namespace TOR
{
	struct OnStateEnterArgsPostProcessingPass : OnStateEnterArgs
	{
	};

	struct OnStateExitArgsPostProcessingPass : OnStateExitArgs
	{
	};

	struct OnConstructionArgsPostProcessingPass : OnPassConstructionArgs
	{
	};

	struct OnDestructionArgsPostProcessingPass : OnPassDestructionArgs
	{
	};

	class PostProcessingPass : public IRenderPass
	{
	public:
		PostProcessingPass(const std::string& name, ERenderPass renderPass);
		~PostProcessingPass() = default;

		void Initialize(OnPassConstructionArgs* args) override;

		void Destroy(OnPassDestructionArgs* args) override;

		void OnStateEnter(OnStateEnterArgs* args) override;

		void OnStateExit(OnStateExitArgs* args) override;

	private:
		LowLevelAPI* m_lowLevelRenderingCore;
	};
}