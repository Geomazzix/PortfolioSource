#pragma once
#include "Tools/PC/Window/IGUIWindow.h"

namespace Frac
{
	class EntityRegistry;
	class Entity;
	class SceneManager;

	class EntityList : public IGUIWindow
	{
	public:
		EntityList(EntityRegistry& a_registry, SceneManager& a_scenemanager, const float& a_guiScale);
		~EntityList() = default;

		void Update() override;
		Entity* GetSelectedEntity() const;

	private: 
		void CheckForChildren(Entity& a_entity);
		void AddEntityToList(Entity& a_entity);

		EntityRegistry& m_registry;
		SceneManager& m_scenemanager;

		int m_entityCount;
		int m_selectedItem;
		bool m_displayInList;
		float m_widgetXOffset;
		float m_layerXOffset;
		float m_offsetMultiplier;
		Entity* m_selectedEntity;

		const float& m_guiScale;

		unsigned int m_childLayer;
	};
} // namespace Frac