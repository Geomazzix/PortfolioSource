#include <pch.h>
#include <Core/ISystem.h>
#include <Core/SystemDirector.h>
Frac::ISystem::ISystem(eStage stage)
{
	m_info.stage = stage;
	EngineCore::GetInstance().GetSystemDirector().RegisterSystem(this);
}

Frac::ISystem::~ISystem()
{
	EngineCore::GetInstance().GetSystemDirector().RemoveSystem(this);
}

const Frac::SystemInfo& Frac::ISystem::GetSystemInfo() const
{
	return m_info;
}

void Frac::ISystem::SetSystemInfo(const SystemInfo& info)
{
	assert(m_info.stage == info.stage && "Stage can't be changed at runtime");
	m_info = info;
}

