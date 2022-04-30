#include "TDpch.h"
#include "BulletSystem.h"
#include "TileManager.h"
#include "UtilityTD.h"
#include "CoreRenderAPI/Components/Transform.h"
#include "SceneManagement/SceneManager.h"

#include "Audio/AudioComponent.h"
#include "Audio/AudioManager.h"

BulletSystem::BulletSystem(TileManager& tileManager) : ISystem{ Frac::eStage::OnUpdate }
, m_enttReg(Frac::EngineCore::GetInstance().GetEnttRegistry())
, m_fracReg(Frac::EngineCore::GetInstance().GetFracRegistry())
, m_tileManager(tileManager)
{
	// Enemy Damage Particles
	m_enemyDamageParticleSettings.FountainHeight = -1;
	m_enemyDamageParticleSettings.TimePerEmit = 0.005f;
	m_enemyDamageParticleSettings.StartSize = glm::vec3(0.03f);
	m_enemyDamageParticleSettings.Endsize = glm::vec3(0.015f);
	m_enemyDamageParticleSettings.StartColor = glm::vec4(1, 1, 0, 1);
	m_enemyDamageParticleSettings.EndColor = glm::vec4(1, 0, 0, 1);
	m_enemyDamageParticleSettings.IsEmissive = true;
	m_enemyDamageParticleSettings.ParticleSpeed = 1.5;
	m_enemyDamageParticleSettings.InitialAngleOffset = 90.f;
	m_enemyDamageParticleSettings.TimeToEmit = 0.2f;
	m_enemyDamageParticleSettings.ParticleLifetime = 0.25f;
	m_enemyDamageParticleSettings.HasGravity = false;

	// Explosion Particles
	m_explosionParticleSettings.FountainHeight = 5;
	m_explosionParticleSettings.TimePerEmit = 0.002f;
	m_explosionParticleSettings.StartSize = glm::vec3(0.01f);
	m_explosionParticleSettings.Endsize = glm::vec3(0.05f);
	m_explosionParticleSettings.StartColor = glm::vec4(1, 0, 0, 1);
	m_explosionParticleSettings.EndColor = glm::vec4(1, 0.5f, 0, 1);
	m_explosionParticleSettings.IsEmissive = true;
	m_explosionParticleSettings.InitialAngleOffset = 80.0f;
	m_explosionParticleSettings.TimeToEmit = 0.15f;
	m_explosionParticleSettings.ParticleLifetime = 0.15f;
	m_explosionParticleSettings.ParticleSpeed = 1.5f;
}

void BulletSystem::Update(float dt)
{
	float DeltaTime = dt;
	if (DeltaTime > GameSettings::FixedDT) DeltaTime = GameSettings::FixedDT;

	m_enttReg.view<BoxComponent, TOR::Transform, GridComponent, BulletComponent, DirectionComponent, VelocityComponent>().each([this, DeltaTime](auto entity, BoxComponent& box, TOR::Transform& trans, GridComponent& grid, BulletComponent& bullet, DirectionComponent& direction, VelocityComponent& velocity)
		{
			glm::vec3 bulletDirection = direction.Direction;
			//Update World and Grid Position
			if (m_enttReg.valid(bullet.Target) && !m_enttReg.any_of<DeathComponent>(bullet.Target))
			{
					const TOR::Transform& enemypos = m_enttReg.get<TOR::Transform>(bullet.Target);
					bulletDirection = glm::normalize(enemypos.Position - trans.Position);
			}
			trans.Position += bulletDirection * velocity.Speed * 0.25f * DeltaTime;
			glm::vec3 rot = glm::eulerAngles(glm::quatLookAt(-bulletDirection, glm::vec3{ 0.f,1.f,0.f }));
			glm::quat angles{ rot };
			trans.Orientation = angles;
			m_enttReg.patch<TOR::Transform>(entity);
			grid.GridPos = m_tileManager.WorldToGrid(trans.Position);
		
			//Is the Bullet in Grid?
			if (m_tileManager.IsInGrid(grid.GridPos.x, grid.GridPos.y))
			{
				entt::entity closestEnemy = ReturnClosestEnemy(4, trans.Position);
				if (closestEnemy != entt::null) 
				{
					EnemyComponent& closestEnemyComp = m_enttReg.get<EnemyComponent>(closestEnemy);
					TOR::Transform& closestEnemyTransComp = m_enttReg.get<TOR::Transform>(closestEnemy);					
					if (IsInRange(trans.Position, closestEnemyTransComp.Position, 0.7f))
					{
						switch (bullet.Stats.Type)
						{
						case EProjectileType::Bullet:
						{
							if (m_enttReg.any_of<HealthComponent>(closestEnemy))
							{
								HealthComponent& enemyHealth = m_enttReg.get<HealthComponent>(closestEnemy);
								enemyHealth.Health -= bullet.Stats.Damage;
							}
							
							Frac::EngineCore::GetInstance().GetParticleSystem().SpawnFountainEmitter(m_enemyDamageParticleSettings, m_fracReg.TranslateENTTEntity(closestEnemy));
						}
							break;
						case EProjectileType::Missile:
						{
							std::vector<entt::entity> enemiesInRange = ReturnEnemiesInRange(GameSettings::RocketLauncherProjectileRange, trans.Position);

							for (size_t i = 0; i < enemiesInRange.size(); i++)
							{
								if (m_enttReg.valid(enemiesInRange[i]) && m_enttReg.any_of<HealthComponent>(enemiesInRange[i]))
								{
									HealthComponent& enemyHealth = m_enttReg.get<HealthComponent>(enemiesInRange[i]);
									enemyHealth.Health -= bullet.Stats.Damage;
									Frac::EngineCore::GetInstance().GetParticleSystem().SpawnFountainEmitter(m_enemyDamageParticleSettings, m_fracReg.TranslateENTTEntity(enemiesInRange[i]));
								}
							}
							Frac::Entity* explosionParticleEntity = new Frac::Entity(m_fracReg, "ExplosionParticle");
							explosionParticleEntity->SetParent(Frac::EngineCore::GetInstance().GetSceneManager().GetActiveScene());
							m_fracReg.AddComponent<TOR::Transform>(*explosionParticleEntity, glm::vec3(trans.Position.x, trans.Position.y + 0.5f, trans.Position.z));
							m_fracReg.AddComponent<RemoveSelfComponent>(*explosionParticleEntity);
							m_fracReg.AddComponent<Frac::AudioSourceComponent>(*explosionParticleEntity);
							Frac::AudioSourceComponent& explosionAudio = m_fracReg.GetComponent<Frac::AudioSourceComponent>(*explosionParticleEntity);
							Frac::EngineCore::GetInstance().GetAudioManager().AddEvent(explosionAudio, "event:/Explosion");
							Frac::EngineCore::GetInstance().GetAudioManager().PlayEvent(explosionAudio, "event:/Explosion");
							Frac::EngineCore::GetInstance().GetParticleSystem().SpawnFountainEmitter(m_explosionParticleSettings, *explosionParticleEntity);
						}
							break;
						default:
							break;
						}
						m_enttReg.destroy(entity);
						
					}
				}
			}
			else
			{
				m_enttReg.destroy(entity);
			}
		});
		RemoveSelf(DeltaTime);
}

entt::entity BulletSystem::ReturnClosestEnemy(int range, glm::vec3 worlpos)
{
	entt::entity nearestEnemy = entt::null;

	glm::ivec2 curGridPos = m_tileManager.WorldToGrid(worlpos);
	entt::entity curTile = m_tileManager.GetTile(curGridPos.x, curGridPos.y);

	if (curTile != entt::null)
	{
		TileComponent& TileComp = m_enttReg.get<TileComponent>(curTile);

		float enemyDistance = 9999999.0f;

		const int allRows = m_tileManager.GetAllRows();
		const int allCols = m_tileManager.GetAllCols();

		int minXRange = -range + curGridPos.x;
		int maxXRange = range + curGridPos.x;

		int minYRange = -range + curGridPos.y;
		int maxYRange = range + curGridPos.y;

		if (minXRange < 0) minXRange = 0;
		if (minYRange < 0) minYRange = 0;
		if (maxXRange > allRows) maxXRange = allRows;
		if (maxYRange > allCols) maxYRange = allCols;

		for (int x = minXRange; x < maxXRange; ++x)
		{
			for (int y = minYRange; y < maxYRange; ++y)
			{
				entt::entity nextTile = m_tileManager.GetTile(x, y);
				
				//Is the Next Tile Occupied?
				if ((nextTile != entt::null) && (m_enttReg.any_of<TileComponent>(nextTile)) && m_tileManager.IsTileOccupiedByEnemy(nextTile))
				{
					entt::entity enemyEntity = m_tileManager.GetAnEnemyOccupier(nextTile);
					
					//Is it an Enemy?
					if (m_enttReg.any_of<EnemyComponent>(enemyEntity))
					{
						TOR::Transform enemyPos = m_enttReg.get<TOR::Transform>(enemyEntity);
						float curDistance = glm::length(worlpos - enemyPos.Position);

						//Is it the closest Enemy?
						if (curDistance < enemyDistance)
						{
							enemyDistance = curDistance;
							nearestEnemy = enemyEntity;
						}
					}					
				}
			}
		}
	}
	return nearestEnemy;
}

std::vector<entt::entity> BulletSystem::ReturnEnemiesInRange(int range, glm::vec3 worldpos)
{
	std::vector<entt::entity> enemiesInRange;
	glm::ivec2 curGridPos = m_tileManager.WorldToGrid(worldpos);
	entt::entity curTile = m_tileManager.GetTile(curGridPos.x, curGridPos.y);

	if (curTile != entt::null)
	{
		const int allRows = m_tileManager.GetAllRows();
		const int allCols = m_tileManager.GetAllCols();

		int minXRange = -range + curGridPos.x;
		int maxXRange = range + curGridPos.x;

		int minYRange = -range + curGridPos.y;
		int maxYRange = range + curGridPos.y;

		if (minXRange < 0) minXRange = 0;
		if (minYRange < 0) minYRange = 0;
		if (maxXRange > allRows) maxXRange = allRows;
		if (maxYRange > allCols) maxYRange = allCols;

		for (int x = minXRange; x < maxXRange; ++x)
		{
			for (int y = minYRange; y < maxYRange; ++y)
			{
				entt::entity nextTile = m_tileManager.GetTile(x, y);

				//Is the Next Tile Occupied?
				if ((nextTile != entt::null) && (m_enttReg.any_of<TileComponent>(nextTile)) && m_tileManager.IsTileOccupiedByEnemy(nextTile))
				{
					TileComponent nextTileComponent = m_enttReg.get<TileComponent>(nextTile);

					//Go over all enemies in the TileOccupier and add them to the vector
					for (size_t i = 0; i < (sizeof(nextTileComponent.TileOccupiers) / sizeof(nextTileComponent.TileOccupiers[0])); i++)
					{
						enemiesInRange.push_back(nextTileComponent.TileOccupiers[i]);
					}
				}
			}
		}
	}
	return enemiesInRange;
}

void BulletSystem::RemoveSelf(float dt)
{
	m_enttReg.view<RemoveSelfComponent>().each([this, dt](auto entity, RemoveSelfComponent& removeSelfComponent)
	{
		if (removeSelfComponent.Timer < removeSelfComponent.Duration)
		{
			removeSelfComponent.Timer += dt;
		}
		else
		{
			m_enttReg.destroy(entity);
		}
	});
}
