#pragma once
#include <string>
#include<glm/vec3.hpp>
struct StaticLevelInteractable
{
public:
	int m_objectId;
	std::string m_name;
	glm::vec3 m_position;
	glm::vec3 m_rotation;
private:
};