#include "TDpch.h"
#include "LevelManager.h"
#include "EnemyDirectorSystem.h"
#include "PlayerManagerSystem.h"
#include "DeathAnimationSystem.h"
#include "CameraSystem.h"

LevelManager::LevelManager(Frac::EntityRegistry& registry, TileManager& tileManager, EnemyEntityCreator& enemyEntityCreator, LevelEntityCreator& levelEntityCreator, PlayerEntityCreator& playerEntityCreator, EnemyDirectorSystem& enemyDirectorSystem, PlayerManagerSystem& playerManagerSystem,DeathAnimationSystem& deathAnimationSystem) : 
	m_tileManager(tileManager), 
	m_enemyEntityCreator(enemyEntityCreator),
	m_levelEntityCreator(levelEntityCreator), 
	m_playerEntityCreator(playerEntityCreator), 
	m_enemyDirector(enemyDirectorSystem), 
	m_playerManagerSystem(playerManagerSystem),
	m_deathAnimationSystem{deathAnimationSystem},
	m_registry(registry),
	m_activeLevel(0)
{
	m_ogmoParser = std::make_unique<OgmoParser>();
}

LevelManager::~LevelManager()
{

}

void LevelManager::Init()
{
	PreLoadLevelData();
}

void LevelManager::PreLoadLevelData()
{
	m_preLoadedLevelData.clear();
	const std::string JsonString = Frac::FileIO::Read("[AllLevelData]LevelData.json");
	m_activePreloadedLevelDataFile = json::parse(JsonString);

	m_preLoadedLevelsAmount = m_activePreloadedLevelDataFile["levelData"].size();
	LOGINFO("Amount of pre loaded levels: %i", m_preLoadedLevelsAmount);

	for (unsigned int i = 0; i < m_activePreloadedLevelDataFile["levelData"].size(); i++) {
		std::shared_ptr<LevelData> newLevelData = std::make_shared<LevelData>();
		newLevelData->m_levelId = m_activePreloadedLevelDataFile["levelData"][i]["m_levelId"];
		newLevelData->m_levelName = m_activePreloadedLevelDataFile["levelData"][i]["m_levelName"];
		newLevelData->m_levelHeightName = m_activePreloadedLevelDataFile["levelData"][i]["m_levelHeightName"];
		newLevelData->m_levelTargetScore = m_activePreloadedLevelDataFile["levelData"][i]["m_levelTargetScore"];
		newLevelData->m_levelWaves = m_activePreloadedLevelDataFile["levelData"][i]["m_levelWaves"];
		newLevelData->m_levelStartResourceAmount = m_activePreloadedLevelDataFile["levelData"][i]["m_levelStartResourceAmount"];
		newLevelData->m_cameraStartPosition = glm::vec3(m_activePreloadedLevelDataFile["levelData"][i]["camStartPosition"]["x"], m_activePreloadedLevelDataFile["levelData"][i]["camStartPosition"]["y"], m_activePreloadedLevelDataFile["levelData"][i]["camStartPosition"]["z"]);
		newLevelData->m_cameraStartRotation = glm::vec3(m_activePreloadedLevelDataFile["levelData"][i]["camStartrotation"]["x"], m_activePreloadedLevelDataFile["levelData"][i]["camStartrotation"]["y"], m_activePreloadedLevelDataFile["levelData"][i]["camStartrotation"]["z"]);
		m_preLoadedLevelData.push_back(newLevelData);
	}
}

void LevelManager::LoadLevel(int level, bool preLoaded)
{
	m_enemyEntityCreator.ClearCreatedEntities();
	m_playerEntityCreator.ClearCreatedEntities();
	m_tileManager.ClearTileEntities();
	m_enemyDirector.SetCurrentWave(0);

	//m_levelEntityCreator.ClearCreatedEntities();

	if (level > m_preLoadedLevelsAmount - 1)
	{
		LOGWARNING("Tryingg to load a undefined level, no level is loaded");
		return;
	}

	m_activeLevelData = m_preLoadedLevelData[level];
	//Getting the right data out of the actual level json file using the ogmo parser.
	//setting the right selected level data in the ogmo parser
	m_ogmoParser->SetActiveLevelFile(m_activeLevelData->m_levelName);
	//Loading grid data from the selected level
	m_ogmoParser->ParseOgmoGridData();
	//Loading all the entity data from the selected level
	m_ogmoParser->ParseStaticLevelInteractables();

	//Actual data getting and setting
	m_activeLevel = level;

	//Clearing older data
	m_activeLevelGridData.clear();
	m_allActiveStaticCores.clear();
	m_allActiveStaticEnemySpawners.clear();
	m_allActiveStaticTowers.clear();
	m_deathAnimationSystem.clear();

	//Getting and setting the new data from ogmoparser parsed data
	m_activeLevelGridData = m_ogmoParser->m_ParsedLevel;
	m_tileMapSize = m_ogmoParser->m_gridSize;
	m_ogmoSpriteSize = m_ogmoParser->m_spriteSize;
	m_allActiveStaticCores = m_ogmoParser->m_allActiveStaticCores;
	m_allActiveStaticEnemySpawners = m_ogmoParser->m_allActiveStaticEnemySpawners;
	m_allActiveStaticTowers = m_ogmoParser->m_allActiveStaticTowers;

	m_playerManagerSystem.SetResources(m_activeLevelData->m_levelStartResourceAmount);

	ParseLevelHeight(m_activeLevelData->m_levelHeightName);

	//m_activeLevelData->m_levelStartResourceAmount;

	//TODO add change camera position and rotation command
	SetGameTileManagerData();
	m_levelEntityCreator.CreateStaticLevelModel(level);
	CreateAllPreLoadedLevelEntities();

	//Positioning the camera on the center of the world.
	m_registry.GetEnTTRegistry().view<TOR::Transform, TOR::Camera, CameraSystem::CameraTrait>().each([this](TOR::Transform& transform, TOR::Camera& camera, CameraSystem::CameraTrait& trait)
	{
		if (camera.IsActive)
		{
			const glm::vec3 centerTile = m_tileManager.GridToWorld(m_tileManager.GetAllCols() / 2, m_tileManager.GetAllRows() / 2);

			trait.LookAt = centerTile;

			if(!trait.Initialized)
			{
				transform.Position += centerTile;

				trait.Offset = transform.Position - trait.LookAt;

				trait.Initialized = true;
			}
			else
			{
				transform.Position = centerTile + trait.Offset;
			}
			
			trait.MinBounds = m_tileManager.GridToWorld(0, 0);
			trait.MaxBounds = m_tileManager.GridToWorld(m_tileManager.GetAllRows(), m_tileManager.GetAllCols());
			
			transform.Orientation = glm::conjugate(glm::toQuat(glm::lookAt(transform.Position, trait.LookAt, { 0.f,1.f,0.f })));
		}
	});

	m_tileManager.RunFloodFill();
	m_tileManager.RunFlowField();
}

void LevelManager::ClearLevel() 
{
	m_enemyEntityCreator.ClearCreatedEntities();
	m_playerEntityCreator.ClearCreatedEntities();
	m_tileManager.ClearTileEntities();
	m_enemyDirector.SetCurrentWave(0);
	m_levelEntityCreator.ClearCreatedEntities();
}

void LevelManager::PrintAllPreLoadedLevelData()
{
	for (unsigned int i = 0; i < m_preLoadedLevelData.size(); i++)
	{
		std::cout << m_preLoadedLevelData[i]->m_levelId << std::endl;
		std::cout << m_preLoadedLevelData[i]->m_levelName << std::endl;
		std::cout << m_preLoadedLevelData[i]->m_levelTargetScore << std::endl;
		std::cout << m_preLoadedLevelData[i]->m_levelWaves << std::endl;
		std::cout << m_preLoadedLevelData[i]->m_cameraStartPosition.x << " " << m_preLoadedLevelData[i]->m_cameraStartPosition.y << " " << m_preLoadedLevelData[i]->m_cameraStartPosition.z << std::endl;
		std::cout << m_preLoadedLevelData[i]->m_cameraStartRotation.x << " " << m_preLoadedLevelData[i]->m_cameraStartRotation.y << " " << m_preLoadedLevelData[i]->m_cameraStartRotation.z << std::endl;
		std::cout << std::endl;
	}
}

void LevelManager::PrintOgmoParsedLevelData()
{
	for (unsigned int i = 0; i < m_activeLevelGridData.size(); i++)
	{
		for (unsigned int y = 0; y < m_activeLevelGridData[i].size(); y++)
		{
			std::cout << m_activeLevelGridData[i][y];
		}
		std::cout << std::endl;
	}
}

void LevelManager::SetGameTileManagerData()
{
	m_tileManager.GenerateGridv2((int)m_tileMapSize.x, (int)m_tileMapSize.y, &m_activeLevelGridData, &m_activeLevelTileHeightData, 1.0f);
}

void LevelManager::CreateAllPreLoadedLevelEntities()
{
	//This will be used to create and add all the actual antities using the resource managers
	for (unsigned int i = 0; i < m_allActiveStaticEnemySpawners.size(); i++)
	{
		const glm::ivec2& pos = glm::ivec2((m_allActiveStaticEnemySpawners[i].m_position.x /= m_ogmoSpriteSize), (m_allActiveStaticEnemySpawners[i].m_position.z /= m_ogmoSpriteSize));
		m_enemyEntityCreator.CreateSpawner(m_allActiveStaticEnemySpawners[i].m_objectId, pos , glm::vec3(0, 0, 0), m_allActiveStaticEnemySpawners[i].m_enemySpawnType, m_allActiveStaticEnemySpawners[i].m_enemySpawnDelay);
	}
	for (unsigned int i = 0; i < m_allActiveStaticCores.size(); i++)
	{
		const glm::ivec2& pos = glm::ivec2((m_allActiveStaticCores[i].m_position.x /= m_ogmoSpriteSize), (m_allActiveStaticCores[i].m_position.z /= m_ogmoSpriteSize));
		m_playerEntityCreator.CreatePlayerBase(m_allActiveStaticCores[i].m_objectId, pos , glm::vec3(0, 0, 0), m_allActiveStaticCores[i].m_coreHealth, m_allActiveStaticCores[i].m_coreTargetData);
	}
	//This is used only for static turrets. Uncomment if we need to bring them back.
	/*for (unsigned int i = 0; i < m_allActiveStaticTowers.size(); i++)
	{
		glm::ivec2 gridPos = glm::vec2(m_allActiveStaticTowers[i].m_position.x / m_ogmoSpriteSize, m_allActiveStaticTowers[i].m_position.z / m_ogmoSpriteSize);
		m_playerEntityCreator.CreateTower(m_allActiveStaticTowers[i].m_objectId, gridPos, glm::vec3(0, 0, 0), m_allActiveStaticTowers[i].m_fireRate, m_allActiveStaticTowers[i].m_bulletType);
	}*/
}

void LevelManager::ParseLevelHeight(std::string levelHeightFile)
{
	const std::string JsonString = Frac::FileIO::Read("[Levels]" + levelHeightFile);
	m_activeLevelHeightFile = json::parse(JsonString);
	m_activeLevelTileHeightData.clear();
	m_activeLevelTileHeightData.resize(m_ogmoParser->m_gridSize.x * m_ogmoParser->m_gridSize.y);

	for (unsigned int i = 0; i < m_activeLevelHeightFile["points"].size(); i++)
	{
		m_activeLevelTileHeightData[i] = (m_activeLevelHeightFile["points"][i]["pos"][1]);
	}
	
}

const int LevelManager::GetAmountOfLevels() 
{
	return m_preLoadedLevelsAmount;
}
