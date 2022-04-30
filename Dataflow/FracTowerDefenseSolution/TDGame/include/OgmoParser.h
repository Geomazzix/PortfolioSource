#pragma once
#include "Nlohmann/json.hpp"
#include "StaticLevelObjects.h"
using json = nlohmann::json;
class OgmoParser
{
public:
	OgmoParser();
	~OgmoParser();

	void SetActiveLevelFile(std::string filepath);
	void ParseOgmoGridData();
	void ParseStaticLevelInteractables();

	std::vector<std::vector<int>> m_ParsedLevel;

	std::vector<StaticLevelEnemySpawner>m_allActiveStaticEnemySpawners;
	std::vector<StaticLevelCore>m_allActiveStaticCores;
	std::vector<StaticLevelTower>m_allActiveStaticTowers;
	int m_spriteSize;
	glm::ivec2 m_gridSize;

	json m_ActiveJsonFile;
private:
	
};

