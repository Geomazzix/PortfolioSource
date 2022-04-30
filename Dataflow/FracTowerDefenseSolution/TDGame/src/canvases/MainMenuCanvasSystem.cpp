#include "TDpch.h"

#include "Canvases/MainMenuCanvasSystem.h"
#include "LevelManager.h"
#include "SceneManagement/SceneManager.h"

#include "Audio/AudioManager.h"
#include "Audio/AudioComponent.h"

MainMenuCanvasSystem::MainMenuCanvasSystem(Frac::EngineCore& engine, LevelManager& levelManager, GameDirector* gameDirector, state gameState)
	: CanvasSystem(engine, gameDirector, gameState), m_levelManager(levelManager)
{
}

void MainMenuCanvasSystem::Initialise()
{
	m_menuItems = std::vector<std::pair<std::string, std::string>>({
		{"Play", "Play"}
		, {"Options", "Settings"}
#if defined (PLATFORM_WINDOWS)
		, {"ReportBug", "Report bug"}
#endif
		, {"Credits", "Credits"}
#if defined (PLATFORM_WINDOWS)
		, {"QuitGame", "Quit"}
#endif
		});

	CanvasSystem::Initialise();
}

void MainMenuCanvasSystem::CreateCanvas()
{
	std::string canvasName = "MainMenuCanvas";

	Frac::SceneManager& sceneManager = m_engine.GetSceneManager();

	Frac::Entity& canvasEnt = *(m_canvasEnt = new Frac::Entity(m_fracReg, canvasName));
	m_fracReg.AddComponent<TOR::Transform>(canvasEnt, TOR::Transform({ glm::vec3(0.f) }));
	m_fracReg.AddComponent<UICanvas>(canvasEnt, UICanvas({ canvasName }));
	m_fracReg.AddComponent<Frac::TextComponent>(canvasEnt, Frac::TextComponent());

	m_fracReg.AddComponent<Frac::AudioSourceComponent>(canvasEnt, Frac::AudioSourceComponent());
	auto& canvasAudio = m_fracReg.GetComponent<Frac::AudioSourceComponent>(canvasEnt);
	m_engine.GetAudioManager().AddEvent(canvasAudio, "event:/UI_Select");
	m_engine.GetAudioManager().AddEvent(canvasAudio, "event:/UI_Confirm");

	canvasEnt.SetParent(sceneManager.GetActiveScene());

	UICanvas& theCanvas = m_fracReg.GetComponent<UICanvas>(canvasEnt);
	{
		Frac::Entity& mmLogoEnt = *new Frac::Entity(m_fracReg, canvasName + "_Logo");
		m_fracReg.AddComponent<TOR::Transform>(mmLogoEnt, TOR::Transform({ m_logoPos, m_logoSize }));
		m_fracReg.AddComponent<UIElement>(mmLogoEnt, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(mmLogoEnt, Frac::TextComponent(
			{ "DataFlow" // paint icon A
			, 2.f
			, glm::vec3(1.f)
			, Frac::TextComponent::FontFamily::Regular }));

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
				Frac::Entity& mmButtonEnt = *new Frac::Entity(m_fracReg, canvasName + "_" + it->second + "_Button");
				m_fracReg.AddComponent<TOR::Transform>(mmButtonEnt, TOR::Transform(
					{ glm::vec3(
						0.f,
						(static_cast<float>(yIncrement++) * -m_buttonVerPadding),
						0.f), glm::vec3(0.5f) }));
				m_fracReg.AddComponent<UIElement>(mmButtonEnt, UIElement(
					{
						it->first
					}));
				m_fracReg.AddComponent<Frac::TextComponent>(mmButtonEnt, Frac::TextComponent(
					{
						it->second
						,m_buttonFontSize
					, theCanvas.IdleColor
					}));

				mmButtonEnt.SetParent(buttonAnchor);

				theCanvas.SelectableElements.push_back(mmButtonEnt.GetHandle());
			}
		}

		Frac::Entity& mmBackdropEnt = *new Frac::Entity(m_fracReg, canvasName + "_Backdrop");
		m_fracReg.AddComponent<TOR::Transform>(mmBackdropEnt, TOR::Transform(
			{
				m_backdropPos
				, m_backdropSize
			}));
		m_fracReg.AddComponent<UIElement>(mmBackdropEnt, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(mmBackdropEnt, Frac::TextComponent(
			{ "S"//"S" // square
			, m_backdropFontSize
			, m_backdropColor
			, Frac::TextComponent::FontFamily::Icons }));

		mmBackdropEnt.SetParent(canvasEnt);
	}
}

void MainMenuCanvasSystem::Update(float dt)
{
	CanvasSystem::Update(dt);

	UICanvas& canvas = m_fracReg.GetComponent<UICanvas>(*m_canvasEnt);

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
			if (uiEventName == "Play")
			{
				LOGINFO("[MainMenuCanvasSystem] Game Started");
				m_levelManager.LoadLevel(m_levelManager.m_activeLevel);
				if (m_tutorial)
				{
					m_gameDirector->SwitchToGroup(GameState::TutorialState);
					m_tutorial = false;
				}
				else
				{
					m_gameDirector->SwitchToGroup(GameState::WavePhase);
				}
			}
			else if (uiEventName == "Options")
			{
				LOGINFO("[MainMenuCanvasSystem] Options");
				m_gameDirector->SwitchToGroup(GameState::OptionsState);
			}
			else if (uiEventName == "Credits")
			{
				LOGINFO("[MainMenuCanvasSystem] Start Credits");
				m_gameDirector->SwitchToGroup(GameState::CreditsState);
			}
#ifdef PLATFORM_WINDOWS
			else if (uiEventName == "ReportBug")
			{
				LOGINFO("[MainMenuCanvasSystem] Reporting a bug!");
				BugReporter::BugReporter::OpenApplication();
			}
#endif
			else if (uiEventName == "QuitGame")
			{
				LOGINFO("[MainMenuCanvasSystem] Entering Quit Phase");
				m_gameDirector->SwitchToGroup(GameState::QuitGameState);
			}

			if (m_fracReg.HasComponent<Frac::AudioSourceComponent>(*m_canvasEnt))
			{
				m_engine.GetAudioManager().PlayEvent(m_fracReg.GetComponent<Frac::AudioSourceComponent>(*m_canvasEnt), "event:/UI_Confirm");
			}

			HideCanvas();
		}
		else
		{
			LOGWARNING("CanvasSystem | MainMenuCanvas | uiEventName is empty, unable to set active canvas!");
		}
	}
}