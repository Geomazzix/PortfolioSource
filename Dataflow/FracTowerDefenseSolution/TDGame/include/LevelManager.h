#pragma once
#include <vector>
#include "OgmoParser.h"
#include "LevelData.h"
#include "StaticLevelObjects.h"
#include "TileManager.h"
#include "EnemyEntityCreator.h"
#include "PlayerEntityCreator.h"
#include "LevelEntityCreator.h"

#include "Nlohmann/json.hpp"
class DeathAnimationSystem;
class EnemyDirectorSystem;
class PlayerManagerSystem; 
using json = nlohmann::json;

class LevelManager
{
public:
	LevelManager(Frac::EntityRegistry& registry, TileManager& tileManager, EnemyEntityCreator& enemyEntityCreator, LevelEntityCreator& levelEntityCreator, PlayerEntityCreator& playerEntityCreator, EnemyDirectorSystem& enemyDirectorSystem, PlayerManagerSystem& playerManagerSystem,DeathAnimationSystem& deathAnimationSystem);
	~LevelManager();
	void Init();
	void PreLoadLevelData();
	void LoadLevel(int level, bool preLoaded = false);
	void ClearLevel();

	void PrintAllPreLoadedLevelData();
	void PrintOgmoParsedLevelData();
	void SetGameTileManagerData();
	void CreateAllPreLoadedLevelEntities();
	void ParseLevelHeight(std::string levelHeightFile);
	const int GetAmountOfLevels();
	int m_activeLevel;
private:
	std::vector<std::vector<int>> m_activeLevelGridData;
	std::vector<float>m_activeLevelTileHeightData;
	glm::vec2 m_tileMapSize;
	std::shared_ptr<LevelData> m_activeLevelData;	
	int m_ogmoSpriteSize;

	std::unique_ptr<OgmoParser> m_ogmoParser;

	TileManager& m_tileManager;
	LevelEntityCreator& m_levelEntityCreator;
	EnemyEntityCreator& m_enemyEntityCreator;
	PlayerEntityCreator& m_playerEntityCreator;
	EnemyDirectorSystem& m_enemyDirector;
	PlayerManagerSystem& m_playerManagerSystem;
	DeathAnimationSystem& m_deathAnimationSystem;
	Frac::EntityRegistry& m_registry;

	std::vector<std::shared_ptr<LevelData>> m_preLoadedLevelData;

	std::vector<StaticLevelEnemySpawner>m_allActiveStaticEnemySpawners;
	std::vector<StaticLevelCore>m_allActiveStaticCores;
	std::vector<StaticLevelTower>m_allActiveStaticTowers;
	
	int m_preLoadedLevelsAmount;

	json m_activePreloadedLevelDataFile;
	json m_activeLevelHeightFile;
};
