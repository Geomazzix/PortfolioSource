#pragma once
#include "Core/ISystem.h"
#include "Graphics/ParticleRenderSystem/ParticleSystem.h"

class TileManager;

class BulletSystem : public Frac::ISystem
{
public:

	BulletSystem(TileManager& tileManager);

	~BulletSystem() = default;

	void Update(float dt) override;

private:

	entt::entity ReturnClosestEnemy(int range, glm::vec3 worlpos);
	std::vector<entt::entity> ReturnEnemiesInRange(int range, glm::vec3 worldpos);

	void RemoveSelf(float dt);

	Frac::FountainParticleEmitterSettings m_enemyDamageParticleSettings;
	Frac::FountainParticleEmitterSettings m_explosionParticleSettings;

	entt::registry& m_enttReg;
	Frac::EntityRegistry& m_fracReg;
	TileManager& m_tileManager;
};

