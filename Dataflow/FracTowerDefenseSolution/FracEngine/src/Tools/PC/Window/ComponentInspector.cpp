#include "pch.h"
#include "Tools/PC/Window/ComponentInspector.h"
#include "Tools/PC/Window/EntityList.h"

#include "Core/EntityRegistry.h"

/// Components
#include "CoreRenderAPI/Components/Transform.h"

#include <DearImGui\imgui.h>

namespace Frac
{
	ComponentInspector::ComponentInspector(EntityRegistry& a_registry, EntityList& a_entityList) :
		m_registry(a_registry),
		m_entityList(a_entityList)
	{

	}

	void ComponentInspector::Update() 
	{
		if (m_windowState) 
		{
			// TODO Reflection to read out known and unknown components automatically
			int imguiID = 0;
			ImGui::SetNextWindowPos(ImVec2(ImGui::GetFrameHeight() * 17.5f, ImGui::GetFrameHeight() * 2.0f), ImGuiCond_Once);
			ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 15.0f, ImGui::GetFontSize() * 20.0f), ImGuiCond_Once);
			ImGui::Begin("Component Inspector", nullptr, ImGuiWindowFlags_NoCollapse);
			if (m_entityList.GetSelectedEntity()) 
			{
				std::string name = "Entity: " + m_entityList.GetSelectedEntity()->GetEntityName();
				ImGui::Text(name.c_str());
				// Transform Component
				if (m_registry.HasComponent<TOR::Transform>(*m_entityList.GetSelectedEntity())) 
				{
					bool hasChangedValues = false;
					const TOR::Transform& transformComponent = m_registry.GetComponent<TOR::Transform>(*m_entityList.GetSelectedEntity());
					ImGui::Separator();
					ImGui::Text("Transform Component");
					ImGui::Spacing();

					TOR::Transform newTransformComponent = transformComponent;

					//Position
					ImGui::Spacing();
					ImGui::Text("Position");
					float position[3]{ transformComponent.Position.x, transformComponent.Position.y, transformComponent.Position.z };
					ImGui::PushID(imguiID++);
					if (ImGui::DragFloat3("", position)) {
						hasChangedValues = true;
						newTransformComponent.Position.x = position[0];
						newTransformComponent.Position.y = position[1];
						newTransformComponent.Position.z = position[2];
					}
					ImGui::PopID();
					
					//Scale
					ImGui::Spacing();
					ImGui::Text("Scale");
					float scale[3]{ transformComponent.Scale.x, transformComponent.Scale.y, transformComponent.Scale.z };
					ImGui::PushID(imguiID++);
					if (ImGui::DragFloat3("", scale)) {
						hasChangedValues = true;
						newTransformComponent.Scale.x = scale[0];
						newTransformComponent.Scale.y = scale[1];
						newTransformComponent.Scale.z = scale[2];
					}
					ImGui::PopID();

					//Rotation
					ImGui::Spacing();
					ImGui::Text("Rotation");
					float rotation[3]{ transformComponent.EulerOrientation.x, transformComponent.EulerOrientation.y, transformComponent.EulerOrientation.z };
					ImGui::PushID(imguiID++);
					if (ImGui::DragFloat3("", rotation, 0.1f)) {
						hasChangedValues = true;
						newTransformComponent.EulerOrientation.x = rotation[0];
						newTransformComponent.EulerOrientation.y = rotation[1];
						newTransformComponent.EulerOrientation.z = rotation[2];
					}
					newTransformComponent.Orientation = glm::quat(glm::vec3(rotation[0], rotation[1], rotation[2]));
					ImGui::PopID();

					if (hasChangedValues) {
						m_registry.GetComponent<TOR::Transform>(*m_entityList.GetSelectedEntity()) = newTransformComponent;
						m_registry.GetEnTTRegistry().patch<TOR::Transform>(m_entityList.GetSelectedEntity()->GetHandle());
					}
				} // end Transform Component
			}
			else {
				std::string name = "Entity: No entity selected";
				ImGui::Text(name.c_str());
			}
			ImGui::End(); // end Component Inspector Window
		}
	}
} // namespace Frac