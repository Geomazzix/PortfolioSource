#include "TDpch.h"

#include "Canvases/LoadCanvasSystem.h"

#include "SceneManagement/SceneManager.h"

LoadCanvasSystem::LoadCanvasSystem(Frac::EngineCore& engine, GameDirector* gameDirector, state gameState) :
	CanvasSystem(engine, gameDirector, gameState)
	, m_infoTextEnt(nullptr)
	
{

}

void LoadCanvasSystem::Initialise()
{
	CanvasSystem::Initialise();
}

void LoadCanvasSystem::CreateCanvas()
{
	std::string canvasName = "LoadCanvas";

	Frac::SceneManager& sceneManager = m_engine.GetSceneManager();

	Frac::Entity& canvasEnt = *(m_canvasEnt = new Frac::Entity(m_fracReg, canvasName));
	m_fracReg.AddComponent<TOR::Transform>(canvasEnt, TOR::Transform({glm::vec3(0.f)}));
	m_fracReg.AddComponent<UICanvas>(canvasEnt, UICanvas({canvasName}));
	m_fracReg.AddComponent<Frac::TextComponent>(canvasEnt, Frac::TextComponent());

	canvasEnt.SetParent(sceneManager.GetActiveScene());

	UICanvas& theCanvas = m_fracReg.GetComponent<UICanvas>(canvasEnt);
	{
		Frac::Entity& mmLogoEnt = *new Frac::Entity(m_fracReg, canvasName + "_Logo");
		m_fracReg.AddComponent<TOR::Transform>(mmLogoEnt, TOR::Transform({m_logoPos + glm::vec3(0.f, 0.f, -10.f), m_logoSize}));
		m_fracReg.AddComponent<UIElement>(mmLogoEnt, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(mmLogoEnt, Frac::TextComponent(
			{"LOADING..." // paint icon A
			, m_logoFontSize + 2.f
			, glm::vec3(0.98f, 0.83f, 0.95f)
			}));

		mmLogoEnt.SetParent(canvasEnt);

		Frac::Entity& mmBackdropEnt = *new Frac::Entity(m_fracReg, canvasName + "_Background");
		m_fracReg.AddComponent<TOR::Transform>(mmBackdropEnt, TOR::Transform({ glm::vec3(0.f, -52.f, 0.f), m_backdropSize}));
		m_fracReg.AddComponent<UIElement>(mmBackdropEnt, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(mmBackdropEnt, Frac::TextComponent(
			{"S"
			, 2000.f // big boy size
			, m_backdropColor // pitch black
			, Frac::TextComponent::FontFamily::Icons }));

		mmBackdropEnt.SetParent(canvasEnt);

		// header
		{
			Frac::Entity& infoBox = *new Frac::Entity(m_fracReg, canvasName + "_BuildInfo");
			m_fracReg.AddComponent<TOR::Transform>(infoBox, TOR::Transform({glm::vec3(0.f, 15.f, m_buttonDepth), glm::vec3(0.25f)}));
			m_fracReg.AddComponent<UIElement>(infoBox, UIElement({}));
			m_fracReg.AddComponent<HealthComponent>(infoBox, HealthComponent({100, 100}));
			m_fracReg.AddComponent<Frac::TextComponent>(infoBox, Frac::TextComponent(
				{
					"GET READY"
					,1.0f
					, theCanvas.IdleColor
				}
			));

			infoBox.SetParent(mmBackdropEnt);

			Frac::Entity* HealthBar = new Frac::Entity(m_fracReg, canvasName + "_ProgressBar");
			HealthBar->SetParent(infoBox);
			m_fracReg.AddComponent<TOR::Transform>(*HealthBar, TOR::Transform{ 
				{
					0.f,-350.f,0.f
				},
				{
					1.f,1.f,1.f
				}
			});

			Frac::TextComponent textComp = Frac::TextComponent{ "S",100.f,{0.f,1.f,0.f},Frac::TextComponent::FontFamily::Icons };
			textComp.noBlend = true;
			
			m_fracReg.AddComponent<Frac::TextComponent>(*HealthBar, textComp);
			m_fracReg.AddComponent<HeathBarComponent>(*HealthBar,HeathBarComponent
				{
					true
					, glm::vec3(1.f,0.f,0.f)
					, glm::vec3(0.98f, 0.83f, 0.95f)
					, glm::vec3(0.03f,0.002f,0.5f)
				});

			m_infoTextEnt = &infoBox;
		}
	}
}

void LoadCanvasSystem::Update(float dt)
{
	CanvasSystem::Update(dt);

	UICanvas& canvas = m_fracReg.GetComponent<UICanvas>(*m_canvasEnt);
	if (m_fracReg.HasComponent<HealthComponent>(*m_infoTextEnt))
	{
		auto& healthComp = m_fracReg.GetComponent<HealthComponent>(*m_infoTextEnt);
		auto& trans = m_fracReg.GetComponent<TOR::Transform>(*m_infoTextEnt);
		if(m_loadNext)
		{
			m_loadNext = false;
			if (!Frac::EngineCore::GetInstance().GetSceneManager().ProcessPreloadModels())
			{
				m_gameDirector->SwitchToGroup(GameState::MainMenuState);
				HideCanvas();	
			}
		}

		m_timer += dt;
		if(m_timer >= m_delay)
		{
			m_timer -= m_delay;
			m_loadNext = true;
		}
		
		uint64_t start, end, progress;
		Frac::EngineCore::GetInstance().GetSceneManager().RetrievePreloadProgress(start, end, progress);
		healthComp.MaxHealth = static_cast<float>(end);
		healthComp.Health = static_cast<float>(progress);
		

		if (progress >= (end - start) / 2 )
		{
			auto& headerText = m_fracReg.GetComponent<Frac::TextComponent>(*m_infoTextEnt);
			headerText.text = "ALMOST THERE...";
		}

		//auto& transChild = m_fracReg.GetComponent<TOR::Transform>(m_infoTextEnt->GetChild(0));
		//transChild.EulerOrientation += glm::vec3(0.f, 0.f, glm::radians<float>((healthComp.Health / 100.f) * dt * 1000.f));
		//transChild.Orientation = glm::quat(transChild.EulerOrientation);
		//m_fracReg.GetEnTTRegistry().patch<TOR::Transform>(m_infoTextEnt->GetHandle());
	}
	
}