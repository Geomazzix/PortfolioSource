#include "TDpch.h"

#include "InputSystem/InputSystem.h"

#include "CursorSystem.h"
#include "EnemyDirectorSystem.h"
#include "Canvases/GameplayCanvasSystem.h"
#include "SceneManagement/SceneManager.h"
#include "LevelManager.h"
#include "PlayerManagerSystem.h"

GameplayCanvasSystem::GameplayCanvasSystem(TileManager* tileManager, CursorSystem* cursorSystem, EnemyDirectorSystem* enemyDirSystem, PlayerEntityCreator* playerEntCreator, LevelManager& levelManager, PlayerManagerSystem* playerManagerSystem, Frac::EngineCore& engine, GameDirector* gameDirector, state gameState) :
	CanvasSystem(engine, gameDirector, gameState)
	, m_tileManager(tileManager)
	, m_cursorSystem(cursorSystem)
	, m_enemyDirSystem(enemyDirSystem)
	, m_playerEntCreator(playerEntCreator)
	,m_levelManager(levelManager)
	, m_playerManagerSystem(playerManagerSystem)
	, m_resourceTextEnt(nullptr)
	, m_resourceElementCount(0)
	, m_prepBarEnt(nullptr)
	, m_buildIDSelected(0)
	, m_leftButton(nullptr)
	, m_rightButton(nullptr)
	, m_sellInfoTextEnt(nullptr)
	, m_prepBarInfoIcon(nullptr)
	, m_infoBoxAnchor(nullptr)
	, m_buildInfoTextEnt(nullptr)
	, m_waveAnchorEnt(nullptr)
	, m_waveElementEnts(std::vector<Frac::Entity*>())
	, m_waveElementsHighlighted(0)
{
}

void GameplayCanvasSystem::Initialise()
{
	m_towerIcons = std::vector<std::pair<std::string, glm::vec3>>({
		{"?", glm::vec3(1.f)} // machine gun
		, {"@", glm::vec3(1.f)} // 
		, {"A", glm::vec3(1.f)} // rocket
	});
	
	CanvasSystem::Initialise();
}

void GameplayCanvasSystem::CreateCanvas()
{
	std::string canvasName = "GameplayCanvas";

	Frac::SceneManager& sceneManager = m_engine.GetSceneManager();

	Frac::Entity& canvasEnt = *(m_canvasEnt = new Frac::Entity(m_fracReg, canvasName));
	m_fracReg.AddComponent<TOR::Transform>(canvasEnt, TOR::Transform({glm::vec3(0.f)}));
	m_fracReg.AddComponent<UICanvas>(canvasEnt, UICanvas({canvasName}));
	m_fracReg.AddComponent<Frac::TextComponent>(canvasEnt, Frac::TextComponent());

	canvasEnt.SetParent(sceneManager.GetActiveScene());

	CreatePrepBarElement(canvasEnt);
	CreateCounterElements(canvasEnt);
	CreateWaveElements(canvasEnt);
	CreateBuildableElements(canvasEnt);
}

void GameplayCanvasSystem::Update(float dt)
{
	CanvasSystem::Update(dt);

	UpdateBuildSelection(dt);
	UpdatePrepPhaseElements(dt);
	UpdateWaveElements();
	UpdateButtonLayout();
	UpdateBuildInfoBox(dt);
	
	RecreateWaveElements();
	
	UICanvas& canvas = m_fracReg.GetComponent<UICanvas>(*m_canvasEnt);

	// Check and set resource text
	if(m_resourceTextEnt != nullptr && m_fracReg.HasComponent<Frac::TextComponent>(*m_resourceTextEnt))
	{
		Frac::TextComponent& resourceTextComp = m_fracReg.GetComponent<Frac::TextComponent>(*m_resourceTextEnt);
		resourceTextComp.text = std::to_string(m_playerManagerSystem->GetResources());
	}

	// check if cursor is on buildable tile
	if (m_infoBoxAnchor != nullptr)
	{
		entt::entity tile = m_cursorSystem->GetSelectedTile();

		if (m_fracReg.GetEnTTRegistry().valid(tile))
		{
			const TileComponent& tileComponent = m_fracReg.GetEnTTRegistry().get<TileComponent>(m_cursorSystem->GetSelectedTile());
			
			//Is the Tile Buildable?
			if (tileComponent.States.isBuildable && !m_isInfoBoxVisible)
			{
				SetTextChildRenderState(*m_infoBoxAnchor, true);
				m_isInfoBoxVisible = true;
			}
			else if (!tileComponent.States.isBuildable && m_isInfoBoxVisible)
			{
				SetTextChildRenderState(*m_infoBoxAnchor, false);
				m_isInfoBoxVisible = false;
			}

			if (m_isInfoBoxVisible)
				UpdateSellTextColor(tile);
		}
		else if (m_isInfoBoxVisible)
		{
			SetTextChildRenderState(*m_infoBoxAnchor, false);
			m_isInfoBoxVisible = false;
		}
	}

#if defined(CONFIG_DEBUG) || defined(CONFIG_DEVELOP)
	if (m_inputSystem.GetKeyOnce(Frac::KEY_COMMA))
	{
		if (m_levelManager.m_activeLevel > 0)
		{
			m_levelManager.LoadLevel(m_levelManager.m_activeLevel - 1);
		}
	}

	if (m_inputSystem.GetKeyOnce(Frac::KEY_PERIOD))
	{
		if (m_levelManager.m_activeLevel < m_levelManager.GetAmountOfLevels() - 1)
		{
			m_levelManager.LoadLevel(m_levelManager.m_activeLevel + 1);
		}
	}
#endif // DEBUG
}

void GameplayCanvasSystem::UpdateBuildSelection(float dt)
{
	// set correct colors for buildables
	for(int i = 0; i < m_towerElementEnts.size(); i++)
	{
		Frac::Entity* towerElement = m_towerElementEnts[i];
		if (m_fracReg.HasComponent<Frac::TextComponent>(*towerElement))
		{
			Frac::TextComponent& textComp = m_fracReg.GetComponent<Frac::TextComponent>(*towerElement);

			TOR::Transform& transform = m_fracReg.GetComponent<TOR::Transform>(towerElement->GetChild(0));
			
			if(m_buildIDSelected == i)
			{
				textComp.color = m_buildSelectedCol;
				transform.Scale = glm::vec3(0.9f);
			}
			else
			{
				textComp.color = m_buildIdleCol;
				transform.Scale = glm::vec3(0.95f);
			}
		}
	}

	// handle input
	if (m_inputSystem.GetAnyJoystickButtonOnce(Frac::JOYSTICK_BUTTON_LB) || m_inputSystem.GetKeyOnce(Frac::KEY_Z))
	{
		m_buildIDSelected--;
	}
	else if (m_inputSystem.GetAnyJoystickButtonOnce(Frac::JOYSTICK_BUTTON_RB) || m_inputSystem.GetKeyOnce(Frac::KEY_X))
	{
		m_buildIDSelected++;	
	}

	// add some UX for LB
	if (m_inputSystem.GetAnyJoystickButton(Frac::JOYSTICK_BUTTON_LB) || m_inputSystem.GetKey(Frac::KEY_Z))
	{
		m_fracReg.GetComponent<Frac::TextComponent>(*m_leftButton).color = m_buildSelectedCol;
	}
	else
	{
		m_fracReg.GetComponent<Frac::TextComponent>(*m_leftButton).color = glm::vec3(m_prepbarColor);

	}
	// and the RB
	if (m_inputSystem.GetAnyJoystickButton(Frac::JOYSTICK_BUTTON_RB) || m_inputSystem.GetKey(Frac::KEY_X))
	{
		m_fracReg.GetComponent<Frac::TextComponent>(*m_rightButton).color = m_buildSelectedCol;
	}
	else
	{
		m_fracReg.GetComponent<Frac::TextComponent>(*m_rightButton).color = glm::vec3(m_prepbarColor);

	}
	
	// wrap values
	if (m_buildIDSelected < 0)
	{
		m_buildIDSelected = static_cast<int>(m_towerElementEnts.size()) - 1;
	}
	else if (m_buildIDSelected > static_cast<int>(m_towerElementEnts.size()) - 1)
	{
		m_buildIDSelected = 0;
	}
	
	m_playerManagerSystem->SetSelectedTower(m_buildIDSelected);
}

void GameplayCanvasSystem::UpdatePrepPhaseElements(float dt)
{
	switch(m_gameDirector->GetCurrentState())
	{
	case GameState::TutorialState:
	case GameState::PreperationPhase:
		{
			// update prep bar
			if(m_prepBarEnt != nullptr && m_fracReg.HasComponent<HealthComponent>(*m_prepBarEnt))
			{
				HealthComponent& healthComp = m_fracReg.GetComponent<HealthComponent>(*m_prepBarEnt);
				healthComp.Health = m_enemyDirSystem->GetPrepPhaseElapsedTimer();
			}

			if (m_inputSystem.GetKeyOnce(Frac::KEY_B) || m_inputSystem.GetAnyJoystickButtonOnce(Frac::JoystickButtons::JOYSTICK_BUTTON_Y))
			{
				m_enemyDirSystem->SetPrepPhaseElapsedTimer(m_enemyDirSystem->GetPrepPhaseTimer());
				m_gameDirector->SwitchToGroup(GameState::PreperationPhase);
			}

			// show up prep bar
			auto& barTrans = m_fracReg.GetComponent<TOR::Transform>(*m_prepBarEnt);
			auto& prepTrans = m_fracReg.GetComponent<TOR::Transform>(*m_sellInfoTextEnt);
			prepTrans.Scale = barTrans.Scale = glm::vec3(1.f);
		}
		break;
	case GameState::WavePhase:
		{
			// hide prepbar
			auto& barTrans = m_fracReg.GetComponent<TOR::Transform>(*m_prepBarEnt);
			auto& prepTrans = m_fracReg.GetComponent<TOR::Transform>(*m_sellInfoTextEnt);
			prepTrans.Scale = barTrans.Scale = glm::vec3(0.f);
		}
		break;
	default: ;
	}
}

void GameplayCanvasSystem::UpdateButtonLayout() const
{
#if defined(PLATFORM_WINDOWS)
	switch(m_inputSystem.GetLastUsedInputType())
	{
		case Frac::JoystickType::JOYSTICK_TYPE_XBOX:
		case Frac::JoystickType::JOYSTICK_TYPE_XBOX_360:
		{
			m_fracReg.GetComponent<Frac::TextComponent>(*m_leftButton).text = "c";
			m_fracReg.GetComponent<Frac::TextComponent>(*m_rightButton).text = "d";

			m_fracReg.GetComponent<Frac::TextComponent>(*m_buildInfoTextEnt).text = ",";
			m_fracReg.GetComponent<Frac::TextComponent>(*m_sellInfoTextEnt).text = "-";
				
			m_fracReg.GetComponent<Frac::TextComponent>(*m_prepBarInfoIcon).text = "/";
		}
			break;
		case Frac::JoystickType::JOYSTICK_TYPE_PLAYSTATION:
		case Frac::JoystickType::JOYSTICK_TYPE_STEAM:
		{
			m_fracReg.GetComponent<Frac::TextComponent>(*m_leftButton).text = "g";
			m_fracReg.GetComponent<Frac::TextComponent>(*m_rightButton).text = "h";

			m_fracReg.GetComponent<Frac::TextComponent>(*m_buildInfoTextEnt).text = ")";
			m_fracReg.GetComponent<Frac::TextComponent>(*m_sellInfoTextEnt).text = "*";

			m_fracReg.GetComponent<Frac::TextComponent>(*m_prepBarInfoIcon).text = "(";
		}
			break;
		case Frac::JoystickType::JOYSTICK_TYPE_NINTENDO_LEFT_JOYCON:
		case Frac::JoystickType::JOYSTICK_TYPE_NINTENDO_RIGHT_JOYCON:
		{
			m_fracReg.GetComponent<Frac::TextComponent>(*m_leftButton).text = "e";
			m_fracReg.GetComponent<Frac::TextComponent>(*m_rightButton).text = "f";

			m_fracReg.GetComponent<Frac::TextComponent>(*m_buildInfoTextEnt).text = "-";
			m_fracReg.GetComponent<Frac::TextComponent>(*m_sellInfoTextEnt).text = ",";

			m_fracReg.GetComponent<Frac::TextComponent>(*m_prepBarInfoIcon).text = ",";
		}
		break;
		case Frac::JoystickType::JOYSTICK_TYPE_NINTENDO_DUAL_JOYCONS:
		case Frac::JoystickType::JOYSTICK_TYPE_NINTENDO_FULLKEY:
		case Frac::JoystickType::JOYSTICK_TYPE_NINTENDO_HANDHELD:
		{
			m_fracReg.GetComponent<Frac::TextComponent>(*m_leftButton).text = "a";
			m_fracReg.GetComponent<Frac::TextComponent>(*m_rightButton).text = "b";

			m_fracReg.GetComponent<Frac::TextComponent>(*m_buildInfoTextEnt).text = "-";
			m_fracReg.GetComponent<Frac::TextComponent>(*m_sellInfoTextEnt).text = ",";

			m_fracReg.GetComponent<Frac::TextComponent>(*m_prepBarInfoIcon).text = ",";
		}
			break;
		case Frac::JoystickType::JOYSTICK_TYPE_VIRTUAL:
		case Frac::JoystickType::JOYSTICK_TYPE_INVALID:
		default:
		{
			m_fracReg.GetComponent<Frac::TextComponent>(*m_leftButton).text = "{";
			m_fracReg.GetComponent<Frac::TextComponent>(*m_rightButton).text = "|";

			m_fracReg.GetComponent<Frac::TextComponent>(*m_buildInfoTextEnt).text = "}";
			m_fracReg.GetComponent<Frac::TextComponent>(*m_sellInfoTextEnt).text = "w";

			m_fracReg.GetComponent<Frac::TextComponent>(*m_prepBarInfoIcon).text = "~";
		};
	}
#endif
}

void GameplayCanvasSystem::UpdateBuildInfoBox(float dt)
{
	// changes color of the Build info box, could be better
	auto& iconTextComp = m_fracReg.GetComponent<Frac::TextComponent>(*m_buildInfoTextEnt);
	auto& iconChildTextComp = m_fracReg.GetComponent<Frac::TextComponent>(m_buildInfoTextEnt->GetChild(0));
	
	if (m_playerManagerSystem->GetResources()  < GameSettings::IndexToTurretStats(m_buildIDSelected).ResourcesWorth)
	{
		iconChildTextComp.color = iconTextComp.color = m_buildInfoTextIdleColor;
	}
	else
	{
		iconChildTextComp.color = iconTextComp.color = m_resTitleTextCol;
	}
}

void GameplayCanvasSystem::CreatePrepBarElement(Frac::Entity& canvasEnt)
{
	Frac::Entity& infoBox = *new Frac::Entity(m_fracReg, canvasEnt.GetEntityName() + "_PreperationText");
	m_fracReg.AddComponent<TOR::Transform>(infoBox, TOR::Transform({glm::vec3(0.f, 120.f, 0.f), glm::vec3(1.f)}));
	m_fracReg.AddComponent<UIElement>(infoBox, UIElement({}));
	m_fracReg.AddComponent<HealthComponent>(infoBox, HealthComponent(
		{
			0.f, m_enemyDirSystem->GetPrepPhaseTimer()
		}));
	m_fracReg.AddComponent<Frac::TextComponent>(infoBox, Frac::TextComponent(
		{
			"Preparation phase"
			,0.2f
		, m_prepbarColor
		}
	));
	
	infoBox.SetParent(canvasEnt);

	Frac::Entity* prepProgressBarEnt = new Frac::Entity(m_fracReg, canvasEnt.GetEntityName() + "_PreperationProgressBarAnchor");
	m_fracReg.AddComponent<TOR::Transform>(*prepProgressBarEnt, TOR::Transform{ 
		{
			glm::vec3(0.f,-12.0f,0.f)
			},
		{
				glm::vec3(1.f,1.f,1.f)
			}
		});
	m_fracReg.AddComponent<Frac::TextComponent>(*prepProgressBarEnt, Frac::TextComponent{
		"Y",
		1.f,
		m_prepbarColor,
		Frac::TextComponent::FontFamily::Icons
		});
	m_fracReg.AddComponent<HeathBarComponent>(*prepProgressBarEnt, HeathBarComponent
		{
			true
			, m_prepbarColor
			, m_prepbarColor
			, glm::vec3(0.25f,0.025f,1.f)
		});

	prepProgressBarEnt->SetParent(infoBox);

	m_fracReg.GetComponent<Frac::TextComponent>(*prepProgressBarEnt).noBlend = true;
	
	m_prepBarEnt = &infoBox;

	Frac::Entity* prepBarIcon = new Frac::Entity(m_fracReg, "prepBarIcon");
	m_fracReg.AddComponent<TOR::Transform>(*prepBarIcon, TOR::Transform({glm::vec3(0.f, -34.f, 0.f), glm::vec3(1.f)}));
	m_fracReg.AddComponent<UIElement>(*prepBarIcon, UIElement({}));
	m_fracReg.AddComponent<Frac::TextComponent>(*prepBarIcon, Frac::TextComponent(
		{
			"."
			,0.025f
			, m_prepbarColor
			, Frac::TextComponent::FontFamily::Icons
		}
	));
	
	prepBarIcon->SetParent(infoBox);
	
	Frac::Entity* prepBarText = new Frac::Entity(m_fracReg, "prepBarText");
	m_fracReg.AddComponent<TOR::Transform>(*prepBarText, TOR::Transform({glm::vec3(0.f, 14.f, 0.f), glm::vec3(1.f)}));
	m_fracReg.AddComponent<UIElement>(*prepBarText, UIElement({}));
	m_fracReg.AddComponent<Frac::TextComponent>(*prepBarText, Frac::TextComponent(
		{
			"Skip"
			,0.15f
			, m_prepbarColor
			, 
		}
	));

	prepBarText->SetParent(*prepBarIcon);
	
	m_prepBarInfoIcon = prepBarIcon;
}

void GameplayCanvasSystem::CreateCounterElements(Frac::Entity& canvasEnt)
{
	Frac::Entity& resAnchor = *new Frac::Entity(m_fracReg, canvasEnt.GetEntityName() + "_resRootAnchor");
	m_fracReg.AddComponent<TOR::Transform>(resAnchor, TOR::Transform(
		{
			glm::vec3(-m_farScreenX, m_topYOffset, 0.f)
		}
	));
	m_fracReg.AddComponent<UIElement>(resAnchor, UIElement({}));

	resAnchor.SetParent(canvasEnt);

	Frac::Entity& resTitleText = *new Frac::Entity(m_fracReg, "resTitleText");
	m_fracReg.AddComponent<TOR::Transform>(resTitleText, TOR::Transform({glm::vec3(0.f, 0.f, 0.f)}));
	m_fracReg.AddComponent<UIElement>(resTitleText, UIElement({}));

	Frac::TextComponent textComp = Frac::TextComponent({
		"Resources"
		, m_resCountFontSize
		, m_resTitleTextCol
		, Frac::TextComponent::FontFamily::Regular });
	textComp.alignment = Frac::TextComponent::Alignment::Right;
	
	m_fracReg.AddComponent<Frac::TextComponent>(resTitleText, textComp);

	resTitleText.SetParent(resAnchor);

	Frac::Entity& resCountText = *new Frac::Entity(m_fracReg, "resCountText");
	m_fracReg.AddComponent<TOR::Transform>(resCountText, TOR::Transform({glm::vec3(0.f, -10.f, 0.f)}));
	m_fracReg.AddComponent<UIElement>(resCountText, UIElement({}));

	textComp.color = m_resCountTextCol;
	textComp.text = "---";
	
	m_fracReg.AddComponent<Frac::TextComponent>(resCountText, textComp);

	resCountText.SetParent(resAnchor);
	
	m_resourceTextEnt = &resCountText;

	// This background is used 
	Frac::TextComponent resBackdropTextComp{"Y", 0.1f, m_resTitleTextCol, Frac::TextComponent::FontFamily::Icons};
	resBackdropTextComp.noBlend = true;

	Frac::Entity& resBackdrop = *new Frac::Entity(m_fracReg, "resBackdrop");
	m_fracReg.AddComponent<TOR::Transform>(resBackdrop, TOR::Transform({glm::vec3(26.3f, -11.64f, 0.1f), glm::vec3(1.38f, 1.15f, 1.f)}));
	m_fracReg.AddComponent<UIElement>(resBackdrop, UIElement({}));
	m_fracReg.AddComponent<Frac::TextComponent>(resBackdrop, resBackdropTextComp);

	resBackdrop.SetParent(resAnchor);

	resBackdropTextComp.fontSize -= 0.0075f;
	resBackdropTextComp.color = m_infoBoxColor;
	
	Frac::Entity& resBackdropInner = *new Frac::Entity(m_fracReg, "resBackdropInner");
	m_fracReg.AddComponent<TOR::Transform>(resBackdropInner, TOR::Transform({glm::vec3(0.f, 0.79f, -0.05f), glm::vec3(1.03f, 0.95f, 1.f)}));
	m_fracReg.AddComponent<UIElement>(resBackdropInner, UIElement({}));
	m_fracReg.AddComponent<Frac::TextComponent>(resBackdropInner, resBackdropTextComp);

	resBackdropInner.SetParent(resBackdrop);
}

void GameplayCanvasSystem::CreateWaveElements(Frac::Entity& canvasEnt)
{
	Frac::Entity& progressAnchor = *new Frac::Entity(m_fracReg, "progressAnchor");
	m_fracReg.AddComponent<TOR::Transform>(progressAnchor, TOR::Transform({glm::vec3(-m_farScreenX, 76.f, 0.f)}));
	m_fracReg.AddComponent<UIElement>(progressAnchor, UIElement({}));

	progressAnchor.SetParent(canvasEnt);

	Frac::TextComponent textComp = Frac::TextComponent({
		"Wave"
		, m_resTitleFontSize
		, m_waveHighlightCol
		, Frac::TextComponent::FontFamily::Regular
		});
	textComp.alignment = Frac::TextComponent::Alignment::Right;
	
	Frac::Entity& progressTextHeader = *new Frac::Entity(m_fracReg, "progressTextHeader");
	m_fracReg.AddComponent<TOR::Transform>(progressTextHeader, TOR::Transform({glm::vec3(0.f, 10.f, 0.f), }));
	m_fracReg.AddComponent<UIElement>(progressTextHeader, UIElement({}));
	m_fracReg.AddComponent<Frac::TextComponent>(progressTextHeader, Frac::TextComponent(textComp));

	progressTextHeader.SetParent(progressAnchor);

	Frac::Entity& waveAnchorEnt = *new Frac::Entity(m_fracReg, "waveAnchorEnt");
	m_fracReg.AddComponent<TOR::Transform>(waveAnchorEnt, TOR::Transform({glm::vec3(35.5f, 0.f, 0.f)}));
	m_fracReg.AddComponent<UIElement>(waveAnchorEnt, UIElement({}));

	waveAnchorEnt.SetParent(progressAnchor);
	
	m_waveAnchorEnt = &waveAnchorEnt;
}

void GameplayCanvasSystem::CreateBuildableElements(Frac::Entity& canvasEnt)
{
	Frac::Entity& rootPrepPhaseElement = *new Frac::Entity(m_fracReg, "RootPrepPhaseElements");
	m_fracReg.AddComponent<TOR::Transform>(rootPrepPhaseElement, TOR::Transform({glm::vec3(0.f), glm::vec3(1.f)}));
	m_fracReg.AddComponent<UIElement>(rootPrepPhaseElement, UIElement({}));

	rootPrepPhaseElement.SetParent(canvasEnt);

	Frac::Entity& infoBoxAnchor = *new Frac::Entity(m_fracReg, "InfoBoxAnchor");
	m_fracReg.AddComponent<TOR::Transform>(infoBoxAnchor, TOR::Transform({glm::vec3(0.f, m_bottomYOffset + 20.f, 0.f), glm::vec3(1.f)}));
	m_fracReg.AddComponent<UIElement>(infoBoxAnchor, UIElement({}));

	infoBoxAnchor.SetParent(rootPrepPhaseElement);

	m_infoBoxAnchor = &infoBoxAnchor;
	
	{
		Frac::Entity& buildInfoIcon = *new Frac::Entity(m_fracReg, "buildInfoIcon");
		m_fracReg.AddComponent<TOR::Transform>(buildInfoIcon, TOR::Transform({glm::vec3(m_infoIconHor, 12.f, 0.f), glm::vec3(1.f)}));
		m_fracReg.AddComponent<UIElement>(buildInfoIcon, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(buildInfoIcon, Frac::TextComponent(
			{
				"-"
				,m_infoBoxIconSize
				, m_resTitleTextCol
				, Frac::TextComponent::FontFamily::Icons
			}
		));

		buildInfoIcon.SetParent(infoBoxAnchor);
		
		Frac::TextComponent buildInfoTextComp{"BUY", m_infoBoxTextSize, m_resTitleTextCol};
		buildInfoTextComp.alignment = Frac::TextComponent::Alignment::Left;
		Frac::Entity& buildInfoText = *new Frac::Entity(m_fracReg, "buildInfoText");
		m_fracReg.AddComponent<TOR::Transform>(buildInfoText, TOR::Transform({glm::vec3(m_infoTextHorOffset, 0.f, 0.f), glm::vec3(1.f)}));
		m_fracReg.AddComponent<UIElement>(buildInfoText, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(buildInfoText, buildInfoTextComp);

		buildInfoText.SetParent(buildInfoIcon);
		
		m_buildInfoTextEnt = &buildInfoIcon;
	}

	{
		Frac::Entity& sellInfoIcon = *new Frac::Entity(m_fracReg, "sellInfoIcon");
		m_fracReg.AddComponent<TOR::Transform>(sellInfoIcon, TOR::Transform({glm::vec3(m_infoIconHor, 0.f, 0.f), glm::vec3(1.f)}));
		m_fracReg.AddComponent<UIElement>(sellInfoIcon, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(sellInfoIcon, Frac::TextComponent(
			{
				","
				,m_infoBoxIconSize
				, m_buildInfoTextIdleColor
				, Frac::TextComponent::FontFamily::Icons
			}
		));

		sellInfoIcon.SetParent(infoBoxAnchor);

		Frac::TextComponent sellInfoTextComp{"SELL", m_infoBoxTextSize, m_buildInfoTextIdleColor};
		sellInfoTextComp.alignment = Frac::TextComponent::Alignment::Left;
		Frac::Entity& sellInfoText = *new Frac::Entity(m_fracReg, "sellInfoText");
		m_fracReg.AddComponent<TOR::Transform>(sellInfoText, TOR::Transform({glm::vec3(m_infoTextHorOffset, 0.f, 0.f), glm::vec3(1.f)}));
		m_fracReg.AddComponent<UIElement>(sellInfoText, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(sellInfoText, sellInfoTextComp);

		sellInfoText.SetParent(sellInfoIcon);
		
		m_sellInfoTextEnt = &sellInfoIcon;
	}

	Frac::TextComponent infoBoxBackdropTextComp{"S", 0.1f, m_resTitleTextCol, Frac::TextComponent::FontFamily::Icons};
	infoBoxBackdropTextComp.noBlend = true;
	
	Frac::Entity& infoBoxBackdrop = *new Frac::Entity(m_fracReg, "infoBoxBackdrop");
	m_fracReg.AddComponent<TOR::Transform>(infoBoxBackdrop, TOR::Transform({glm::vec3(-1.0f, 5.63f, 0.1f), glm::vec3(1.35f, 0.85f, 1.f)}));
	m_fracReg.AddComponent<UIElement>(infoBoxBackdrop, UIElement({}));
	m_fracReg.AddComponent<Frac::TextComponent>(infoBoxBackdrop, infoBoxBackdropTextComp);

	infoBoxBackdrop.SetParent(infoBoxAnchor);

	infoBoxBackdropTextComp.fontSize -= 0.0075f;
	infoBoxBackdropTextComp.color = m_infoBoxColor;

	Frac::Entity& resBackdropInner = *new Frac::Entity(m_fracReg, "resBackdropInner");
	m_fracReg.AddComponent<TOR::Transform>(resBackdropInner, TOR::Transform({glm::vec3(0.f, 0.0f, -0.05f), glm::vec3(1.03f, 1.f, 1.f)}));
	m_fracReg.AddComponent<UIElement>(resBackdropInner, UIElement({}));
	m_fracReg.AddComponent<Frac::TextComponent>(resBackdropInner, infoBoxBackdropTextComp);

	resBackdropInner.SetParent(infoBoxBackdrop);

	// the buildable towers
	Frac::Entity& rootBuildableElements = *new Frac::Entity(m_fracReg, "RootBuildableElements");
	m_fracReg.AddComponent<TOR::Transform>(rootBuildableElements, TOR::Transform(
		{
			glm::vec3(0.f, m_bottomYOffset - 5.f, 0.f),
			glm::vec3(1.f)
		}));
	m_fracReg.AddComponent<UIElement>(rootBuildableElements, UIElement({}));

	rootBuildableElements.SetParent(rootPrepPhaseElement);
	
	for (int i = 0; i < m_towerIcons.size(); i++)
	{
		Frac::Entity& buildAnchor = *new Frac::Entity(m_fracReg, "buildableElementsAnchor");
		m_fracReg.AddComponent<TOR::Transform>(buildAnchor, TOR::Transform(
			{
				glm::vec3((static_cast<float>(m_towerIcons.size() + -1) * 15.f) - (static_cast<float>(i) * 30.f), 0.f, 0.f)
			}));
		m_fracReg.AddComponent<UIElement>(buildAnchor, UIElement({}));

		buildAnchor.SetParent(rootBuildableElements);
		
		Frac::Entity& builableIcon = *new Frac::Entity(m_fracReg, "BuilableIcon");
		m_fracReg.AddComponent<TOR::Transform>(builableIcon, TOR::Transform({glm::vec3(0.f, 4.f, -2.f), glm::vec3(1.f, 1.f, 1.f)}));
		m_fracReg.AddComponent<UIElement>(builableIcon, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(builableIcon, Frac::TextComponent(
			{m_towerIcons[i].first // square
			, 0.03f
			, m_towerIcons[i].second
			, Frac::TextComponent::FontFamily::Icons
			, false
			}));

		builableIcon.SetParent(buildAnchor);

		Frac::Entity& costText = *new Frac::Entity(m_fracReg, "costText");
		m_fracReg.AddComponent<TOR::Transform>(costText, TOR::Transform({glm::vec3(0.f, -8.5f, -2.f), glm::vec3(1.f, 1.f, 1.f)}));
		m_fracReg.AddComponent<UIElement>(costText, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(costText, Frac::TextComponent(
			{ std::to_string(GameSettings::IndexToTurretStats(i).ResourcesWorth)
			, 0.1f
			, m_towerIcons[i].second
			, Frac::TextComponent::FontFamily::Regular
			, false
			}));

		costText.SetParent(buildAnchor);
		
		Frac::Entity& buildableBackground = *new Frac::Entity(m_fracReg, "BuildableBackground");
		m_fracReg.AddComponent<TOR::Transform>(buildableBackground, TOR::Transform({glm::vec3(0.f)}));
		m_fracReg.AddComponent<UIElement>(buildableBackground, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(buildableBackground, Frac::TextComponent(
			{"S" // square
			, 0.0825f
			, glm::vec3(0.6f)
			, Frac::TextComponent::FontFamily::Icons
			, false
			}));

		buildableBackground.SetParent(buildAnchor);

		Frac::Entity& buildableBackgroundInner = *new Frac::Entity(m_fracReg, "buildableBackgroundInner");
		m_fracReg.AddComponent<TOR::Transform>(buildableBackgroundInner, TOR::Transform({glm::vec3(0.f, 0.f, -0.01f)}));
		m_fracReg.AddComponent<UIElement>(buildableBackgroundInner, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(buildableBackgroundInner, Frac::TextComponent(
			{"S" // square
			, 0.078f
			, glm::vec3(0.2f)
			, Frac::TextComponent::FontFamily::Icons
			, false
			}));

		buildableBackgroundInner.SetParent(buildableBackground);
		
		m_towerElementEnts.push_back(&buildableBackground);
	}
	
	Frac::Entity& leftInputHelper = *new Frac::Entity(m_fracReg, "leftInputHelper");
	m_fracReg.AddComponent<TOR::Transform>(leftInputHelper, TOR::Transform({glm::vec3(65.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f)}));
	m_fracReg.AddComponent<UIElement>(leftInputHelper, UIElement({}));
	m_fracReg.AddComponent<Frac::TextComponent>(leftInputHelper, Frac::TextComponent(
		{"a"
		, 0.04f
		, m_buildSelectedCol
		, Frac::TextComponent::FontFamily::Icons
		, false
		}));

	leftInputHelper.SetParent(rootBuildableElements);

	Frac::Entity& rightInputHelper = *new Frac::Entity(m_fracReg, "rightInputHelper");
	m_fracReg.AddComponent<TOR::Transform>(rightInputHelper, TOR::Transform({glm::vec3(-65.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f)}));
	m_fracReg.AddComponent<UIElement>(rightInputHelper, UIElement({}));
	m_fracReg.AddComponent<Frac::TextComponent>(rightInputHelper, Frac::TextComponent(
		{"b"
		, 0.04f
		, m_buildSelectedCol
		, Frac::TextComponent::FontFamily::Icons
		, false
		}));

	rightInputHelper.SetParent(rootBuildableElements);

	m_leftButton = &leftInputHelper;
	m_rightButton = &rightInputHelper;
}

void GameplayCanvasSystem::UpdateWaveElements()
{
	// color them
	if (m_waveElementsHighlighted < m_enemyDirSystem->GetCurrentWave())
	{
		m_waveElementsHighlighted++;
		
		for (int i = 0; i < m_waveElementEnts.size(); i++)
		{
			// don't highlight wave elements we haven't reached
			if (i >= m_enemyDirSystem->GetCurrentWave())
			{
				return;
			}

			const auto element = m_waveElementEnts[i];

			// highlight wave elements that need highlighting
			if(m_fracReg.HasComponent<TOR::Transform>(element->GetChild(0)))
			{
				auto& transform = m_fracReg.GetComponent<TOR::Transform>(element->GetChild(0));
				transform.Scale = glm::vec3(1.f);
			}
		}
	}
}

void GameplayCanvasSystem::RecreateWaveElements()
{
	if(m_waveAnchorEnt == nullptr)
	{
		return;
	}

	// Recreate wave elements where applicable
	if (static_cast<int>(m_waveElementEnts.size()) != m_enemyDirSystem->GetMaxWaves() 
		|| m_enemyDirSystem->GetCurrentWave() < m_waveElementsHighlighted)
	{
		// clear wave elements if max waves has changed
		for (auto element : m_waveElementEnts)
		{
			m_fracReg.GetEnTTRegistry().destroy(element->GetHandle());
		}
		
		m_waveElementEnts.clear();
		m_waveElementsHighlighted = 0;

		// create wave elements
		for (int i = 0; i < m_enemyDirSystem->GetMaxWaves(); i++)
		{
			Frac::Entity& waveElementInner = *new Frac::Entity(m_fracReg, "waveElementInner_" + std::to_string(i));
			m_fracReg.AddComponent<TOR::Transform>(waveElementInner, TOR::Transform(
				{
					glm::vec3(i * m_waveHorOffset, 0.f, 0.f),
					glm::vec3(1.f, 1.f, 1.f)
				}
			));
			
			m_fracReg.AddComponent<UIElement>(waveElementInner, UIElement({}));
			
			m_fracReg.AddComponent<Frac::TextComponent>(waveElementInner, Frac::TextComponent(
				{"]" // square
				, 0.015f
				, m_waveIdleCol
				, Frac::TextComponent::FontFamily::Icons
				, false
				}));

			
			Frac::Entity& waveElementOuter = *new Frac::Entity(m_fracReg, "waveElementOuter");
			m_fracReg.AddComponent<TOR::Transform>(waveElementOuter, TOR::Transform(
				{
					glm::vec3(0.f),
					glm::vec3(0.f)
				}
			));

			m_fracReg.AddComponent<UIElement>(waveElementOuter, UIElement({}));

			m_fracReg.AddComponent<Frac::TextComponent>(waveElementOuter, Frac::TextComponent(
				{"[" // square
				, 0.0125f
				, m_waveHighlightCol
				, Frac::TextComponent::FontFamily::Icons
				, false
				}));
			
			waveElementInner.SetParent(*m_waveAnchorEnt);
			waveElementOuter.SetParent(waveElementInner);

			m_waveElementEnts.push_back(&waveElementInner);
		}
	}
}

void GameplayCanvasSystem::HandlePopupEntity(Frac::Entity target)
{
	auto& enttReg = m_fracReg.GetEnTTRegistry();

	const std::string& entName = target.GetEntityName();

	TOR::Transform& popupTransform = m_fracReg.GetComponent<TOR::Transform>(m_fracReg.TranslateENTTEntity(m_activePopupEnt));
	TOR::Transform& targetTransform = m_fracReg.GetComponent<TOR::Transform>(target);

	popupTransform.Position = targetTransform.Position + m_popupOffset;

	UIPopUp& uiPopup = m_fracReg.GetComponent<UIPopUp>(m_fracReg.TranslateENTTEntity(m_activePopupEnt));

	// set data
	Frac::TextComponent& textHeader = m_fracReg.GetComponent<Frac::TextComponent>(m_fracReg.TranslateENTTEntity(uiPopup.InfoElements["TextHeader"]));
	textHeader.text = entName.empty() ? "Unnamed" : entName;
}

void GameplayCanvasSystem::UpdateSellTextColor(entt::entity tile) const
{
	// changes color of the Sell info box, could be better
	auto& iconTextComp = m_fracReg.GetComponent<Frac::TextComponent>(*m_sellInfoTextEnt);
	auto& iconChildTextComp = m_fracReg.GetComponent<Frac::TextComponent>(m_sellInfoTextEnt->GetChild(0));
	
	const entt::entity& tileOccupier = m_tileManager->GetSpecificTileOccupier(tile, false);
	if (m_fracReg.GetEnTTRegistry().valid(tileOccupier) && m_fracReg.GetEnTTRegistry().any_of<TurretComponent>(tileOccupier))
	{
		iconChildTextComp.color = iconTextComp.color = m_resTitleTextCol;
	}
	else
	{
		iconChildTextComp.color = iconTextComp.color = m_buildInfoTextIdleColor;
	}
}
