#include "TDpch.h"
#include "GameDirector.h"
#include "Core/ISystem.h"
void GameDirector::AssignSystemToStates(Frac::ISystem* system, state states, bool isEnabledAtStart)
{
	Frac::SystemInfo info = system->GetSystemInfo();
	info.enabled = isEnabledAtStart;
	system->SetSystemInfo(info);

	state flag = 1;
	for (int i = 0; i < s_maxNumberOfStates; ++i)
	{
		if (states & (flag << i))
		{
			m_states[i].AddSystem(system);
		}
	}
}

void GameDirector::RemoveSystemFromStates(Frac::ISystem* system, state states)
{
	state flag = 1;
	for (int i = 0; i < s_maxNumberOfStates; ++i)
	{
		if (states & (flag << i))
		{
			m_states[i].RemoveSystem(system);
		}
	}
}

void GameDirector::SwitchToGroup(state stateToSwitchTo)
{
	FRAC_ASSERT(stateToSwitchTo != 0, "State passed in equal to 0");
	m_previousState = m_currentState;
	m_currentState = stateToSwitchTo;
	state index{ static_cast<state>(glm::log2(static_cast<float>(stateToSwitchTo))) };
	m_states[m_currentStateIndex].DisableAll();
	m_currentStateIndex = index;
	m_states[index].EnableAll();
}

state GameDirector::GetCurrentState()
{
	return m_currentState;
}

state GameDirector::GetPreviousState()
{
	return m_previousState;
}
