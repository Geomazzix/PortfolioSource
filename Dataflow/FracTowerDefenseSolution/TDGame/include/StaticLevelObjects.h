#pragma once
#include "StaticLevelInteractable.h"
struct StaticLevelTower : public StaticLevelInteractable {
public:
	int m_bulletType;
	float m_fireRate;
	int m_towerHealth;
private:
};

struct StaticLevelCore : public StaticLevelInteractable {
public:
	int m_coreHealth;
	int m_coreTargetData;
private:
};

struct StaticLevelEnemySpawner : public StaticLevelInteractable {
public:
	int m_enemySpawnType;
	float m_enemySpawnDelay;
private:
};