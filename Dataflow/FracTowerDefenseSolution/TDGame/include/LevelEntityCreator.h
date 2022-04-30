#pragma once
#include "IEntityCreator.h"
#include "Components.h"
#include "SceneManagement/SceneManager.h"

class LevelEntityCreator : public IEntityCreator
{
public:
	LevelEntityCreator(TileManager& tileManager, Frac::EntityRegistry& entityReg, Frac::SceneManager& sceneManager);
	~LevelEntityCreator();
	void CreateStaticLevelModel(int level);
	void SetLights(int level);
	void SetCameraPosition();
	void ClearCreatedEntities() override;
private:
	Frac::Entity* m_ActiveLevel = nullptr;
	Frac::Entity* LevelParent;
};
