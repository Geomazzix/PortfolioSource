#include "TDpch.h"

#include "Canvases/DeveloperCanvasSystem.h"

#include "SceneManagement/SceneManager.h"

#include "Tools/Version.h"

DeveloperCanvasSystem::DeveloperCanvasSystem(CanvasSystem* gameplayCanvasSystem, Frac::EngineCore& engine, GameDirector* gameDirector, state gameState) :
	CanvasSystem(engine, gameDirector, gameState)
	, m_gameplayCanvasSystem(gameplayCanvasSystem)
{
}

void DeveloperCanvasSystem::Initialise()
{
	m_updateInput = false;
	
	CanvasSystem::Initialise();
}

void DeveloperCanvasSystem::CreateCanvas()
{
	std::string canvasName = "DeveloperCanvas";

	Frac::SceneManager& sceneManager = m_engine.GetSceneManager();

	Frac::Entity& canvasEnt = *(m_canvasEnt = new Frac::Entity(m_fracReg, canvasName));
	m_fracReg.AddComponent<TOR::Transform>(canvasEnt, TOR::Transform({glm::vec3(0.f), glm::vec3(0.f)}));
	m_fracReg.AddComponent<UICanvas>(canvasEnt, UICanvas({canvasName}));
	m_fracReg.AddComponent<Frac::TextComponent>(canvasEnt, Frac::TextComponent());

	canvasEnt.SetParent(sceneManager.GetActiveScene());

	UICanvas& theCanvas = m_fracReg.GetComponent<UICanvas>(canvasEnt);

	std::string devString =
#if defined(CONFIG_RELEASE)
		"Release";
#elif defined(CONFIG_DEVELOP)
		"Develop";
#elif defined(CONFIG_DEBUG)
		"Debug";
#else
		"Development";
#endif
	
	Frac::Entity& infoBox = *new Frac::Entity(m_fracReg, "InputInfoRoot");
	m_fracReg.AddComponent<TOR::Transform>(infoBox, TOR::Transform({glm::vec3(-170.f, m_lowTextYPoint, 0.f), glm::vec3(1.f)}));
	m_fracReg.AddComponent<UIElement>(infoBox, UIElement({}));
	m_fracReg.AddComponent<Frac::TextComponent>(infoBox, Frac::TextComponent(
		{
			devString + " Build | Version: " + Version::GetVersion()
			,0.09f
		, glm::vec3(0.5f)
		}
	));

	infoBox.SetParent(canvasEnt);
}

void DeveloperCanvasSystem::Update(float dt)
{
	CanvasSystem::Update(dt);

	// TODO find a better way of hidding the gameplay canvas on different states
	auto curState = m_gameDirector->GetCurrentState();
	if (curState != GameState::PreperationPhase && curState != GameState::WavePhase && curState != GameState::TutorialState)
	{
		m_gameplayCanvasSystem->HideCanvas();
	}
}