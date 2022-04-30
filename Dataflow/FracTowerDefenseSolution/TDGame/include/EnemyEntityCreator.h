#pragma once
#include "IEntityCreator.h"
#include "Components.h"

class LevelManager;
class EnemyEntityCreator : public IEntityCreator
{
public:
	EnemyEntityCreator(TileManager& tileManager, Frac::EntityRegistry& entityReg, Frac::SceneManager& sceneManager);
	~EnemyEntityCreator();
	void CreateEnemy(EEnemyType type, const glm::ivec2& gridposition, const glm::vec3& rotation, int enemyLevel, const entt::entity& spawner);
	void CreateSpawner(int id, const glm::ivec2& gridPosition, const glm::vec3& rotation, int enemySpawnType, float enemySpawnDelay);

	void ClearCreatedEntities() override;
	EEnemyType GetEnemySpawnType(int type);

	void SetLevelManager(LevelManager& levelManager);
private:
	int createdSpawners = 0;
	int createdEnemies = 0;

	LevelManager* m_LevelManager;
};
