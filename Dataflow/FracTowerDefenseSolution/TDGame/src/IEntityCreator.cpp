#include "TDpch.h"
#include "IEntityCreator.h"

IEntityCreator::IEntityCreator(TileManager& tileManager, Frac::EntityRegistry& entityReg, Frac::SceneManager& sceneManager) : m_tileManager(tileManager), m_entityRegistry(entityReg), m_sceneManager(sceneManager), m_sceneEntity(sceneManager.GetActiveScene())
{
 
}

IEntityCreator::~IEntityCreator() {

}

bool IEntityCreator::CheckAvailability(glm::vec2 position) {
	return m_tileManager.IsTileOccupied(m_tileManager.GetTile((int)position.x, (int)position.y));
}

void IEntityCreator::ClearCreatedEntities()
{

}
