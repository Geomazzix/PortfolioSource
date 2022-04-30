#pragma once
#include "IEntityCreator.h"
#include "Components.h"
#include "TileManager.h"
class PlayerEntityCreator : public IEntityCreator
{
public:
	PlayerEntityCreator(TileManager& tileManager, Frac::EntityRegistry& entityReg, Frac::SceneManager& sceneManager);
	~PlayerEntityCreator();
	void CreatePlayerBase(int id, const glm::ivec2& position, const glm::vec3& rotation, float coreHealth, float coreTargetData);
	void CreateTower(int id, const glm::ivec2& gridPos); 
	void CreatePlayerBullet(const entt::entity& target, const ETurretType& type, const glm::vec3& position, const glm::vec2& gridPosition, const glm::vec3& direction);

	void ClearCreatedEntities() override;
	const Frac::Entity& GetActivePlayerBase();
	glm::vec2 GetBasePosition() const;
private:
	int m_createdTurrets = 0;
	int m_createdBullets = 0;
	glm::vec2 m_basePos = glm::vec2(-1);

	Frac::Entity* m_activeBaseEntity = nullptr;
};

