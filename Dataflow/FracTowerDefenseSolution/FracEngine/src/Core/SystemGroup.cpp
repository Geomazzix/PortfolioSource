#include "pch.h"
#include "Core/SystemGroup.h"
#include "Core/ISystem.h"

void Frac::SystemGroup::AddSystem(ISystem* system)
{
	m_systems.insert({ system->GetSystemInfo().ID,system });
}

void Frac::SystemGroup::RemoveSystem(ISystem* system)
{
	m_systems.erase( system->GetSystemInfo().ID);
}

void Frac::SystemGroup::EnableAll()
{
	for (auto && system : m_systems)
	{
		SystemInfo newInfo = system.second->GetSystemInfo();
		newInfo.enabled = true;
		system.second->SetSystemInfo(newInfo);
	}
}

void Frac::SystemGroup::DisableAll()
{
	for (auto&& system : m_systems)
	{
		SystemInfo newInfo = system.second->GetSystemInfo();
		newInfo.enabled = false;
		system.second->SetSystemInfo(newInfo);
	}
}
