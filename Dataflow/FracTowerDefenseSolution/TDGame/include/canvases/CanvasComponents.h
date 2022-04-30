#pragma once
#include "entt/entity/entity.hpp"
#include "entt/entity/fwd.hpp"

struct UIPopUp
{
	entt::entity TheEnt;
	std::unordered_map<std::string, entt::entity> InfoElements;
};

struct UIElement
{
	std::string EventName = ""; /*! <- used when this element should switch active canvas */
};

struct UICanvas
{
	std::string Name = "";

	// Settable (resource counter, etc)
	std::unordered_map<std::string, entt::entity> SettableElements;

	// Selection (menu items, buttons, etc)
	std::vector<entt::entity> SelectableElements;
	int CurSelectedElement = 0;
	int LastSelectedElement = 0;

	glm::vec3 IdleColor = glm::vec3(1.f, 0.89f, 0.97f);
	glm::vec3 HoverColor = glm::vec3(0.9f, 0.55f, 0.45f);

	float SelectorAnimTime = 0.f;
	float SelectorAnimSpeed = 2.5f;
	float SelectorAnimBounce = 1.f;
	float SelectorAnimCurrent = 0.f;

	bool IsActive = true;
};