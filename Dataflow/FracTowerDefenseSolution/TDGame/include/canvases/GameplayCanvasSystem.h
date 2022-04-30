#pragma once

#include "CanvasSystem.h"

class CursorSystem;
class EnemyDirectorSystem;
class PlayerEntityCreator;
class PlayerManagerSystem;
class HealthComponent;
class LevelManager;
class GameplayCanvasSystem : public CanvasSystem
{
public:
	GameplayCanvasSystem(TileManager* tileManager, CursorSystem* cursorSystem, EnemyDirectorSystem* enemyDirSystem, PlayerEntityCreator* playerEntCreator, LevelManager& levelManager, PlayerManagerSystem* playerManagerSystem, Frac::EngineCore& engine, GameDirector* gameDirector = nullptr, state gameState = GameState::Default);

	void Initialise() override;

private:
	void CreateCanvas() override;
	void Update(float dt) override;
	void UpdateBuildSelection(float dt);
	void UpdatePrepPhaseElements(float dt);
	void UpdateButtonLayout() const;
	void UpdateBuildInfoBox(float dt);
	
	void CreatePrepBarElement(Frac::Entity& canvasEnt);
	void CreateCounterElements(Frac::Entity& canvasEnt);
	void CreateWaveElements(Frac::Entity& canvasEnt);
	void CreateBuildableElements(Frac::Entity& canvasEnt);
	
	void UpdateWaveElements();
	void RecreateWaveElements();
	
	/*!
	* @brief Handles the popup entity
	*/
	void HandlePopupEntity(Frac::Entity target);

	void UpdateSellTextColor(entt::entity) const;
	
	TileManager* m_tileManager;
	CursorSystem* m_cursorSystem;
	EnemyDirectorSystem* m_enemyDirSystem;
	PlayerEntityCreator* m_playerEntCreator;
	PlayerManagerSystem* m_playerManagerSystem;
	LevelManager& m_levelManager;

	Frac::Entity* m_resourceTextEnt;
	int m_resourceElementCount;
	
	Frac::Entity* m_prepBarEnt;

	Frac::Entity* m_infoBoxAnchor;
	
	Frac::Entity* m_buildInfoTextEnt;
	Frac::Entity* m_sellInfoTextEnt;
	
	Frac::Entity* m_prepBarInfoIcon;

	std::vector<std::pair<std::string, glm::vec3>> m_towerIcons;
	std::vector<Frac::Entity*> m_towerElementEnts;

	// build behavior, might want to move to a different system
	int m_buildIDSelected;

	// wave elements
	Frac::Entity* m_waveAnchorEnt;
	std::vector<Frac::Entity*> m_waveElementEnts;
	int m_waveElementsHighlighted;
	
	// on screen buttons
	Frac::Entity* m_leftButton;
	Frac::Entity* m_rightButton;

	bool m_isInfoBoxVisible;
	
	// style
	const float m_farScreenX = 220.f;
	const float m_topYOffset = 110.f;

	const float m_bottomYOffset = -110.f;

	//prep bar
	const glm::vec3 m_prepbarColor = glm::vec3(1.f, 0.59f, 0.74f);
	
	// style - counter elements
	const float m_resTitleFontSize = 0.18f;
	const float m_resCountFontSize = 0.16f;
	
	const glm::vec3 m_resTitleTextCol = glm::vec3(0.4f, 0.86f, 0.98f);
	const glm::vec3 m_resCountTextCol = glm::vec3(1.f);
	
	// style buildable elements
	const glm::vec3 m_buildIdleCol		= glm::vec3(0.93f, 0.43f, 0.61f);
	const glm::vec3 m_buildSelectedCol	= glm::vec3(0.98f, 0.98f, 0.57f);

	const glm::vec3 m_infoBoxColor		= glm::vec3(0.2f);

	const float m_infoBoxIconSize		= 0.018f;
	const float m_infoBoxTextSize		= 0.16f;

	const float m_infoIconHor			= 11.2f;
	const float m_infoTextHorOffset		= -8.0f;

	const glm::vec3 m_buildInfoTextIdleColor = glm::vec3(0.75f);
	
	// style wave elements
	const float m_waveHorOffset			= -8.0f;
};
