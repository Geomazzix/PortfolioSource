#pragma once

#include "CanvasSystem.h"
class LevelManager;
class MainMenuCanvasSystem : public CanvasSystem
{
public:
	MainMenuCanvasSystem(Frac::EngineCore& engine, LevelManager& levelManager, GameDirector* gameDirector = nullptr, state gameState = GameState::Default);

	void Initialise() override;
	
private:
	void CreateCanvas() override;

	void Update(float dt) override;
	LevelManager& m_levelManager;
	std::vector<std::pair<std::string, std::string>> m_menuItems; /* <- first = query id, second = button text */
	bool m_tutorial = true;
};


