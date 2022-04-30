#pragma once

#include "CanvasSystem.h"

class OptionsCanvasSystem : public CanvasSystem
{
public:
	OptionsCanvasSystem(Frac::EngineCore& engine, GameDirector* gameDirector = nullptr, state gameState = GameState::Default);

	void Initialise() override;

private:
	void CreateCanvas() override;
	void Update(float dt) override;

	Frac::Entity* m_volumeTextEnt;
	
	float m_volumeValue;
	float m_volumeStep;

	float m_volumeTimer;
	float m_volumeStepWait;
	
	// styling
	const float m_settingElementVerOffset	= -70.f;
	const float m_volumeSliderHeightOffset	= -25.f;

	const float m_volumeIndicatorSize		= 0.03f;
};