#pragma once
#include "ISystem.h"

namespace Frac
{
	class ISystem;
	class SystemDirector
	{
	public:
		SystemDirector() = default;
		~SystemDirector() = default;
		void RegisterSystem(ISystem* system);
		void RemoveSystem(ISystem* system);
		void Update(float deltaTime);
		private:
		uint64_t m_ID = 0;
		std::array<std::vector<ISystem*>,static_cast<int>(eStage::STAGE_MAX)> m_systems;
	};
}


