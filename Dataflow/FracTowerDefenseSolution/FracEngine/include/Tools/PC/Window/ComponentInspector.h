#pragma once
#include "Tools/PC/Window/IGUIWindow.h"

namespace Frac
{
	class EntityRegistry;
	class EntityList;
	class ComponentInspector : public IGUIWindow
	{
	public:
		ComponentInspector(EntityRegistry& a_registry, EntityList& a_entityList);
		~ComponentInspector() = default;

		void Update() override;

	private:
		EntityRegistry& m_registry;
		EntityList& m_entityList;
	};

} // namespace Frac