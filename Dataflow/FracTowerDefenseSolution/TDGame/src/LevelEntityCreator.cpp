#include "TDpch.h"
#include "LevelEntityCreator.h"


LevelEntityCreator::LevelEntityCreator(TileManager& tileManager, Frac::EntityRegistry& entityReg, Frac::SceneManager& sceneManager) : IEntityCreator(tileManager, entityReg, sceneManager)
{
	LevelParent = new Frac::Entity(m_entityRegistry, "LevelObjectsParent");
	m_entityRegistry.AddComponent<TOR::Transform>(*LevelParent, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
	LevelParent->SetParent(m_sceneEntity);
}

LevelEntityCreator::~LevelEntityCreator()
{

}

void LevelEntityCreator::CreateStaticLevelModel(int level)
{

	//TODO see if removing active level entity is needed here, when getting to reloading different levels
	if(m_ActiveLevel != nullptr){
		//Deloading level first
		m_entityRegistry.GetEnTTRegistry().destroy(m_ActiveLevel->GetHandle());
		m_ActiveLevel = nullptr;
	}
	
	Frac::Entity* levelEntity = new Frac::Entity(m_entityRegistry, "Level " + std::to_string(level));
	m_entityRegistry.AddComponent<TOR::Transform>(*levelEntity, TOR::Transform{ {0.0f,0.0f,0.0f},{1.0f, 1.0f, 1.0f} });
	levelEntity->SetParent(m_sceneEntity);
	m_ActiveLevel = levelEntity;

	switch (level)
	{
	case 0: {
		LOGINFO("Loading level 0 main asset");
		m_sceneManager.LoadModelOnEntityOnceLoaded(*m_ActiveLevel, Frac::EModelFileType::GLTF, "Models/NewLevels/Leve01.gltf");
	}break;
	case 1: {
		m_sceneManager.LoadModelOnEntityOnceLoaded(*m_ActiveLevel, Frac::EModelFileType::GLTF, "Models/NewLevels/Level1.gltf");
		LOGINFO("Loading level 1 main asset");
	}break;
	case 2: {
		m_sceneManager.LoadModelOnEntityOnceLoaded(*m_ActiveLevel, Frac::EModelFileType::GLTF, "Models/NewLevels/Level2.gltf");
		LOGINFO("Loading level 2 main asset");
	}break;
	case 3: {
		m_sceneManager.LoadModelOnEntityOnceLoaded(*m_ActiveLevel, Frac::EModelFileType::GLTF, "Models/NewLevels/Level3.gltf");
		LOGINFO("Loading level 3 main asset");
	}break;
	case 4: {
		m_sceneManager.LoadModelOnEntityOnceLoaded(*m_ActiveLevel, Frac::EModelFileType::GLTF, "Models/NewLevels/Level4.gltf");
		LOGINFO("Loading level 4 main asset");
	}break;
	case 5: {
		LOGINFO("Loading level 5 main asset");
		m_sceneManager.LoadModelOnEntityOnceLoaded(*m_ActiveLevel, Frac::EModelFileType::GLTF, "Models/NewLevels/Level5.gltf");
	}break;
	default:
		break;
	}
}

void LevelEntityCreator::SetLights(int level)
{

}

void LevelEntityCreator::SetCameraPosition()
{

}

void LevelEntityCreator::ClearCreatedEntities()
{
	if (m_ActiveLevel != nullptr) {
		//Deloading level first
		m_entityRegistry.GetEnTTRegistry().destroy(m_ActiveLevel->GetHandle());
		m_ActiveLevel = nullptr;
	}
}
