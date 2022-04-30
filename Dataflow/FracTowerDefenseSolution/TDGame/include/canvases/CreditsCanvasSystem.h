#pragma once

#include "CanvasSystem.h"

class CameraSystem;

class CreditsCanvasSystem : public CanvasSystem
{
public:
	CreditsCanvasSystem(CameraSystem* camSystem, Frac::EngineCore& engine, GameDirector* gameDirector = nullptr, state gameState = GameState::Default);

	void Initialise() override;

private:
	void CreateCanvas() override;
	void Update(float dt) override;

	void UpdateButtonLayout() const;
	
	void Reset();

	CameraSystem* m_camSystem;
	
	std::vector<std::pair<std::string, char>> m_roles;
	std::vector<std::pair<std::string, std::vector<char>>> m_names;

	Frac::Entity* m_crawlerAnchorEnt;
	Frac::Entity* m_skipAnchorEnt;
	Frac::Entity* m_skipGraphicEnt;
	
	float m_timer;
	float m_inputActivateTarget;
	
	glm::vec3 m_startPos;
	float m_crawlerSpeed;

	float m_timeUntilSkip;
	float m_timeUntilSkipGraphic;

	bool m_skipGraphicIsVisible;
	
	// styling
	const glm::vec3 m_roleColor = glm::vec3(1.f, 1.f, 1.f);
	const glm::vec3 m_nameColor = glm::vec3(1.f, 1.f, 0.f);

	const float m_roleNameVerOffset = 25.f;
	
	const float m_nameHeightOffset = 12.f;
	const float m_roleHeightOffset = 15.f;
};
