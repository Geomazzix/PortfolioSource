#pragma once
#include <string>
#include<glm/vec3.hpp>

struct LevelData
{
public:
	int m_levelId;
	std::string m_levelName;
	std::string m_levelHeightName;
	int m_levelTargetScore;
	int m_levelWaves;
	int m_levelStartResourceAmount;
	glm::vec3 m_cameraStartPosition;
	glm::vec3 m_cameraStartRotation;
private:
};