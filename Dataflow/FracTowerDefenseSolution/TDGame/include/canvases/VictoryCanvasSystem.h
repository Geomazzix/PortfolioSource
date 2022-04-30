#pragma once

#include "CanvasSystem.h"
#include "LevelManager.h"
class VictoryCanvasSystem : public CanvasSystem
{
public:
	VictoryCanvasSystem(Frac::EngineCore& engine, LevelManager& levelManager,GameDirector* gameDirector = nullptr, state gameState = GameState::Default);

	void Initialise() override;
	
private:
	void CreateCanvas() override;
	void Update(float dt) override;
	LevelManager& m_levelManager;
	std::vector<std::pair<std::string, std::string>> m_menuItems; /* <- first = query id, second = button text */
};