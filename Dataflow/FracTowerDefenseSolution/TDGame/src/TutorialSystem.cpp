#include "TDpch.h"
#include "TutorialSystem.h"

#include "CursorSystem.h"
#include "TileManager.h"
#include "Graphics/ParticleRenderSystem/ParticleSystem.h"
#include "InputSystem/InputSystem.h"
#include "GameDirector.h"
#include "Graphics/Components/TextComponent.h"
#include "SceneManagement/SceneManager.h"

TutorialSystem::TutorialSystem(TileManager* tileManager, GameDirector* gameDirector,CursorSystem* cursorSystem):ISystem{Frac::eStage::OnUpdate},
                                                                                                                m_tileManager{tileManager},m_gameDirector{gameDirector},m_cursorSystem{cursorSystem},m_Text{nullptr}
{
}

TutorialSystem::~TutorialSystem()
{
}

void TutorialSystem::Update(float DeltaTime)
{
	if(m_gameDirector->GetCurrentState() == GameState::TutorialState)
	{
		if (m_tile == entt::null)
		{
			if (m_Text == nullptr)
			{
				Frac::EntityRegistry& fracReg = Frac::EngineCore::GetInstance().GetFracRegistry();
				m_Text = new Frac::Entity{ Frac::EngineCore::GetInstance().GetFracRegistry(),"TutorialText" };
				fracReg.AddComponent<TOR::Transform>(*m_Text, TOR::Transform{ {},{0.1f,0.1f,0.1f} });
				m_Text->SetParent(Frac::EngineCore::GetInstance().GetSceneManager().GetActiveScene());
				fracReg.AddComponent<Frac::TextComponent>(*m_Text, Frac::TextComponent{ "",0.02f,{1.f,1.f,1.f},Frac::TextComponent::FontFamily::Icons,true });
			}
			glm::vec2 gp = m_tileManager->WorldToGrid(glm::vec3{ 11.4f,0.f,-11.4f });
			m_tile = m_tileManager->GetTile((int)gp.x, (int)gp.y);
			Frac::EntityRegistry& fracReg = Frac::EngineCore::GetInstance().GetFracRegistry();
			TOR::Transform& tileTransform = fracReg.GetEnTTRegistry().get<TOR::Transform>(m_tile);
			entt::entity activeCam = entt::null;
			Frac::EngineCore::GetInstance().GetEnttRegistry().view<TOR::Camera>().each(
				[this, &activeCam](TOR::Camera& camera) mutable
				{
					if (camera.IsActive)
					{
						activeCam = entt::to_entity(Frac::EngineCore::GetInstance().GetEnttRegistry(), camera);
					}
				});
			TOR::Transform& transform = fracReg.GetComponent<TOR::Transform>(*m_Text);
			transform.Orientation = glm::quatLookAt(glm::normalize(Frac::EngineCore::GetInstance().GetEnttRegistry().get<TOR::Transform>(activeCam).Position - tileTransform.Position), { 0.f,1.f,0.f });
			transform.Position = tileTransform.Position + glm::vec3{ 0.f,2.f,0.f };
			fracReg.GetEnTTRegistry().patch<TOR::Transform>(m_Text->GetHandle());
		}
		else
		{
#ifdef PLATFORM_WINDOWS
			switch (Frac::EngineCore::GetInstance().GetInputSystem().GetLastUsedInputType())
			{
			case Frac::JoystickType::JOYSTICK_TYPE_XBOX:
			case Frac::JoystickType::JOYSTICK_TYPE_XBOX_360:
				if (m_boughtTurret == false)
				{
					Frac::EngineCore::GetInstance().GetFracRegistry().GetComponent<Frac::TextComponent>(*m_Text).text = ".";
				}
				else
				{
					Frac::EngineCore::GetInstance().GetFracRegistry().GetComponent<Frac::TextComponent>(*m_Text).text = "-";
				}
				break;
			case Frac::JoystickType::JOYSTICK_TYPE_PLAYSTATION:
			case Frac::JoystickType::JOYSTICK_TYPE_STEAM:

			case Frac::JoystickType::JOYSTICK_TYPE_VIRTUAL:
				break;
			case Frac::JoystickType::JOYSTICK_TYPE_INVALID:
				if (m_boughtTurret == false)
				{
					Frac::EngineCore::GetInstance().GetFracRegistry().GetComponent<Frac::TextComponent>(*m_Text).text = "}";
				}
				else
				{
					Frac::EngineCore::GetInstance().GetFracRegistry().GetComponent<Frac::TextComponent>(*m_Text).text = "w";
				}
				break;
			}
#else
			if (m_boughtTurret == false)
			{
				Frac::EngineCore::GetInstance().GetFracRegistry().GetComponent<Frac::TextComponent>(*m_Text).text = ",";
			}
			else
			{
				Frac::EngineCore::GetInstance().GetFracRegistry().GetComponent<Frac::TextComponent>(*m_Text).text = "-";
			}
#endif


			m_spawnTimer += DeltaTime;
			if (m_spawnTimer >= m_spawnDelay)
			{
				m_spawnTimer = 0.f;
				Frac::CircleParticleEmitterSettings settings;
				settings.radius = 0.5f;
				settings.TimePerEmit = 0.005f;
				settings.StartSize = glm::vec3(0.03f);
				settings.Endsize = glm::vec3(0.015f);
				settings.StartColor = glm::vec4(1, 0, 1, 1);
				settings.EndColor = glm::vec4(0.1, 0.0, 0.1, 1);
				settings.IsEmissive = true;
				settings.ParticleSpeed = 1.f;
				settings.TimeToEmit = 1.75f;
				settings.ParticleLifetime = 0.25f;
				settings.HasGravity = false;
				Frac::EngineCore::GetInstance().GetParticleSystem().SpawnCircleEmitter(settings, Frac::EngineCore::GetInstance().GetFracRegistry().TranslateENTTEntity(m_tile));
			}
			Frac::InputSystem& inputSystem = Frac::EngineCore::GetInstance().GetInputSystem();
			if ((inputSystem.GetKey(Frac::KEY_E)) || inputSystem.GetAnyJoystickButton(Frac::JOYSTICK_BUTTON_A))
			{
				if (m_cursorSystem->GetSelectedTile() == m_tile)
				{
					m_boughtTurret = true;
				}
			}
			if ((inputSystem.GetKey(Frac::KEY_Q)) || inputSystem.GetAnyJoystickButton(Frac::JOYSTICK_BUTTON_B))
			{
				if (m_cursorSystem->GetSelectedTile() == m_tile)
				{
					m_soldTurret = true;
				}
			}
		}
	}
	else if(m_gameDirector->GetCurrentState() == GameState::CreditsState)
	{
		if(m_Text != nullptr && Frac::EngineCore::GetInstance().GetEnttRegistry().valid(m_Text->GetHandle()))
			Frac::EngineCore::GetInstance().GetFracRegistry().GetComponent<Frac::TextComponent>(*m_Text).text = "";
	}
	else
	{
		m_boughtTurret = true;
		m_soldTurret = true;
	}
	if (m_boughtTurret && m_soldTurret)
	{
		if(m_Text != nullptr)
		{
			Frac::EngineCore::GetInstance().GetEnttRegistry().destroy(m_Text->GetHandle());
			m_Text = nullptr;
			m_tile = entt::null;
			m_gameDirector->SwitchToGroup(GameState::PreperationPhase);
			m_gameDirector->RemoveSystemFromStates(this, GameState::TutorialState | GameState::PreperationPhase | GameState::WavePhase);
			m_info.enabled = false;
		}
	}
}
