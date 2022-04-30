#pragma once

#include "CanvasSystem.h"

class DeveloperCanvasSystem : public CanvasSystem
{
public:
	DeveloperCanvasSystem (CanvasSystem* gameplayCanvasSystem, Frac::EngineCore& engine, GameDirector* gameDirector = nullptr, state gameState = GameState::Default);

	void Initialise() override;

private:
	void CreateCanvas() override;
	void Update(float dt) override;

	CanvasSystem* m_gameplayCanvasSystem;
};