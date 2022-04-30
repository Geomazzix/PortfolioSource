#include <pch.h>
#include <Core/SystemDirector.h>
#include <Core/ISystem.h>

namespace Frac
{
	void SystemDirector::RegisterSystem(ISystem* system)
	{
		SystemInfo newInfo = system->GetSystemInfo();
		newInfo.enabled = true;
		newInfo.ID = m_ID;

		m_systems[static_cast<int>(newInfo.stage)].push_back(system);
		system->SetSystemInfo(newInfo);
		++m_ID;
	}

	void SystemDirector::RemoveSystem(ISystem* system)
	{
		int index = (int)system->GetSystemInfo().stage;
		if (!m_systems[index].empty())
		{
			auto pos = std::find(m_systems[index].begin(), m_systems[index].end(), system);
			if (pos != m_systems[index].end())
			{
				m_systems[index].erase(pos);
			}
		}
	}

	void SystemDirector::Update(float deltaTime)
	{
		for (int stage = 0; stage < static_cast<int>(eStage::STAGE_MAX); ++stage)
		{
			for (int i = 0; i < m_systems[stage].size(); ++i)
			{
				if(m_systems[stage][i]->GetSystemInfo().enabled)
					m_systems[stage][i]->Update(deltaTime);
			}
		}
	}
}
