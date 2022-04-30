#pragma once

#include "CanvasSystem.h"
class LevelManager;
class PauseCanvasSystem : public CanvasSystem
{
public:
	PauseCanvasSystem(Frac::EngineCore& engine, LevelManager& levelManager, GameDirector* gameDirector = nullptr, state gameState = GameState::Default);

	void Initialise() override;
	
private:
	void CreateCanvas() override;
	void Update(float dt) override;

	void Reset();

	state m_previousRunningGameState;
	
	std::vector<std::pair<std::string, std::string>> m_menuItems; /* <- first = query id, second = button text */
	LevelManager& m_levelManager;
};