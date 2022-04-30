#include "TDpch.h"
#include "HealthSystem.h"
#include "GameDirector.h"
#include "PlayerManagerSystem.h"
#include "Graphics/Components/TextComponent.h"
#include "Glm/gtx/compatibility.hpp"

#include "Audio/AudioComponent.h"
#include "Audio/AudioManager.h"

HealthSystem::HealthSystem(GameDirector& gameDirector, PlayerManagerSystem& playerManager, TileManager& tileManager) : 
	ISystem{Frac::eStage::OnUpdate}
, m_gameDirector(gameDirector)
, m_playerManagerSystem(playerManager)
, m_tileManager(tileManager)
{
	gameDirector.AssignSystemToStates(this, GameState::WavePhase);
	gameDirector.AssignSystemToStates(this, GameState::PreperationPhase | GameState::LoadGameState | GameState::TutorialState);
	m_info.enabled = true;
}

void HealthSystem::Update(float DeltaTime)
{
	entt::registry& reg = Frac::EngineCore::GetInstance().GetEnttRegistry();
	auto entitiesWithHealthComponents = reg.view<HealthComponent, BoxComponent, GridComponent>();
	entitiesWithHealthComponents.each([this, &reg](const auto entity, const HealthComponent& healthComponent, BoxComponent& boxComponent, GridComponent& gridComponent)
		{
			if (healthComponent.Health <= 0)
			{
				if (reg.any_of<BaseComponent>(entity))
				{
					reg.remove_if_exists<BoxComponent>(entity);
					m_gameDirector.SwitchToGroup(GameState::LoseState);
				}
				else
				{
					if (reg.any_of<ResourceComponent>(entity))
					{
						ResourceComponent& resourceComp = reg.get<ResourceComponent>(entity);
						m_playerManagerSystem.AddResources(resourceComp.ResourcesWorth);
						reg.remove<ResourceComponent>(entity);
					}
					if (reg.any_of<EnemyComponent>(entity))
					{
						EnemyComponent& enemyComp = reg.get<EnemyComponent>(entity);
						reg.emplace<DeathComponent>(entity);
						reg.remove<EnemyComponent>(entity);
					}

					m_tileManager.RemoveTileOccupier(m_tileManager.GetTile(gridComponent.GridPos.x, gridComponent.GridPos.y), entity);

					Frac::EntityRegistry& fracReg = Frac::EngineCore::GetInstance().GetFracRegistry();
					Frac::Entity currentTile = fracReg.TranslateENTTEntity(m_tileManager.GetTile(gridComponent.GridPos.x, gridComponent.GridPos.y));
					if (!fracReg.HasComponent<Frac::AudioSourceComponent>(currentTile))
					{
						fracReg.AddComponent<Frac::AudioSourceComponent>(currentTile);
					}

					Frac::AudioSourceComponent& audio = fracReg.GetComponent<Frac::AudioSourceComponent>(currentTile);
					if (!Frac::EngineCore::GetInstance().GetAudioManager().HasEvent(audio, "event:/Enemy_Death"))
					{
						Frac::EngineCore::GetInstance().GetAudioManager().AddEvent(audio, "event:/Enemy_Death");
					}
					Frac::EngineCore::GetInstance().GetAudioManager().PlayEvent(audio, "event:/Enemy_Death");
					reg.remove<GridComponent>(entity);
				}
			}
		});

	auto HealthBarView = reg.view<Frac::TextComponent, TOR::Transform,HeathBarComponent>();
	HealthBarView.each([this, &reg](const auto entity, Frac::TextComponent& textComponent, TOR::Transform& transform,const HeathBarComponent& hbc)
	{
		Frac::Entity& e = Frac::EngineCore::GetInstance().GetFracRegistry().TranslateENTTEntity(entity);
		Frac::Entity& p = e.GetParent();
		if(reg.any_of<HealthComponent>(p.GetHandle()))
		{
			const TOR::Transform tp = reg.get<TOR::Transform>(p.GetHandle());
			const HealthComponent& hp = reg.get<HealthComponent>(p.GetHandle());
			float percent{ glm::clamp(glm::max(hp.Health,0.f) / hp.MaxHealth,0.f,1.f) };
			if (hbc.overrideScale)
			{
				transform.Scale = { (hbc.invertLoadDirection ? 1.f - percent : percent) * (hbc.ComponentSize.x/tp.Scale.x),hbc.ComponentSize.y/tp.Scale.y ,hbc.ComponentSize.z/tp.Scale.z };
			}
			else
			{
				transform.Scale = { (hbc.invertLoadDirection ? 1.f - percent : percent) * hbc.ComponentSize.x,hbc.ComponentSize.y ,hbc.ComponentSize.z };
			}
			
			textComponent.color = glm::lerp(hbc.emptyColor, hbc.fullColor, (hbc.invertLoadDirection ? 1.f - percent : percent));
			reg.patch<TOR::Transform>(entity);
		}
	});
	
}
