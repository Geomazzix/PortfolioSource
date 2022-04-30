#include "TDpch.h"
#include "TurretSystem.h"
#include "TileManager.h"
#include "CoreRenderAPI/Components/Transform.h"
#include "SceneManagement/SceneManager.h"

#include "Audio/AudioComponent.h"
#include "Audio/AudioManager.h"

TurretSystem::TurretSystem(TileManager& tileManager, PlayerEntityCreator& playerEntityCreator) : ISystem { Frac::eStage::OnUpdate }
, m_playerEntityCreator(playerEntityCreator)
, m_enttReg(Frac::EngineCore::GetInstance().GetEnttRegistry())
, m_fracReg(Frac::EngineCore::GetInstance().GetFracRegistry())
, m_tileManager(tileManager)
{

}

void TurretSystem::Update(float dt)
{
	float DeltaTime = dt;
	if (DeltaTime > GameSettings::FixedDT) DeltaTime = GameSettings::FixedDT;

	TurretFindTargetSystem();
	TurretShootTargetSystem(DeltaTime);

	SpawnAnimationSystem(DeltaTime);
	DespawnAnimationSystem(DeltaTime);
}

const entt::entity& TurretSystem::GetEnemyInRange(int range, const glm::ivec2& gridPos)
{
	const int allRows = m_tileManager.GetAllRows();
	const int allCols = m_tileManager.GetAllCols();

	int xMin = gridPos.x - range;
	int yMin = gridPos.y - range;
	int xMax = gridPos.x + range;
	int yMax = gridPos.y + range;

	if (xMin < 0) xMin = 0;
	if (yMin < 0) yMin = 0;
	if (xMax > allRows) xMax = allRows;
	if (yMax > allCols) yMax = allCols;

	int bestCost = 99999;
	entt::entity enemyInRange = entt::null;

	for (int x = xMin; x < xMax; x++)
	{
		for (int y = yMin; y < yMax; y++)
		{
			const entt::entity& curEntity = m_tileManager.GetTile(x, y);

			if (!((gridPos.x == x) && (gridPos.y == y)) && (curEntity != entt::null))
			{
				TileComponent& curTile = m_enttReg.get<TileComponent>(curEntity);

				if ((m_tileManager.IsTileOccupiedByEnemy(curEntity)) && (curTile.FloodFill.Cost < bestCost) && (curTile.States.isWalkable) && (curTile.States.isActive))
				{								
					bestCost = curTile.FloodFill.Cost;
					enemyInRange = m_tileManager.GetAnEnemyOccupier(curEntity);									
				}
			}
		}
	}
	return enemyInRange;
}

void TurretSystem::TurretFindTargetSystem()
{
	m_enttReg.view<BoxComponent, TOR::Transform, GridComponent, DirectionComponent, VelocityComponent, TurretComponent>().each([this](const auto entity, BoxComponent& box, TOR::Transform& trans, GridComponent& grid, DirectionComponent& direction, VelocityComponent& velocity, TurretComponent& turret)
		{
			entt::entity enemy = GetEnemyInRange(turret.Stats.Range, grid.GridPos);

			if (enemy != entt::null)
			{
				TOR::Transform& enemyTransform = m_enttReg.get<TOR::Transform>(enemy);				
				turret.Target = enemy;
				turret.hasTarget = true;				
			}
			else
			{
				turret.hasTarget = false;
			}
		});
}

void TurretSystem::TurretShootTargetSystem(float dt)
{
	m_enttReg.view<BoxComponent, TOR::Transform, GridComponent, DirectionComponent, VelocityComponent, TurretComponent, Frac::AudioSourceComponent>().each([this, dt](const auto entity, BoxComponent& box, TOR::Transform& trans, GridComponent& grid, DirectionComponent& direction, VelocityComponent& velocity, TurretComponent& turret, Frac::AudioSourceComponent& audio)
		{
			turret.TimeElapsed += dt;
			glm::vec3 fixedTarget = { 0.f,0.f,0.f };
			if(m_enttReg.valid(turret.Target))
			{
				fixedTarget = m_enttReg.get<TOR::Transform>(turret.Target).Position;
			}
			fixedTarget.y += turret.TargetHeightOffset;
			direction.Direction = glm::normalize(glm::vec3(trans.Position.x - fixedTarget.x, 0, trans.Position.z - fixedTarget.z));
			trans.Orientation = glm::slerp(trans.Orientation, glm::quatLookAt(direction.Direction, glm::vec3(0, 1, 0)), dt * turret.Stats.TurretRotationSpeed);
			
			m_enttReg.patch<TOR::Transform>(entity);

			if ((turret.hasTarget))
			{
				if ((turret.TimeElapsed >= (turret.Stats.FireDelay) + turret.Stats.FireDelayRandomness))
				{
					if (turret.FirstShot)
					{
						AddRandomFireDelay(turret.Stats);
						turret.FirstShot = false;
						turret.TimeElapsed = (turret.Stats.FireDelay);
					}
					else 
					{
						glm::vec3 bulletSpawnPos = trans.Position;
						bulletSpawnPos.y += m_bulletSpawnYOffset;
						turret.TimeElapsed = 0.0f;

						direction.Direction = glm::normalize(fixedTarget - bulletSpawnPos);
						turret.hasTarget = false;

						Frac::EngineCore::GetInstance().GetAudioManager().PlayEvent(audio, "event:/Tower_Shoot");
						m_playerEntityCreator.CreatePlayerBullet(turret.Target,turret.Stats.Type, bulletSpawnPos, grid.GridPos, direction.Direction);
						AddRandomFireDelay(turret.Stats);
					}
				}
			}
			else 
			{
				turret.FirstShot = true;
			}
		});
}

void TurretSystem::SpawnAnimationSystem(float dt)
{
	m_enttReg.view<TOR::Transform, TurretComponent>().each([this, dt](const auto entity, TOR::Transform& trans, TurretComponent& turret)
	{
		if (turret.SpawnState)
		{
			trans.Scale += (turret.Stats.SpawnSize * m_turretSpawnAnimationSpeed * dt);
			if (trans.Scale.x >= turret.Stats.SpawnSize.x)
			{
				turret.SpawnState = false;
				trans.Scale = turret.Stats.SpawnSize;
				AddRandomFireDelay(turret.Stats);
			}
			m_enttReg.patch<TOR::Transform>(entity);
		}
	});
}

void TurretSystem::DespawnAnimationSystem(float dt)
{
	m_enttReg.view<TOR::Transform, TurretComponent>().each([this, dt](const auto entity, TOR::Transform& trans, TurretComponent& turret)
	{
		if (turret.DespawnState)
		{
			trans.Scale -= (turret.Stats.SpawnSize * m_turretSpawnAnimationSpeed * dt);
			m_enttReg.patch<TOR::Transform>(entity);

			if (trans.Scale.x <= 0.0f)
			{
				glm::ivec2 gridPos = m_tileManager.WorldToGrid(trans.Position);
				entt::entity tileEntity = m_tileManager.GetTile(gridPos.x, gridPos.y);
				entt::entity tileOccupier = m_tileManager.GetSpecificTileOccupier(tileEntity, false);

				//Free Tile Occupancy
				m_tileManager.RemoveTileOccupier(tileEntity, entity);

				//Delete Turret
				m_enttReg.destroy(entity);
			}
		}
	});
}

//Adds between 0.125 to 0.25 x the original FireDelay as an extra Random Delay to the firerate before every shot
void TurretSystem::AddRandomFireDelay(TurretStats& stats)
{
	std::srand(++m_randomSeed); //Use different seed every iteration for random generation
	int randomNumber = std::rand() % 50 + 50; //Random int between 50 and 100
	stats.FireDelayRandomness = ((stats.FireDelay * 0.25f) * (static_cast<float>(randomNumber) * 0.01f)); 
}
