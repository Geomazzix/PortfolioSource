#pragma once

#include "CanvasSystem.h"

class LoadCanvasSystem : public CanvasSystem
{
public:
	LoadCanvasSystem(Frac::EngineCore& engine, GameDirector* gameDirector = nullptr, state gameState = GameState::Default);

	void Initialise() override;

private:
	void CreateCanvas() override;

	void Update(float dt) override;

	Frac::Entity* m_infoTextEnt; // has the 'progress bar'
	float m_delay = 0.f;
	float m_timer = 0.f;
	bool m_loadNext = true;
};