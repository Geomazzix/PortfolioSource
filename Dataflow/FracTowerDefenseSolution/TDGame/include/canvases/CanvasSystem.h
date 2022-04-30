#pragma once

#include "Core/ISystem.h"

#include "GameDirector.h" // for GameState

// for potential canvases inheriting from this
#include "Canvases/CanvasComponents.h"
#include "Graphics/Components/TextComponent.h"

namespace Frac 
{
	class EngineCore;
	class InputSystem;
}

class CanvasSystem : public Frac::ISystem
{
public:
	CanvasSystem(Frac::EngineCore& engine, GameDirector* gameDirector = nullptr, state gameState = GameState::Default);
	~CanvasSystem() = default;

	virtual void Initialise();

	void HideCanvas();
	void ShowCanvas();

protected:
	void Update(float a_deltaTime) override;
	
	virtual void CreateCanvas() = 0;
	
	void UpdateCanvasSelector(float dt, UICanvas& canvas);

	/*!
	* @brief always makes sure canvases are following and facing the camera
	*/
	void UpdateCanvasTransform(TOR::Transform& transform, Frac::TextComponent& text);
	/*!
	* @brief A centralized place to handle input from
	*/
	void HandleInput();

	entt::entity GetActiveCamera();

	void UpdateHeightScaling() const;
	
	void SetTextChildRenderState(Frac::Entity&, bool) const;

	Frac::Entity* m_canvasEnt;

	entt::entity m_activeCamHandle;

	entt::entity m_activePopupEnt;
	entt::entity m_lastActiveCanvasEnt;

	Frac::EngineCore& m_engine;
	Frac::EntityRegistry& m_fracReg;
	Frac::InputSystem& m_inputSystem;

	GameDirector* m_gameDirector;

	bool m_firstRun;
	bool m_inputFirstRun;
	
	bool m_updateInput;

	bool m_isCanvasHidden;
	
	// INPUT
	bool m_select;
	bool m_up;
	bool m_down;
	bool m_pause;

	// hide canvas var
	bool m_isCanvasManuallyHidden = false;
	
protected:
	// hard coded style values
	const float m_canvasDepthOffset		= 0.5f;

	const glm::vec3 m_logoPos			= glm::vec3(0.f, 90.f, 0.f);
	const glm::vec3 m_logoSize			= glm::vec3(0.2f);
	const float m_logoFontSize			= 1.0f;

	const glm::vec3 m_backdropPos		= glm::vec3(0.f, 0.f, 5.f);
	const glm::vec3 m_backdropSize		= glm::vec3(1.f, 1.f, 1.f);
	const glm::vec3 m_backdropColor		= glm::vec3(0.38f, 0.19f, 0.34f);
	const float m_backdropFontSize		= 40.f;

	const float m_buttonDepth			= -1.0f;
	const float m_buttonVerStartOffset	= 30.f;
	const float m_buttonVerPadding		= 30.f;
	const float m_buttonFontSize		= 0.45f;

	const glm::vec3 m_primaryButtonColor = glm::vec3(0.99f, 0.75f, 0.94f);
	const glm::vec3 m_quitButtonColor = glm::vec3(1.f, 0.72f, 0.72f);
	
	// options, controls related
	const float m_controlsButtonVerOffset	= -60.f;

	// game play canvas related
	const glm::vec3 m_iconBasePos		= glm::vec3(220.f, 120.f, 0.f);

	const float m_iconHorOffset			= -60.f;
	const glm::vec3 m_resCountColor		= glm::vec3(1.f, 1.f, 1.f);

	const glm::vec3 m_counterListOffset	= glm::vec3(0.f, -30.f, 0.f);

	const glm::vec3 m_resCountPos		= glm::vec3(0.f, 0.f, 0.f);
	const glm::vec3 m_waveCounterPos	= glm::vec3(0.f, 40.f, 0.f);
	const glm::vec3 m_incomingCounterPos= glm::vec3(0.f, 80.f, 0.f);
	const glm::vec3 m_baseHpCounterPos	= glm::vec3(0.f, 120.f, 0.f);

	// win/lose
	const glm::vec3 m_winBackDropCol	= glm::vec3(0.09f, 0.24f, 0.32f);
	const glm::vec3 m_winHeaderCol		= glm::vec3(0.65f, 0.91f, 0.99f);

	const glm::vec3 m_loseBackDropCol	= glm::vec3(0.33f, 0.24f, 0.25f);
	const glm::vec3 m_loseHeaderCol		= glm::vec3(0.976f, 0.52f, 0.498f);

	const float m_winLoseBackdropSize	= 50.f;
	const float m_winLoseButtonVerOffset= -10.f;

	// popup
	const glm::vec3 m_popupCol			= glm::vec3(0.f, 0.f, 0.f);
	const glm::vec3 m_popupSize			= glm::vec3(0.045f, 0.05f, 0.05f);

	const glm::vec3 m_popupOffset		= glm::vec3(-1.f, 1.5f, 0.5f);

	// misc 
	const float m_selectorFontSize		= 0.1f;

	// boundaries
	const float m_lowTextYPoint			= -130.f;

	// hide/show canvas vars
	float m_canvasBaseScale		= 0.055f;

	// wave elements
	const glm::vec3 m_waveIdleCol		= glm::vec3(1.f, 1.f, 1.f);
	const glm::vec3 m_waveHighlightCol	= glm::vec3(0.89f, 0.89f, 0.27f);
};