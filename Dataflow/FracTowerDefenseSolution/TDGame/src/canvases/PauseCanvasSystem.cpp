#include "TDpch.h"

#include "Canvases/PauseCanvasSystem.h"
#include "LevelManager.h"
#include "SceneManagement/SceneManager.h"

#include "Audio/AudioManager.h"
#include "Audio/AudioComponent.h"

PauseCanvasSystem::PauseCanvasSystem(Frac::EngineCore& engine, LevelManager& levelManager, GameDirector* gameDirector, state gameState) :
	CanvasSystem(engine, gameDirector, gameState)
	, m_levelManager(levelManager)
	, m_previousRunningGameState(GameState::Default)
{
	
}

void PauseCanvasSystem::Initialise()
{
	m_menuItems = std::vector<std::pair<std::string, std::string>>({
		{"Resume", "Resume"}
		, {"Restart", "Restart"}
		, {"Settings", "Settings"}
		, {"MainMenu", "Main Menu"}
#if defined (PLATFORM_WINDOWS)
		, {"ReportBug", "Report bug"}
		,{"QuitGame", "Quit"}
#endif
	});

	CanvasSystem::Initialise();
}

void PauseCanvasSystem::CreateCanvas()
{
	std::string canvasName = "PauseCanvas";

	Frac::SceneManager& sceneManager = m_engine.GetSceneManager();
	
	Frac::Entity& canvasEnt = *(m_canvasEnt = new Frac::Entity(m_fracReg, canvasName));
	m_fracReg.AddComponent<TOR::Transform>(canvasEnt, TOR::Transform({glm::vec3(0.f), glm::vec3(0.f)}));
	m_fracReg.AddComponent<UICanvas>(canvasEnt, UICanvas({canvasName}));
	m_fracReg.AddComponent<Frac::TextComponent>(canvasEnt, Frac::TextComponent());

	m_fracReg.AddComponent<Frac::AudioSourceComponent>(canvasEnt, Frac::AudioSourceComponent());
	auto& canvasAudio = m_fracReg.GetComponent<Frac::AudioSourceComponent>(canvasEnt);
	m_engine.GetAudioManager().AddEvent(canvasAudio, "event:/UI_Select");
	m_engine.GetAudioManager().AddEvent(canvasAudio, "event:/UI_Confirm");

	canvasEnt.SetParent(sceneManager.GetActiveScene());

	UICanvas& theCanvas = m_fracReg.GetComponent<UICanvas>(canvasEnt);
	{
		Frac::Entity& mmLogoEnt = *new Frac::Entity(m_fracReg, canvasName + "_Logo");
		m_fracReg.AddComponent<TOR::Transform>(mmLogoEnt, TOR::Transform({m_logoPos, m_logoSize}));
		m_fracReg.AddComponent<UIElement>(mmLogoEnt, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(mmLogoEnt, Frac::TextComponent(
			{"u" // paint icon A
			, 0.75f
			, glm::vec3(1.f)
			, Frac::TextComponent::FontFamily::Icons }));

		mmLogoEnt.SetParent(canvasEnt);
		
		Frac::Entity& buttonAnchor = *new Frac::Entity(m_fracReg, "ButtonAnchor");
		m_fracReg.AddComponent<TOR::Transform>(buttonAnchor, TOR::Transform(
			{
				glm::vec3(0.f, m_buttonVerStartOffset, 0.f)
			}));
		m_fracReg.AddComponent<UIElement>(buttonAnchor, UIElement({}));

		buttonAnchor.SetParent(canvasEnt);
		
		{
			int yIncrement = 0;
			for (auto it = m_menuItems.begin(); it != m_menuItems.end(); ++it)
			{
				const std::string butName = it->second;

				Frac::Entity& mmButtonEnt = *new Frac::Entity(m_fracReg, canvasName + "_" + butName + "_Button");
				m_fracReg.AddComponent<TOR::Transform>(mmButtonEnt, TOR::Transform(
					{
						glm::vec3(
							0.f, 
							( static_cast<float>(yIncrement++) * -m_buttonVerPadding), 
							0.f), glm::vec3(0.5f)}
				));
				m_fracReg.AddComponent<UIElement>(mmButtonEnt, UIElement(
					{
						it->first
					}
				));
				m_fracReg.AddComponent<Frac::TextComponent>(mmButtonEnt, Frac::TextComponent(
					{
						it->second
						, m_buttonFontSize
					, theCanvas.IdleColor
					}
				));

				mmButtonEnt.SetParent(buttonAnchor);

				theCanvas.SelectableElements.push_back(mmButtonEnt.GetHandle());
			}
		}

		Frac::Entity& mmBackdropEnt = *new Frac::Entity(m_fracReg, canvasName + "_BackDrop");
		m_fracReg.AddComponent<TOR::Transform>(mmBackdropEnt, TOR::Transform({m_backdropPos, m_backdropSize}));
		m_fracReg.AddComponent<UIElement>(mmBackdropEnt, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(mmBackdropEnt, Frac::TextComponent(
			{"S" // square
			, m_backdropFontSize
			, m_backdropColor
			, Frac::TextComponent::FontFamily::Icons }));

		mmBackdropEnt.SetParent(canvasEnt);
	}
}

void PauseCanvasSystem::Update(float dt)
{
	CanvasSystem::Update(dt);
	
	UICanvas& canvas = m_fracReg.GetComponent<UICanvas>(*m_canvasEnt);
	
	if (m_firstRun)
	{
		if (m_gameDirector->GetPreviousState() != GameState::OptionsState)
		{
			m_previousRunningGameState = m_gameDirector->GetPreviousState();
		}
		
		m_firstRun = false;
	}
	
	if (canvas.SelectableElements.empty())
	{
		return;
	}

	UpdateCanvasSelector(dt, canvas);

	if (m_select)
	{
		const auto uiEventName = m_fracReg.GetComponent<UIElement>(m_fracReg.TranslateENTTEntity(canvas.SelectableElements[canvas.CurSelectedElement])).EventName;

		if (!uiEventName.empty())
		{
			if (uiEventName == "Resume")
			{
				LOGINFO("[PauseCanvasSystem] Game Resumed");
				m_gameDirector->SwitchToGroup(m_previousRunningGameState);
			}
			else if (uiEventName == "Restart")
			{
				LOGINFO("[PauseCanvasSystem] Restart");
				m_levelManager.LoadLevel(m_levelManager.m_activeLevel);
				m_gameDirector->SwitchToGroup(GameState::WavePhase);
			}
			else if (uiEventName == "Settings")
			{
				LOGINFO("[PauseCanvasSystem] Settings");
				m_gameDirector->SwitchToGroup(GameState::OptionsState);
			}
#ifdef PLATFORM_WINDOWS
			else if (uiEventName == "ReportBug")
			{
				LOGINFO("[PauseCanvasSystem] Reporting a bug!");
				BugReporter::BugReporter::OpenApplication();
			}
#endif
			else if (uiEventName == "MainMenu")
			{
				LOGINFO("[PauseCanvasSystem] Entering Main Menu Phase");
				m_levelManager.ClearLevel();
				m_gameDirector->SwitchToGroup(GameState::MainMenuState);
			}
			else if (uiEventName == "QuitGame")
			{
				LOGINFO("[PauseCanvasSystem] Entering QuitGame Phase");
				m_gameDirector->SwitchToGroup(GameState::QuitGameState);
			}

			if (m_fracReg.HasComponent<Frac::AudioSourceComponent>(*m_canvasEnt))
			{
				m_engine.GetAudioManager().PlayEvent(m_fracReg.GetComponent<Frac::AudioSourceComponent>(*m_canvasEnt), "event:/UI_Confirm");
			}

			HideCanvas();
			Reset();
		}
		else
		{
			LOGWARNING("CanvasSystem | PauseCanvas select, uiEventName is empty");
		}

		return;
	}

	if (m_pause)
	{
		m_gameDirector->SwitchToGroup(m_previousRunningGameState); //If we were in Prep Phase before Pausing, the Enemy Director will return to that state.
		LOGINFO("[PauseCanvasSystem] Game Resumed");
		
		HideCanvas();
		Reset();
	}
}

void PauseCanvasSystem::Reset()
{
	m_firstRun = true;
}
