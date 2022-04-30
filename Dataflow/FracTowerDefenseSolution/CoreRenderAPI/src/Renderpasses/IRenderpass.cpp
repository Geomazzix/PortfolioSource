#include "CoreRenderAPI/pch.h"
#include "CoreRenderAPI/Renderpasses/IRenderPass.h"

namespace TOR
{
	int IRenderPass::s_id = 0;

	IRenderPass::IRenderPass(const std::string& name, ERenderPass renderPass) :
		m_name(name),
		m_id(-1),
		m_renderPass(renderPass),
		m_state(true)
	{
		m_id = s_id++;
	}

	void IRenderPass::SetState(bool state)
	{
		m_state = state;
	}

	bool IRenderPass::IsEnabled() const
	{
		return m_state;
	}

	unsigned int IRenderPass::GetId() const
	{
		return m_id;
	}

	const std::string& IRenderPass::GetName() const
	{
		return m_name;
	}
}