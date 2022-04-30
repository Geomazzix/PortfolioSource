#include "TDpch.h"
#include "EnemyDirectorSystem.h"
#include "SceneManagement/SceneManager.h"
#include "GameDirector.h"
#include "Graphics/Components/TextComponent.h"
#include "UtilityTD.h"

#include "Audio/AudioComponent.h"
#include "Audio/AudioManager.h"

EnemyDirectorSystem::EnemyDirectorSystem(GameDirector& gameDirector, TileManager& tileManager, EnemyEntityCreator& enemyEntityCreator) : ISystem{ Frac::eStage::OnUpdate }
, m_gameDirector(gameDirector)
, m_enemyEntityCreator(enemyEntityCreator)
, m_tileManager(tileManager)
, m_enttReg(Frac::EngineCore::GetInstance().GetEnttRegistry())
, m_fracReg(Frac::EngineCore::GetInstance().GetFracRegistry())
{
	m_enemiesAlive = 0;
	m_currentWave = 0;
	m_maxWaves = 0;
	m_spawnersActive = 0;
	m_prepPhaseTime = 5.0f;
	m_prepPhaseTimeElapsed = 0.0f;

	m_alignmentForce = glm::vec3(0.0f);
	m_separationForce = glm::vec3(0.0f);
	m_cohesionForce = glm::vec3(0.0f);
	
	gameDirector.AssignSystemToStates(this, GameState::WavePhase | GameState::PreperationPhase | GameState::TutorialState);
}

void EnemyDirectorSystem::Update(float dt)
{
	float DeltaTime = dt;
	if (DeltaTime > GameSettings::FixedDT) DeltaTime = GameSettings::FixedDT;

	//Wave Phase
	if (m_gameDirector.GetCurrentState() == GameState::WavePhase)
	{
		SpawnerUpdate(DeltaTime);
		EnemyMovementUpdate(DeltaTime);
	}

	//Prep Phase
	else if (m_gameDirector.GetCurrentState() == GameState::PreperationPhase)
	{
		m_prepPhaseTimeElapsed += DeltaTime;
		
		if (m_currentWave >= m_maxWaves && m_nextLevel)
		{
			m_nextLevel = false;
			//You Win!
			LOGINFO("[Enemy Director] You win!");
			m_gameDirector.SwitchToGroup(GameState::NextLevelState);
		}
		
		if (m_prepPhaseTimeElapsed >= m_prepPhaseTime)
		{
			m_prepPhaseTimeElapsed = 0.0f;
			CalculateSpawners(); //TODO: Move to Constructor once Entity Creators are in place.
			CalculateNewWave();		
		}		
	}	
}

void EnemyDirectorSystem::EnemyMovementUpdate(float dt)
{
	int activeEnemies = 0;

	m_enttReg.view<EnemyComponent, DirectionComponent, GridComponent, TOR::Transform, VelocityComponent, HealthComponent>().each(
		[this, &dt, &activeEnemies](
			const auto entity, 
			EnemyComponent& enemyComp, 
			DirectionComponent& dirComp, 
			GridComponent& gridComp, 
			TOR::Transform& transComp, 
			VelocityComponent& velComp, 
			HealthComponent& healthComp
		) {
			activeEnemies++;

			//Update and Reset Stats
			glm::vec3 desiredDir = glm::vec3(0.0f);

			//Get current Tile
			const entt::entity& curTileEntt = m_tileManager.GetTile(gridComp.GridPos.x, gridComp.GridPos.y);
			
			//Does the Tile exist?
			if (curTileEntt != entt::null)
			{
				const TileComponent& curTileComp = m_enttReg.get<TileComponent>(curTileEntt);

				//Did we reach the Base?
				if (m_enttReg.valid(curTileComp.TileOccupiers[0]) && m_enttReg.any_of<BaseComponent>(curTileComp.TileOccupiers[0])) //Only way to check if the Base is the Occupier
				{
					m_enttReg.get<HealthComponent>(curTileComp.TileOccupiers[0]).Health -= enemyComp.Data.DamageToBase;
					healthComp.Health = 0; //Destroy Enemy. This way the Player gets resources.
				}
				else
				{
					desiredDir = { curTileComp.FloodFill.FlowDirection.x, 0.0f, curTileComp.FloodFill.FlowDirection.z };
					dirComp.Direction = glm::normalize(desiredDir + dirComp.Direction) * velComp.Speed;								//Max Movement towards Target.
				}

				//Flocking Movement
				CalculateFlocking(entity);
				glm::vec3 acceleration = m_alignmentForce + m_separationForce + m_cohesionForce;								//Adding all Flocking Forces
				if (acceleration != glm::vec3(0.0f))
				{
					enemyComp.Flocking.Acceleration = glm::normalize(acceleration) * enemyComp.Flocking.MaxForce;				//Max Acceleration
					enemyComp.Flocking.Acceleration.y = 0.0f;
					dirComp.Direction = glm::normalize(enemyComp.Flocking.Acceleration + dirComp.Direction) * velComp.Speed;	//Combine Flocking (Acceleration) with Direction
				}
			}


			//Will the Enemy Collide with a wall?
			const glm::vec3 nextWorldPos = transComp.Position + (dirComp.Direction * dt);
			const glm::ivec2 nextGridPos = m_tileManager.WorldToGrid(nextWorldPos);

			//Are we going to move into another Tile?
			if (nextGridPos != gridComp.GridPos && nextGridPos != glm::ivec2(-1))
			{
				//Get next Tile				
				const entt::entity& nextTile = m_tileManager.GetTile(nextGridPos.x, nextGridPos.y);
				bool isAccessible = true;

				//Does the Tile exist?
				if (nextTile != entt::null)
				{
					const TileComponent& nextTileComp = m_enttReg.get<TileComponent>(nextTile);
					if (!nextTileComp.States.isActive || !nextTileComp.States.isWalkable)
					{
						isAccessible = false;
					}
				}
				else
				{
					isAccessible = false;
				}

				//Can we move into the next Tile?
				if (isAccessible)
				{
					enemyComp.Data.PreviousPosition = gridComp.GridPos;
					enemyComp.Data.PreviousDirection = dirComp.Direction;
				}
				else
				{
					//Get a Direction that pushes us away from the wall
					dirComp.Direction = glm::normalize(glm::reflect(dirComp.Direction, FindPushForce(gridComp.GridPos, nextGridPos))) * velComp.Speed;
				}
			}
			else
			{
				enemyComp.Data.PreviousPosition = gridComp.GridPos;
			}


			transComp.Position.x += dirComp.Direction.x * dt;
			transComp.Position.z += dirComp.Direction.z * dt;
			dirComp.Direction.y = 0.0f;
			transComp.Orientation = glm::slerp(transComp.Orientation, glm::quatLookAt(dirComp.Direction * dt, glm::vec3(0, 1, 0)), enemyComp.Data.RotationSpeed * dt);	//This is scalling the enemies :o

			//Update Occupancy if we moved into a new Tile
			const glm::ivec2 newGridPos = m_tileManager.WorldToGrid(transComp.Position);
			if (enemyComp.Data.PreviousPosition != newGridPos && newGridPos != glm::ivec2(-1))
			{
				gridComp.GridPos = newGridPos;

				m_tileManager.RemoveTileOccupier(m_tileManager.GetTile(enemyComp.Data.PreviousPosition.x, enemyComp.Data.PreviousPosition.y), entity);
				m_tileManager.SetTileOccupier(m_tileManager.GetTile(gridComp.GridPos.x, gridComp.GridPos.y), entity);
			}

			m_enttReg.patch<TOR::Transform>(entity);
					
		});

	m_enemiesAlive = activeEnemies;
	int a = 0;
}

void EnemyDirectorSystem::CalculateFlocking(const entt::entity& enemy)
{
	EnemyComponent& enemyComp = m_enttReg.get<EnemyComponent>(enemy);
	TOR::Transform& enemyTrans = m_enttReg.get<TOR::Transform>(enemy);
	DirectionComponent& enemyDir = m_enttReg.get<DirectionComponent>(enemy);
	VelocityComponent& enemyVel = m_enttReg.get<VelocityComponent>(enemy);

	int alignmentSum = 0;
	int separationSum = 0;
	int cohesionSum = 0;

	m_alignmentForce = glm::vec3(0.0f);
	m_separationForce = glm::vec3(0.0f);
	m_cohesionForce = glm::vec3(0.0f);

	enemyComp.Flocking.Acceleration = glm::vec3(0.0f);

	//Go over all Enemy Entities
	m_enttReg.view<EnemyComponent, DirectionComponent, TOR::Transform, VelocityComponent>().each
	([this, &enemy, &enemyComp, &enemyTrans, &enemyDir, &enemyVel, &alignmentSum, &separationSum, &cohesionSum]
		(auto entity, EnemyComponent& curEnemyComp, DirectionComponent& curDir, TOR::Transform& curTrans, VelocityComponent& curVel)
		{
			//Check everyone except ourself
			if (entity != enemy)
			{
				//Alignment
				if (IsInRadius(enemyTrans.Position, curTrans.Position, enemyComp.Flocking.AlignmentRadius))
				{		
					if (curDir.Direction != glm::vec3(0))
					{
						m_alignmentForce += curDir.Direction;
						alignmentSum++;
					}
				}

				//Separation
				if (IsInRadius(enemyTrans.Position, curTrans.Position, enemyComp.Flocking.SeparationRadius))
				{
					float distance = glm::length(curTrans.Position - enemyTrans.Position);			//distance between us and nearby enemy
					glm::vec3 difference = enemyTrans.Position - curTrans.Position;					//vector pointing away 'pushing force'
					difference /= distance;															//difference should be inversely-proportional to distance
					m_separationForce += difference;
					separationSum++;
				}

				//Cohesion
				if (IsInRadius(enemyTrans.Position, curTrans.Position, enemyComp.Flocking.CohesionRadius))
				{
					m_cohesionForce += curTrans.Position;
					cohesionSum++;
				}
			}
		});

	if (alignmentSum > 0)
	{
		m_alignmentForce /= alignmentSum;
		m_alignmentForce -= enemyDir.Direction;

		if(m_alignmentForce != glm::vec3(0.0f))
		m_alignmentForce = glm::normalize(m_alignmentForce) * enemyComp.Flocking.AlignmentMultiplier;
	}

	if (separationSum > 0)
	{
		m_separationForce /= separationSum;
		m_separationForce -= enemyDir.Direction;

		if (m_separationForce != glm::vec3(0.0f))
		m_separationForce = glm::normalize(m_separationForce) * enemyComp.Flocking.SeparationMultiplier;
	}

	if (cohesionSum > 0)
	{
		m_cohesionForce /= cohesionSum;
		m_cohesionForce -= enemyTrans.Position;
		m_cohesionForce -= enemyDir.Direction;

		if (m_cohesionForce != glm::vec3(0.0f))
		m_cohesionForce = glm::normalize(m_cohesionForce) * enemyComp.Flocking.CohesionMultiplier;
	}

}

void EnemyDirectorSystem::SpawnerUpdate(float dt)
{
	m_enttReg.view<SpawnerComponent, GridComponent, TOR::Transform>().each([this, &dt](const auto entity, SpawnerComponent& spawnerComp, GridComponent& gridComp, TOR::Transform& transComp)
	{
		if ((spawnerComp.ToSpawn != 0))
		{
			spawnerComp.TimeElapsed += dt;

			if (spawnerComp.TimeElapsed >= spawnerComp.Stats.Waves[m_currentWave - 1].SpawnDelays[spawnerComp.Stats.Waves[m_currentWave - 1].Enemies.size() - spawnerComp.ToSpawn])
			{
				glm::ivec2 spawnPos = FindSpawnPoint(gridComp.GridPos);
				EEnemyType enemytype = spawnerComp.Stats.Waves[m_currentWave - 1].Enemies[spawnerComp.Stats.Waves[m_currentWave - 1].Enemies.size() - spawnerComp.ToSpawn];
				spawnerComp.TimeElapsed = 0.0f;
				m_enemyEntityCreator.CreateEnemy(enemytype, spawnPos, glm::vec3(0, 0, 0), 0, entity);
				spawnerComp.ToSpawn--;

				//spawnerComp.TimeElapsed = 0.0f;
				//m_enemyEntityCreator.CreateEnemy(2, spawnPos, glm::vec3(0, 0, 0), 0,entity);
			}
		}
		else if ((m_enemiesAlive == 0) && (m_gameDirector.GetCurrentState() == GameState::WavePhase))
		{
			if ((m_currentWave < m_maxWaves))
			{
				++m_currentWave;
			}
			else 
			{
				m_nextLevel = true;
			}
			m_gameDirector.SwitchToGroup(GameState::PreperationPhase);
			LOGINFO("[Enemy Director] Entering Preparation Phase");
		}
	});
}

void EnemyDirectorSystem::CalculateSpawners()
{
	int activeSpawners = 0;

	m_enttReg.view<SpawnerComponent>().each([this, &activeSpawners](const auto entity, SpawnerComponent& spawnerComp)
		{
			activeSpawners++;
		});
	m_spawnersActive = activeSpawners;
}

void EnemyDirectorSystem::CalculateNewWave()
{
	//A simple way to increase the number of enemies being spawned
	if (m_enemiesAlive == 0 && m_spawnersActive > 0)
	{
		if(m_currentWave == 0) 
			++m_currentWave;
			
		if (m_currentWave <= m_maxWaves && m_currentWave != 0)
		{
			LOGINFO("[Enemy Director] New Wave Reached! Wave: %i", m_currentWave);

			DistributeEnemies();
			m_gameDirector.SwitchToGroup(GameState::WavePhase);
		}
	}
}

void EnemyDirectorSystem::DistributeEnemies()
{
	m_enttReg.view<SpawnerComponent>().each([this](const auto entity, SpawnerComponent& spawnerComp)
		{
			spawnerComp.ToSpawn = spawnerComp.Stats.Waves[m_currentWave-1].Enemies.size();
		});
}

void EnemyDirectorSystem::SetMaxWaves(int value)
{
	if (value >= 0)
	{
		m_maxWaves = value;
		LOGINFO("[Enemy Director] Max Waves assigned to %i", value);
	}
	else
	{
		LOGWARNING("[Enemy Director] Max Wave cannot be assigned to the %i value ", value);
	}
}

void EnemyDirectorSystem::SetCurrentWave(int value)
{
	if (value >= 0 && value <= m_maxWaves)
	{
		m_currentWave = value;
		LOGINFO("[Enemy Director] Current Wave assigned to %i", value);
	}
	else
	{
		LOGWARNING("[Enemy Director] Current Wave cannot be assigned to the %i value ", value);
	}
}

void EnemyDirectorSystem::SetPrepPhaseTimer(float value)
{
	if (value >= 0.0f)
	{
		m_prepPhaseTime = value;
		LOGINFO("[Enemy Director] Prep Phase Timer assigned to %.*f", 2 ,value);
	}
	else
	{
		LOGWARNING("[Enemy Director] Prep Phase Timer cannot be assigned to the %f value ", value);
	}
}

void EnemyDirectorSystem::SetPrepPhaseElapsedTimer(float value)
{
	if (value >= 0.0f)
	{
		m_prepPhaseTimeElapsed = value;
		LOGINFO("[Enemy Director] Prep Phase Elapsed Timer assigned to %.*f", 2, value);
	}
	else
	{
		LOGWARNING("[Enemy Director] Prep Phase Elapsed Timer cannot be assigned to the %f value ", value);
	}
}

const int EnemyDirectorSystem::GetMaxWaves()
{
	return m_maxWaves;
}

const int EnemyDirectorSystem::GetCurrentWave()
{
	return m_currentWave;
}

const int EnemyDirectorSystem::GetEnemiesAlive()
{
	return m_enemiesAlive;
}

const float EnemyDirectorSystem::GetPrepPhaseTimer()
{
	return m_prepPhaseTime;
}

const float EnemyDirectorSystem::GetPrepPhaseElapsedTimer()
{
	return m_prepPhaseTimeElapsed;
}

glm::ivec2 EnemyDirectorSystem::FindSpawnPoint(const glm::ivec2& spawnerGridPos)
{
	int xMin = spawnerGridPos.x - 1;
	int yMin = spawnerGridPos.y - 1;
	int xMax = spawnerGridPos.x + 1;
	int yMax = spawnerGridPos.y + 1;

	for (int x = xMin; x < xMax; x++)
	{
		for (int y = yMin; y < yMax; y++)
		{
			if (m_tileManager.IsInGrid(x, y))
			{
				const entt::entity& curEntt = m_tileManager.GetTile(x, y);
				if (curEntt != entt::null) 
				{
					TileComponent& curTile = m_enttReg.get<TileComponent>(curEntt);

					if (!m_tileManager.IsTileOccupied(curEntt) && curTile.States.isWalkable)
					{
						return glm::ivec2{ x,y };
					}
				}
			}			
		}
	}

	return spawnerGridPos;
}

glm::vec3 EnemyDirectorSystem::FindPushForce(const glm::ivec2& curPos, const glm::ivec2& nextPos)
{
	glm::ivec2 result = curPos - nextPos;
	glm::vec3 pushForce(0.0f);

	if (result.y == 1)			//Tile is Up -> Push Down
	{
		pushForce.x += 1.0f;
	}
	else if (result.y == -1)	//Tile is Down -> Push Up
	{
		pushForce.x += -1.0f;
	}
	if (result.x == 1)			//Tile is Left -> Push Right
	{
		pushForce.z += -1.0f;
	}
	else if (result.x == -1)	//Tile is Right -> Push Left
	{
		pushForce.z += +1.0f;
	}

	if (pushForce != glm::vec3(0.0f))
	{
		return glm::normalize(pushForce);
	}
	return pushForce;
}
