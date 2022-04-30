#include "pch.h"
#include "Tools/PC/Window/EntityList.h"
#include "CoreRenderAPI/Components/Transform.h"
#include "Core/EntityRegistry.h"
#include "SceneManagement/SceneManager.h"

#include <DearImGui\imgui.h>


namespace Frac
{
	EntityList::EntityList(EntityRegistry& a_registry, SceneManager& a_scenemanager, const float& a_guiScale) :
		m_registry(a_registry),
		m_entityCount(0),
		m_selectedItem(0),
		m_widgetXOffset(5.0f),
		m_layerXOffset(15.0f),
		m_offsetMultiplier(6.0f),
		m_selectedEntity(nullptr),
		m_scenemanager(a_scenemanager),
		m_childLayer(0),
		m_displayInList(true),
		m_guiScale(a_guiScale)
	{ }

	void EntityList::Update()
	{
		if (m_windowState)
		{
			//Begin
			{
				ImGui::SetNextWindowPos(ImVec2(ImGui::GetFrameHeight() * 1.0f, ImGui::GetFrameHeight() * 2.0f), ImGuiCond_Once);
				ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 20.0f, ImGui::GetFontSize() * 40.0f), ImGuiCond_Once);
				ImGui::Begin("Entity List", nullptr, ImGuiWindowFlags_NoCollapse);
				m_entityCount = 0;
			}

			m_displayInList = true;
			//Start checking for children at the parent
			CheckForChildren(m_scenemanager.GetActiveScene());

			//End
			{
				ImGui::End(); // end Entity List Window
			}
		}
	}

	Entity* EntityList::GetSelectedEntity() const
	{
		return m_selectedEntity;
	}

	//Recursive function to check all chrilden of a parent
	void EntityList::CheckForChildren(Entity& a_entity)
	{
		if (m_displayInList) {
			AddEntityToList(a_entity);

			//Recursive exit
			if (a_entity.GetReferenceToChildren().size() <= 0) {
				return;
			}
			else {
				m_childLayer++;
			}

			//Recursively check all childs from this entity
			for each (auto child in a_entity.GetReferenceToChildren())
			{
				CheckForChildren(*child);
			}

			if (m_childLayer > 0)
			{
				m_childLayer--;
				m_displayInList = true;
			}
		}
	}

	void EntityList::AddEntityToList(Entity& a_entity)
	{
		if (m_displayInList) {
			int imguiID = 0;
			m_entityCount++;
			std::string itemid = "##" + std::to_string(m_entityCount);
			ImGui::PushID(imguiID++);
			ImVec2 currentCursorPos = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2(currentCursorPos.x + (m_widgetXOffset * m_offsetMultiplier +  m_childLayer * m_layerXOffset) * m_guiScale, currentCursorPos.y));
			if (ImGui::Selectable(itemid.c_str(), m_entityCount == m_selectedItem)) 
			{
				m_selectedItem = m_entityCount;
				m_selectedEntity = &a_entity;
			}
			ImGui::SameLine();
			if (a_entity.GetReferenceToChildren().size() > 0) 
			{
				ImGui::SetCursorPos(ImVec2(currentCursorPos.x + (m_widgetXOffset + m_childLayer * m_layerXOffset) * m_guiScale, currentCursorPos.y));
				ImGui::SetNextItemOpen(true, ImGuiCond_Once);
				if (ImGui::TreeNode(a_entity.GetEntityName().c_str()))
				{
					m_displayInList = true;
					ImGui::TreePop();
				}
				else 
				{
					m_displayInList = false;
				}
			}
			else 
			{
				ImGui::SetCursorPos(ImVec2(currentCursorPos.x + (m_widgetXOffset * m_offsetMultiplier + m_childLayer * m_layerXOffset) * m_guiScale, currentCursorPos.y));
				ImGui::Text(a_entity.GetEntityName().c_str());
			}
			
			ImGui::PopID();
		}
	}
} // namespace Frac