#include "TDpch.h"

#include "Canvases/VictoryCanvasSystem.h"

#include "SceneManagement/SceneManager.h"
#include "Audio/AudioManager.h"
#include "Audio/AudioComponent.h"

VictoryCanvasSystem::VictoryCanvasSystem(Frac::EngineCore& engine, LevelManager& levelManager, GameDirector* gameDirector, state gameState) :
	CanvasSystem(engine, gameDirector, gameState), m_levelManager(levelManager)
{

}

void VictoryCanvasSystem::Initialise()
{
	m_menuItems = std::vector<std::pair<std::string, std::string>>({
		{"MainMenu", "Main Menu"}
		, {"Quit", "Quit"}
	});
	
	CanvasSystem::Initialise();
}

void VictoryCanvasSystem::CreateCanvas()
{
	std::string canvasName = "VictoryCanvas";
	
	Frac::SceneManager& sceneManager = m_engine.GetSceneManager();

	Frac::Entity& canvasEnt = *(m_canvasEnt = new Frac::Entity(m_fracReg, canvasName));
	m_fracReg.AddComponent<TOR::Transform>(canvasEnt, TOR::Transform({glm::vec3(0.f)}));
	m_fracReg.AddComponent<UICanvas>(canvasEnt, UICanvas({canvasName}));
	m_fracReg.AddComponent<Frac::TextComponent>(canvasEnt, Frac::TextComponent());

	canvasEnt.SetParent(sceneManager.GetActiveScene());

	UICanvas& theCanvas = m_fracReg.GetComponent<UICanvas>(canvasEnt);
	{
		Frac::Entity& mmLogoEnt = *new Frac::Entity(m_fracReg, canvasName + "_Logo");
		m_fracReg.AddComponent<TOR::Transform>(mmLogoEnt, TOR::Transform({m_logoPos, m_logoSize}));
		m_fracReg.AddComponent<UIElement>(mmLogoEnt, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(mmLogoEnt, Frac::TextComponent(
			{"Victory!" // paint icon A
			, m_logoFontSize + 3.f
			, m_winHeaderCol
			}));

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
				Frac::Entity& buttonElement = *new Frac::Entity(m_fracReg, canvasName + "_BuildInfo");
				m_fracReg.AddComponent<TOR::Transform>(buttonElement, TOR::Transform(
					{glm::vec3(
						0.f, 
						m_winLoseButtonVerOffset - (15.f * static_cast<float>(yIncrement++)), 
						m_buttonDepth), glm::vec3(0.5f)}
				));
				
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
			{"S"
			, m_winLoseBackdropSize
			, m_winBackDropCol
			, Frac::TextComponent::FontFamily::Icons }));

		mmBackdropEnt.SetParent(canvasEnt);
	}
}

void VictoryCanvasSystem::Update(float dt)
{
	CanvasSystem::Update(dt);
	
	UICanvas& canvas = m_fracReg.GetComponent<UICanvas>(*m_canvasEnt);

	UpdateCanvasSelector(dt, canvas);

	if (m_select)
	{
		const auto uiEventName = m_fracReg.GetComponent<UIElement>(m_fracReg.TranslateENTTEntity(canvas.SelectableElements[canvas.CurSelectedElement])).EventName;

		if (!uiEventName.empty())
		{
			if (uiEventName == "MainMenu")
			{
				LOGINFO("[VictoryCanvasSystem] Entering Main Menu Phase");
				m_levelManager.ClearLevel();
				m_gameDirector->SwitchToGroup(GameState::MainMenuState);
			}
			else if (uiEventName == "Quit")
			{
				LOGINFO("[VictoryCanvasSystem] Entering Quit Phase");
				m_gameDirector->SwitchToGroup(GameState::QuitGameState);
			}

			HideCanvas();
		}
		else
		{
			LOGWARNING("CanvasSystem | VictoryCanvas select, uiEventName is empty");
		}
	}
}
