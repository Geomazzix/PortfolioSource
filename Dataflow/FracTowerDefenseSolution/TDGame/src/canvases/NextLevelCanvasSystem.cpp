#include "TDpch.h"

#include "Canvases/NextLevelCanvasSystem.h"

#include "SceneManagement/SceneManager.h"

#include "CameraSystem.h"
#include "LevelManager.h"

#include "InputSystem/InputSystem.h"

#include "Audio/AudioComponent.h"
#include "Audio/AudioManager.h"

NextLevelCanvasSystem::NextLevelCanvasSystem(LevelManager* levelManager, Frac::EngineCore& engine, GameDirector* gameDirector, state gameState) :
	CanvasSystem(engine, gameDirector, gameState)
	, m_levelManager(levelManager)
	, m_curLevelEnt(nullptr)
	, m_maxLevelEnt(nullptr)
	, m_timer(0.f)
	, m_inputActivateTarget(0.5f)
	, m_timeUntilNextLevel(3.f)
{
}

void NextLevelCanvasSystem::Initialise()
{
	CanvasSystem::Initialise();
}

void NextLevelCanvasSystem::CreateCanvas()
{
	std::string canvasName = "NextLevelCanvas";

	Frac::SceneManager& sceneManager = m_engine.GetSceneManager();

	Frac::Entity& canvasEnt = *(m_canvasEnt = new Frac::Entity(m_fracReg, canvasName));
	m_fracReg.AddComponent<TOR::Transform>(canvasEnt, TOR::Transform({glm::vec3(0.f), glm::vec3(0.f)}));
	m_fracReg.AddComponent<UICanvas>(canvasEnt, UICanvas({canvasName}));
	m_fracReg.AddComponent<Frac::TextComponent>(canvasEnt, Frac::TextComponent());

	canvasEnt.SetParent(sceneManager.GetActiveScene());

	UICanvas& theCanvas = m_fracReg.GetComponent<UICanvas>(canvasEnt);

	Frac::Entity& mmBackdropEnt = *new Frac::Entity(m_fracReg, "Backdrop");
	m_fracReg.AddComponent<TOR::Transform>(mmBackdropEnt, TOR::Transform({glm::vec3(0.f, 0.f, 700.f), glm::vec3(100.f, 100.f, 1.f)}));
	m_fracReg.AddComponent<UIElement>(mmBackdropEnt, UIElement({}));
	m_fracReg.AddComponent<Frac::TextComponent>(mmBackdropEnt, Frac::TextComponent(
		{"S"
		, 1.f
		, glm::vec3(0.f)
		, Frac::TextComponent::FontFamily::Icons }));

	mmBackdropEnt.SetParent(canvasEnt);

	{
		Frac::Entity& nextLevelAnchor = *new Frac::Entity(m_fracReg, "CrawlerAnchor");
		m_fracReg.AddComponent<TOR::Transform>(nextLevelAnchor, TOR::Transform({glm::vec3(0.f), glm::vec3(1.f)}));
		m_fracReg.AddComponent<UIElement>(nextLevelAnchor, UIElement({}));

		nextLevelAnchor.SetParent(canvasEnt);

		Frac::Entity& infoBox = *new Frac::Entity(m_fracReg, "Header");
		m_fracReg.AddComponent<TOR::Transform>(infoBox, TOR::Transform({glm::vec3(0.f, 15.f, 0.f), glm::vec3(1.f)}));
		m_fracReg.AddComponent<UIElement>(infoBox, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(infoBox, Frac::TextComponent(
			{
				"Moving to next level"
				,0.4f
			, m_headerColor
			}));

		infoBox.SetParent(nextLevelAnchor);
		
		Frac::Entity& curLevelText = *new Frac::Entity(m_fracReg, "curIndicatorText");
		m_fracReg.AddComponent<TOR::Transform>(curLevelText, TOR::Transform({glm::vec3(m_horOffset, m_verOffset, 0.f), glm::vec3(1.f)}));
		m_fracReg.AddComponent<UIElement>(curLevelText, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(curLevelText, Frac::TextComponent(
			{
				"-"
				,m_fontSize
				, m_nextLevelIndicatorColor
			}));

		curLevelText.SetParent(nextLevelAnchor);

		Frac::Entity& splitLevelText = *new Frac::Entity(m_fracReg, "splitIndicatorText");
		m_fracReg.AddComponent<TOR::Transform>(splitLevelText, TOR::Transform({glm::vec3(0.f, m_verOffset, 0.f), glm::vec3(1.f)}));
		m_fracReg.AddComponent<UIElement>(splitLevelText, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(splitLevelText, Frac::TextComponent(
			{
				"/"
				,m_fontSize
				, m_splitIndicatorColor
			}));

		splitLevelText.SetParent(nextLevelAnchor);
		
		Frac::Entity& maxLevelText = *new Frac::Entity(m_fracReg, "maxIndicatorText");
		m_fracReg.AddComponent<TOR::Transform>(maxLevelText, TOR::Transform({glm::vec3(-m_horOffset, m_verOffset, 0.f), glm::vec3(1.f)}));
		m_fracReg.AddComponent<UIElement>(maxLevelText, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(maxLevelText, Frac::TextComponent(
			{
				"-"
				, m_fontSize
				, m_nextLevelIndicatorColor
			}));

		maxLevelText.SetParent(nextLevelAnchor);
		
		m_curLevelEnt = &curLevelText;
		m_maxLevelEnt = &maxLevelText;

		Frac::Entity& mmBackdropEnt = *new Frac::Entity(m_fracReg, "Backdrop");
		m_fracReg.AddComponent<TOR::Transform>(mmBackdropEnt, TOR::Transform({glm::vec3(0.f, 0.f, 5.f), m_backdropSize}));
		m_fracReg.AddComponent<UIElement>(mmBackdropEnt, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(mmBackdropEnt, Frac::TextComponent(
			{"S"
			, m_winLoseBackdropSize
			, m_loseBackDropCol
			, Frac::TextComponent::FontFamily::Icons }));

		mmBackdropEnt.SetParent(canvasEnt);
	}
}

void NextLevelCanvasSystem::Update(float dt)
{
	CanvasSystem::Update(dt);

	if (m_firstRun)
	{
		auto& curTextComp = m_fracReg.GetComponent<Frac::TextComponent>(*m_curLevelEnt);
		auto& maxTextComp = m_fracReg.GetComponent<Frac::TextComponent>(*m_maxLevelEnt);

		curTextComp.text = std::to_string(m_levelManager->m_activeLevel + 2);
		maxTextComp.text = std::to_string(m_levelManager->GetAmountOfLevels());

		if (m_levelManager->m_activeLevel >= m_levelManager->GetAmountOfLevels() - 1)
		{
			m_timer = m_timeUntilNextLevel;
		}
		
		m_firstRun = false;
	}

	m_timer += dt;

	if (m_timer < m_inputActivateTarget)
	{
		return;
	}
	
	if (m_timer > m_timeUntilNextLevel) // skip credits
	{
		LOGINFO("[NextLevelCanvasSystem] Entering Next Level");
		
		if (m_levelManager->m_activeLevel < m_levelManager->GetAmountOfLevels() - 1)
		{
			m_levelManager->LoadLevel(m_levelManager->m_activeLevel +=1);
			
			m_gameDirector->SwitchToGroup(GameState::WavePhase);
		}
		else {
			m_levelManager->ClearLevel();
			m_gameDirector->SwitchToGroup(GameState::CreditsState);
		}
		
		HideCanvas();
		
		Reset();
	}
}

void NextLevelCanvasSystem::Reset()
{
	m_timer = 0.f;

	m_firstRun = true;
}
