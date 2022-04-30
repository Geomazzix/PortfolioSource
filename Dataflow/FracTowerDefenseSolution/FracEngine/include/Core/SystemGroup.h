#pragma once
#include <map>

namespace Frac
{
	class ISystem;
	class SystemGroup
	{
	public:
		void AddSystem(ISystem* system);
		void RemoveSystem(ISystem* system);
		void EnableAll();
		void DisableAll();
	private:
		std::map<uint64_t,ISystem*> m_systems;
	};

}

