#pragma once
#include "Components.h"
#include "TileManager.h"
#include "glm/common.hpp"
#include "Core/EntityRegistry.h"
#include "SceneManagement/SceneManager.h"
class IEntityCreator
{

public:
	IEntityCreator(TileManager& tileManager, Frac::EntityRegistry& entityReg, Frac::SceneManager& sceneManager);
	~IEntityCreator();
	bool CheckAvailability(glm::vec2 position);
	virtual void ClearCreatedEntities();
	TileManager& m_tileManager;
	Frac::EntityRegistry& m_entityRegistry;
	Frac::SceneManager& m_sceneManager;
	Frac::Entity& m_sceneEntity;
private:

};