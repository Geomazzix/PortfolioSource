#include "TDpch.h"
#include "PlayerManagerSystem.h"
#include "InputSystem/InputSystem.h"
#include "SceneManagement/SceneManager.h"
#include "GameDirector.h"
#include "CursorSystem.h"
#include "Audio/AudioComponent.h"
#include "Audio/AudioManager.h"
#include "Graphics/ParticleRenderSystem/ParticleSystem.h"

PlayerManagerSystem::PlayerManagerSystem(TileManager& tileManager, GameDirector& gameDirector, CursorSystem& cursorSystem, PlayerEntityCreator& playerEntityCreator) :
	m_tileManager(tileManager)
	, m_gameDirector(gameDirector)
	, m_cursorSystem(cursorSystem)
	, ISystem{ Frac::eStage::OnUpdate }
	, m_playerEntityCreator(playerEntityCreator)
	, m_inputSystem(Frac::EngineCore::GetInstance().GetInputSystem())
{
	m_resources = 0;
	m_selectedTower = 0;
	gameDirector.AssignSystemToStates(this, GameState::AllState);
}

void PlayerManagerSystem::Update(float DeltaTime)
{
	state currentState = m_gameDirector.GetCurrentState();

	//Play State
	if ((currentState == GameState::PreperationPhase) || (currentState == GameState::WavePhase) || (currentState == GameState::TutorialState))
	{
		//Pause Game
		if ((m_inputSystem.GetKeyOnce(Frac::KEY_ESCAPE)) || m_inputSystem.GetAnyJoystickButtonOnce(Frac::JOYSTICK_BUTTON_START) 
		)
		{		
			m_gameDirector.SwitchToGroup(GameState::PauseState);
			LOGINFO("[Player Manager] Game Paused");	
		}

		//Buy Turret Action
		else if ((m_inputSystem.GetKeyOnce(Frac::KEY_E)) || m_inputSystem.GetAnyJoystickButtonOnce(Frac::JOYSTICK_BUTTON_A))
		{
			BuildAction();
		}

		else if (currentState == GameState::PreperationPhase || (currentState == GameState::TutorialState))
		{			
			//Sell Turret Action
			if ((m_inputSystem.GetKeyOnce(Frac::KEY_Q)) || m_inputSystem.GetAnyJoystickButtonOnce(Frac::JOYSTICK_BUTTON_B))
			{
				SellAction();
			}
		}
		ShowTurretRange(DeltaTime);
	}

	//Quit Game
	else if (currentState == GameState::QuitGameState)
	{
		LOGINFO("[Player Manager] Quitting Game");
		Frac::EngineCore::GetInstance().SetRunState(false);
	}
	
}

int PlayerManagerSystem::GetResources()
{
	return m_resources;
}

void PlayerManagerSystem::ShowTurretRange(float DeltaTime)
{
	bool newTile = false;
	entt::entity selectedTile = m_cursorSystem.GetSelectedTile();
	if(m_selectedTile != selectedTile)
	{
		newTile = true;
		m_selectedTile = selectedTile;
	}

	bool newTowerSelected = false;
	if(m_selectedTower != m_oldSelectedTower)
	{
		m_oldSelectedTower = m_selectedTower;
		newTowerSelected = true;
	}
	
	m_showRangeTimer += DeltaTime;
	if(m_showRangeTimer >= m_showRangeDelay || newTile || newTowerSelected)
	{
		m_showRangeTimer = 0;

		entt::registry& enttReg = Frac::EngineCore::GetInstance().GetEnttRegistry();
		entt::entity tileEntity = m_cursorSystem.GetSelectedTile();

		//Is our selected Tile valid?
		if (tileEntity != entt::null)
		{
			//Is the Tile Available?
			if (!m_tileManager.IsTileOccupied(tileEntity))
			{
				const TileComponent tileComponent = enttReg.get<TileComponent>(tileEntity);

				//Is the Tile Buildable?
				if (tileComponent.States.isBuildable)
				{
					TurretStats turretStats = GameSettings::IndexToTurretStats(static_cast<int>(m_selectedTower));
					Frac::CircleParticleEmitterSettings settings;
					settings.radius = turretStats.Range;
					settings.TimePerEmit = 0.0005f;
					settings.StartSize = glm::vec3(0.03f);
					settings.Endsize = glm::vec3(0.015f);
					settings.StartColor = glm::vec4(0, 0, 1, 1);
					settings.EndColor = glm::vec4(0, 0.5, 0.5, 1);
					settings.IsEmissive = true;
					settings.ParticleSpeed = 0.0001f;
					settings.TimeToEmit = 0.1f;
					settings.ParticleLifetime = 0.25f;
					settings.HasGravity = true;
					Frac::EngineCore::GetInstance().GetParticleSystem().SpawnCircleEmitter(settings, Frac::EngineCore::GetInstance().GetFracRegistry().TranslateENTTEntity(tileEntity));
				}
			}
		}
		
	}
	
}

void PlayerManagerSystem::SetResources(int value)
{
	if (value >= 0)
	{
		m_resources = value;
	}
}

void PlayerManagerSystem::AddResources(int toAdd)
{
	if ((m_resources + toAdd) >= 0) //Just to be safe. Maybe someone will add a negative number :p
	{
		m_resources += toAdd;
	}
	else
	{
		m_resources = 0;
	}
}

void PlayerManagerSystem::RemoveResources(int toRemove)
{
	if ((m_resources - toRemove) >= 0)
	{
		m_resources -= toRemove;
	}
	else
	{
		m_resources = 0;
	}

	LOGINFO("[Player Manager] Resources: %i", m_resources);
}

void PlayerManagerSystem::SellAction()
{
	entt::registry& enttReg = Frac::EngineCore::GetInstance().GetEnttRegistry();
	entt::entity tileEntity = m_cursorSystem.GetSelectedTile();

	//Is our selected Tile valid?
	if (tileEntity != entt::null && m_tileManager.IsTileOccupied(tileEntity))
	{
		entt::entity tileOccupier = m_tileManager.GetSpecificTileOccupier(tileEntity, false);

		//Is the Occupier a Turret?
		if ((tileOccupier != entt::null) && (enttReg.any_of<TurretComponent>(tileOccupier)))
		{
			TurretComponent& turret = enttReg.get<TurretComponent>(tileOccupier);
			if (!turret.SpawnState && !turret.DespawnState)
			{
				//Add Resources
				ResourceComponent& resComp = enttReg.get<ResourceComponent>(tileOccupier);
				AddResources(resComp.ResourcesWorth);

				LOGINFO("[Player Manager] Sold Turret");
				Frac::EngineCore::GetInstance().GetAudioManager().PlayEvent(enttReg.get<Frac::AudioSourceComponent>(tileEntity), "event:/Tower_Sell");

				turret.DespawnState = true;
			}
		}
	}
}

void PlayerManagerSystem::BuildAction()
{
	entt::registry& enttReg = Frac::EngineCore::GetInstance().GetEnttRegistry();
	Frac::EntityRegistry& fracReg = Frac::EngineCore::GetInstance().GetFracRegistry();
	entt::entity tileEntity = m_cursorSystem.GetSelectedTile();
	
	//Is our selected Tile valid?
	if (tileEntity != entt::null)
	{
		
		//Is the Tile Available?
		if (!m_tileManager.IsTileOccupied(tileEntity))
		{
			const TileComponent tileComponent = enttReg.get<TileComponent>(tileEntity);

			//Does the Player have enough resources to buy a Turret 
			if (GameSettings::IndexToTurretStats(m_selectedTower).ResourcesWorth <= m_resources)
			{

				//Is the Tile Buildable?
				if (tileComponent.States.isBuildable)
				{
					//Create Turret (Temporary Code until Entity Creators are in)
					glm::ivec2 gridPos = enttReg.get<GridComponent>(tileEntity).GridPos;
					m_playerEntityCreator.CreateTower(m_selectedTower, gridPos);
				
					//Remove Resources
					RemoveResources(GameSettings::IndexToTurretStats(m_selectedTower).ResourcesWorth);
					LOGINFO("[Player Manager] Build Turret");

					Frac::Entity tileFracEntity = fracReg.TranslateENTTEntity(tileEntity);
					if (!fracReg.HasComponent<Frac::AudioSourceComponent>(tileFracEntity))
					{
						fracReg.AddComponent<Frac::AudioSourceComponent>(tileFracEntity, Frac::AudioSourceComponent());
						auto& audioSourceComponent = fracReg.GetComponent<Frac::AudioSourceComponent>(tileFracEntity);
						Frac::EngineCore::GetInstance().GetAudioManager().AddEvent(audioSourceComponent, "event:/Tower_Place");
						Frac::EngineCore::GetInstance().GetAudioManager().AddEvent(audioSourceComponent, "event:/Tower_Sell");
					}
					Frac::EngineCore::GetInstance().GetAudioManager().PlayEvent(fracReg.GetComponent<Frac::AudioSourceComponent>(tileFracEntity), "event:/Tower_Place");
				}
				else
				{
					LOGINFO("[Player Manager] Cannot build Turret on path");
				}
			}
			else
			{
				LOGINFO("[Player Manager] Not enough resources to buy a Turret");
			}
		}
	}
}

void PlayerManagerSystem::SetSelectedTower(int value)
{
	if (value >= 0)
	{
		m_selectedTower = value;
	}
}
