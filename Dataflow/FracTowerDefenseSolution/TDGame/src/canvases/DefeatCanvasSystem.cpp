#include "TDpch.h"

#include "Canvases/DefeatCanvasSystem.h"

#include "SceneManagement/SceneManager.h"
#include "LevelManager.h"
#include "Audio/AudioManager.h"
#include "Audio/AudioComponent.h"

DefeatCanvasSystem::DefeatCanvasSystem(Frac::EngineCore& engine, LevelManager& levelManager, GameDirector* gameDirector, state gameState) :
CanvasSystem(engine, gameDirector, gameState), m_LevelManager(levelManager)
{
	
}

void DefeatCanvasSystem::Initialise()
{
	m_menuItems = std::vector<std::pair<std::string, std::string>>({
		{"Retry", "Retry"}
		, {"MainMenu", "Main Menu"}
		, {"Quit", "Quit"}
		});
	
	CanvasSystem::Initialise();
}

void DefeatCanvasSystem::CreateCanvas()
{
	std::string canvasName = "DefeatCanvas";

	Frac::SceneManager& sceneManager = m_engine.GetSceneManager();

	Frac::Entity& canvasEnt = *(m_canvasEnt = new Frac::Entity(m_fracReg, canvasName));
	m_fracReg.AddComponent<TOR::Transform>(canvasEnt, TOR::Transform({glm::vec3(0.f), glm::vec3(0.f)}));
	m_fracReg.AddComponent<UICanvas>(canvasEnt, UICanvas({canvasName}));
	m_fracReg.AddComponent<Frac::TextComponent>(canvasEnt, Frac::TextComponent());

	canvasEnt.SetParent(sceneManager.GetActiveScene());

	m_fracReg.AddComponent<Frac::AudioSourceComponent>(canvasEnt, Frac::AudioSourceComponent());
	auto& canvasAudio = m_fracReg.GetComponent<Frac::AudioSourceComponent>(canvasEnt);
	m_engine.GetAudioManager().AddEvent(canvasAudio, "event:/UI_Select");
	m_engine.GetAudioManager().AddEvent(canvasAudio, "event:/UI_Confirm");
	
	UICanvas& theCanvas = m_fracReg.GetComponent<UICanvas>(canvasEnt);
	{
		Frac::Entity& mmLogoEnt = *new Frac::Entity(m_fracReg, canvasName + "_Logo");
		m_fracReg.AddComponent<TOR::Transform>(mmLogoEnt, TOR::Transform({m_logoPos, m_logoSize}));
		m_fracReg.AddComponent<UIElement>(mmLogoEnt, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(mmLogoEnt, Frac::TextComponent(
			{"Defeat" // paint icon A
			, m_logoFontSize + 3.f
			, m_loseHeaderCol
			}));

		mmLogoEnt.SetParent(canvasEnt);

		Frac::Entity& buttonAnchor = *new Frac::Entity(m_fracReg, "ButtonAnchor");
		m_fracReg.AddComponent<TOR::Transform>(buttonAnchor, TOR::Transform(
			{
				glm::vec3(0.f, m_buttonVerStartOffset, 0.f)
			}));
		m_fracReg.AddComponent<UIElement>(buttonAnchor, UIElement({}));

		buttonAnchor.SetParent(canvasEnt);

		// selectable elements + selector
		{
			int yIncrement = 0;
			for (auto it = m_menuItems.begin(); it != m_menuItems.end(); ++it)
			{
				Frac::Entity& buttonElement = *new Frac::Entity(m_fracReg, canvasName + "_BuildInfo");
				m_fracReg.AddComponent<TOR::Transform>(buttonElement, TOR::Transform(
					{glm::vec3(
					0.f, 
					m_winLoseButtonVerOffset - (15.f * static_cast<float>(yIncrement++)), 
					m_buttonDepth), glm::vec3(0.5f)}));
				m_fracReg.AddComponent<UIElement>(buttonElement, UIElement(
					{
						it->first
					}
				));
				m_fracReg.AddComponent<Frac::TextComponent>(buttonElement, Frac::TextComponent(
					{
						it->second
						,m_buttonFontSize - 0.1f
						, theCanvas.IdleColor
					}
				));

				buttonElement.SetParent(buttonAnchor);

				theCanvas.SelectableElements.push_back(buttonElement.GetHandle());
			}
		}

		Frac::Entity& mmBackdropEnt = *new Frac::Entity(m_fracReg, canvasName + "_Backdrop");
		m_fracReg.AddComponent<TOR::Transform>(mmBackdropEnt, TOR::Transform({m_backdropPos, m_backdropSize}));
		m_fracReg.AddComponent<UIElement>(mmBackdropEnt, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(mmBackdropEnt, Frac::TextComponent(
			{"S" // Circle
			, m_winLoseBackdropSize
			, m_loseBackDropCol
			, Frac::TextComponent::FontFamily::Icons }));

		mmBackdropEnt.SetParent(canvasEnt);
	}
}

void DefeatCanvasSystem::Update(float dt)
{
	CanvasSystem::Update(dt);
	
	UICanvas& canvas = m_fracReg.GetComponent<UICanvas>(*m_canvasEnt);

	UpdateCanvasSelector(dt, canvas);

	if (m_select)
	{
		const auto uiEventName = m_fracReg.GetComponent<UIElement>(m_fracReg.TranslateENTTEntity(canvas.SelectableElements[canvas.CurSelectedElement])).EventName;

		if (!uiEventName.empty())
		{
			if (uiEventName == "Retry")
			{
				LOGINFO("[VictoryCanvasSystem] Restarting Level");
				m_LevelManager.LoadLevel(m_LevelManager.m_activeLevel);
				m_gameDirector->SwitchToGroup(GameState::WavePhase);
			}
			else if (uiEventName == "MainMenu")
			{
				LOGINFO("[VictoryCanvasSystem] Entering Main Menu Phase");
				m_LevelManager.ClearLevel();
				m_gameDirector->SwitchToGroup(GameState::MainMenuState);
			}
			else if (uiEventName == "Quit")
			{
				LOGINFO("[VictoryCanvasSystem] Entering Quit Phase");
				m_gameDirector->SwitchToGroup(GameState::QuitGameState);
			}
			
			HideCanvas();
		}
	}
}
