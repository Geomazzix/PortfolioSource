#pragma once

#include "CanvasSystem.h"

class LevelManager;
class CameraSystem;

class NextLevelCanvasSystem : public CanvasSystem
{
public:
	NextLevelCanvasSystem(LevelManager* levelManager, Frac::EngineCore& engine, GameDirector* gameDirector = nullptr, state gameState = GameState::Default);

	void Initialise() override;

private:
	void CreateCanvas() override;
	void Update(float dt) override;

	void Reset();

	LevelManager* m_levelManager;
	
	Frac::Entity* m_curLevelEnt;
	Frac::Entity* m_maxLevelEnt;

	float m_timer;
	float m_inputActivateTarget;
	float m_timeUntilNextLevel;

	// style
	const glm::vec3 m_headerColor = glm::vec3(0.99f, 0.93f, 0.66f);
	const glm::vec3 m_nextLevelIndicatorColor = glm::vec3(0.99f, 0.69f, 0.33f);
	const glm::vec3 m_splitIndicatorColor = glm::vec3(0.99f, 0.95f, 0.67f);
	const float m_horOffset = 20.f;
	const float m_verOffset = -15.f;
	const float m_fontSize = 0.4f;
};
