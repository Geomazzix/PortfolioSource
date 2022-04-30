#pragma once
#include "TileManager.h"
#include "Core/ISystem.h"
#include "EnemyEntityCreator.h" 

class GameDirector;

class EnemyDirectorSystem : public Frac::ISystem
{
public: 
	EnemyDirectorSystem(GameDirector& gameDirector, TileManager& tileManager, EnemyEntityCreator& enemyEntityCreator);

	~EnemyDirectorSystem() = default;

	void Update(float dt) override;
	void SetMaxWaves(int value);
	void SetCurrentWave(int value);
	void SetPrepPhaseTimer(float value);
	void SetPrepPhaseElapsedTimer(float value);

	const int GetMaxWaves();
	const int GetCurrentWave();
	const int GetEnemiesAlive();
	const float GetPrepPhaseTimer();
	const float GetPrepPhaseElapsedTimer();

private:

	//Private Functions
	void EnemyMovementUpdate(float dt);

	void SpawnerUpdate(float dt);

	void CalculateSpawners();

	void CalculateNewWave();

	void CalculateFlocking(const entt::entity& enemy);

	void DistributeEnemies();

	glm::ivec2 FindSpawnPoint(const glm::ivec2& spawnerGridPos);

	glm::vec3 FindPushForce(const glm::ivec2& curPos, const glm::ivec2& nextPos);

	//Private Variables
	TileManager& m_tileManager;
	GameDirector& m_gameDirector;

	int m_enemiesAlive;
	int m_spawnersActive;

	int m_maxWaves;
	int m_currentWave;
	bool m_nextLevel = false;

	//These values should be used from the Game Director once we merge everything.
	float m_prepPhaseTime;
	float m_prepPhaseTimeElapsed;

	entt::registry& m_enttReg;
	Frac::EntityRegistry& m_fracReg;

	EnemyEntityCreator& m_enemyEntityCreator;

	//Flocking Values
	glm::vec3 m_alignmentForce;
	glm::vec3 m_separationForce;
	glm::vec3 m_cohesionForce;

};

