#pragma once
#include "Renderpasses/IRenderPass.h"

namespace TOR
{
	/// <summary>
	/// The pipeline state represents a minimal rendering pipeline with configurable render stages.
	/// </summary>
	class PipelineState
	{
	public:
		PipelineState(const std::string& name);
		~PipelineState();

		PipelineState(const PipelineState&) = delete;				// non construction-copyable
		PipelineState& operator=(const PipelineState&) = delete;	// non copyable

		void SetRenderPassState(ERenderPass pass, bool state);
		IRenderPass& GetRenderPass(ERenderPass renderPass);

		const std::string& GetName() const;
		const int GetId() const;

		bool GetInitialized() const { return m_initialized; }
		void SetInitialized(bool valueToSet) { m_initialized = valueToSet; }
	private:
		bool m_initialized = false;
		std::string m_name;
		int m_id;
		static int s_id;
		std::array<std::unique_ptr<IRenderPass>, static_cast<int>(ERenderPass::COUNT)> m_renderPasses;
	};
}