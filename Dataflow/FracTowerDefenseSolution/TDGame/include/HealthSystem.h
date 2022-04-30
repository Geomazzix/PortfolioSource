#pragma once
#include "Core/ISystem.h"

class GameDirector;
class PlayerManagerSystem;
class TileManager;

/// <summary>
/// The healthsystem manages changes in the health systems.
/// </summary>
class HealthSystem : public Frac::ISystem
{
public:
	HealthSystem(GameDirector& gameDirector, PlayerManagerSystem& playerManager, TileManager& tileManager);
	~HealthSystem() = default;
	void Update(float DeltaTime) override;

private:
	GameDirector& m_gameDirector;
	PlayerManagerSystem& m_playerManagerSystem;
	TileManager& m_tileManager;
};