#pragma once
#include "Core/ISystem.h"
#include "Glm/glm.hpp"
#include "PlayerEntityCreator.h"
class TileManager;

class TurretSystem : public Frac::ISystem
{
public:

	TurretSystem(TileManager& tileManager, PlayerEntityCreator& playerEntityCreator);

	~TurretSystem() = default;

	void Update(float dt) override;

private:
	const entt::entity& GetEnemyInRange(int range, const glm::ivec2& gridPos);
	void TurretFindTargetSystem();
	void TurretShootTargetSystem(float dt);

	void SpawnAnimationSystem(float dt);
	void DespawnAnimationSystem(float dt);

	void AddRandomFireDelay(TurretStats& stats);

	entt::registry& m_enttReg;
	Frac::EntityRegistry& m_fracReg;
	TileManager& m_tileManager;
	PlayerEntityCreator& m_playerEntityCreator;

	const float m_turretSpawnAnimationSpeed = 7.5f;
	const float m_bulletSpawnYOffset = 0.4f;

	unsigned int m_randomSeed = 0;
};

