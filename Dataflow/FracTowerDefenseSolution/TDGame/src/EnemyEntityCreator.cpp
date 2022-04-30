#include "TDpch.h"
#include "EnemyEntityCreator.h"
#include "Graphics/Components/TextComponent.h"

#include "Graphics/ParticleRenderSystem/ParticleSystem.h"
#include "Core/EngineCore.h"
#include "LevelManager.h"

EnemyEntityCreator::EnemyEntityCreator(TileManager& tileManager, Frac::EntityRegistry& entityReg, Frac::SceneManager& sceneManager) : IEntityCreator(tileManager, entityReg, sceneManager), m_LevelManager(nullptr)
{

}

EnemyEntityCreator::~EnemyEntityCreator() {
	const float tileSize = m_tileManager.GetTileSize();
}

void EnemyEntityCreator::CreateEnemy(EEnemyType type, const glm::ivec2& gridposition, const glm::vec3& rotation, int enemyLevel, const entt::entity& spawner) {
	
	//Create Entity
	Frac::Entity& enemyEnt = *new Frac::Entity(m_entityRegistry, "Enemy " + std::to_string(createdEnemies));
	enemyEnt.SetParent(m_sceneEntity);

	//Add required Components
	m_entityRegistry.AddComponent<BoxComponent>(enemyEnt, glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f });
	m_entityRegistry.AddComponent<GridComponent>(enemyEnt, glm::ivec2{ gridposition.x ,gridposition.y });
	m_entityRegistry.AddComponent<TOR::Transform>(enemyEnt, TOR::Transform{ m_tileManager.GridToWorld(gridposition.x, gridposition.y)});
	m_entityRegistry.AddComponent<EnemyComponent>(enemyEnt);
	m_entityRegistry.AddComponent<HealthComponent>(enemyEnt);
	m_entityRegistry.AddComponent<DirectionComponent>(enemyEnt);
	m_entityRegistry.AddComponent<VelocityComponent>(enemyEnt);
	m_entityRegistry.AddComponent<ResourceComponent>(enemyEnt);
	m_entityRegistry.AddComponent<SpawnComponent>(enemyEnt, SpawnComponent{ 1.f });

	//Get required Components
	EnemyComponent& enemyComp = m_entityRegistry.GetComponent<EnemyComponent>(enemyEnt);
	HealthComponent& healthComp = m_entityRegistry.GetComponent<HealthComponent>(enemyEnt);
	ResourceComponent& resourceComp = m_entityRegistry.GetComponent<ResourceComponent>(enemyEnt);
	VelocityComponent& velocityComp = m_entityRegistry.GetComponent<VelocityComponent>(enemyEnt);
	SpawnComponent& animationComp = m_entityRegistry.GetComponent<SpawnComponent>(enemyEnt);
	
	//Create Types
	switch (type)
	{
	case EEnemyType::Slow:
	{
		//Stats
		enemyComp.Data = GameSettings::SlowEnemyData;
		enemyComp.Flocking = GameSettings::SlowEnemyFlock;

		//Health
		healthComp.Health = enemyComp.Data.MaxHealth;
		healthComp.MaxHealth = enemyComp.Data.MaxHealth;

		//Resources
		resourceComp.ResourcesWorth = enemyComp.Data.ResourcesWorth;

		//Velocity
		velocityComp.Speed = enemyComp.Flocking.MaxSpeed;

		//Extra
		enemyComp.Data.PreviousPosition = gridposition;
		animationComp.endScale = enemyComp.Data.Size;
		m_entityRegistry.GetComponent<TOR::Transform>(enemyEnt).Position.y += 1.0f;

		//Model
		m_sceneManager.LoadModelOnEntity(enemyEnt, Frac::EModelFileType::GLTF, enemyComp.Data.Model);

		break;
	}
	case EEnemyType::Normal:
	{
		//Stats
		enemyComp.Data = GameSettings::NormalEnemyData;
		enemyComp.Flocking = GameSettings::NormalEnemyFlock;

		//Health
		healthComp.Health = enemyComp.Data.MaxHealth;
		healthComp.MaxHealth = enemyComp.Data.MaxHealth;

		//Resources
		resourceComp.ResourcesWorth = enemyComp.Data.ResourcesWorth;

		//Velocity
		velocityComp.Speed = enemyComp.Flocking.MaxSpeed;

		//Extra
		enemyComp.Data.PreviousPosition = gridposition;
		animationComp.endScale = enemyComp.Data.Size;
		m_entityRegistry.GetComponent<TOR::Transform>(enemyEnt).Position.y += 1.0f;

		//Model
		m_sceneManager.LoadModelOnEntity(enemyEnt, Frac::EModelFileType::GLTF, enemyComp.Data.Model);

		break;
	}
	case EEnemyType::Fast:
	{
		//Stats
		enemyComp.Data = GameSettings::FastEnemyData;
		enemyComp.Flocking = GameSettings::FastEnemyFlock;

		//Health
		healthComp.Health = enemyComp.Data.MaxHealth;
		healthComp.MaxHealth = enemyComp.Data.MaxHealth;

		//Resources
		resourceComp.ResourcesWorth = enemyComp.Data.ResourcesWorth;

		//Velocity
		velocityComp.Speed = enemyComp.Flocking.MaxSpeed;

		//Extra
		enemyComp.Data.PreviousPosition = gridposition;
		animationComp.endScale = enemyComp.Data.Size;
		m_entityRegistry.GetComponent<TOR::Transform>(enemyEnt).Position.y += 1.0f;

		//Model
		m_sceneManager.LoadModelOnEntity(enemyEnt, Frac::EModelFileType::GLTF, enemyComp.Data.Model);

		break;
	}
	}
	//Increase Enemy Count
	createdEnemies++;

	//Set-up Health Bar
	Frac::Entity* HealthBar = new Frac::Entity(m_entityRegistry, "HealthBar");
	HealthBar->SetParent(enemyEnt);
	m_entityRegistry.AddComponent<TOR::Transform>(*HealthBar, TOR::Transform
		{
		{ 0.f,0.5f,0.f }
			,{0.f, 0.f, 0.0f}
			,{0.f, 0.f, 0.f, 0.f}
			, {glm::vec3(0.f, glm::radians<float>(90.f), 0.f)}
		});
	
	m_entityRegistry.AddComponent<Frac::TextComponent>(*HealthBar, Frac::TextComponent{ "S",1.f,{0.f,1.f,0.f},Frac::TextComponent::FontFamily::Icons });
	m_entityRegistry.AddComponent<HeathBarComponent>(*HealthBar, HeathBarComponent{});

	//Set-up Particles
	Frac::FountainParticleEmitterSettings settings;
	settings.FountainHeight = 1.f;
	settings.TimePerEmit = 0.01f;
	settings.StartSize = glm::vec3(0.05f);
	settings.IsEmissive = true;
	settings.InitialAngleOffset = 25.f;
	settings.TimeToEmit = 0.2f;
	settings.ParticleLifetime = 1.5f;
	Frac::EngineCore::GetInstance().GetParticleSystem().SpawnFountainEmitter(settings, Frac::EngineCore::GetInstance().GetFracRegistry().TranslateENTTEntity(spawner));

	auto& textComp = m_entityRegistry.GetComponent<Frac::TextComponent>(*HealthBar);
	auto& healthBarComp = m_entityRegistry.GetComponent<HeathBarComponent>(*HealthBar);
	
	textComp.overrideOrientation = true;
	healthBarComp.ComponentSize = glm::vec3(0.004f, 0.0004f, 1.0f);
	healthBarComp.overrideScale = true;
}

void EnemyEntityCreator::CreateSpawner(int id, const glm::ivec2& gridPosition, const glm::vec3& rotation, int enemySpawnType, float enemySpawnDelay) {
	Frac::Entity& spawnerEntity = *new Frac::Entity(m_entityRegistry, "Spawner " + std::to_string(createdSpawners));
	spawnerEntity.SetParent(m_sceneEntity);
	glm::vec3 size = glm::vec3{ 2.0f, 2.0f, 2.0f };
	m_entityRegistry.AddComponent<BoxComponent>(spawnerEntity, glm::vec4{ 0.0f, 0.0f, 1.0f, 1.0f });
	glm::vec3 pos =  m_entityRegistry.GetEnTTRegistry().get<TOR::Transform>(m_tileManager.GetTile(gridPosition.x, gridPosition.y)).Position;

	m_entityRegistry.AddComponent<GridComponent>(spawnerEntity, glm::ivec2{ gridPosition.x ,gridPosition.y });
	m_entityRegistry.AddComponent<TOR::Transform>(spawnerEntity, pos, size);
	m_entityRegistry.AddComponent<SpawnerComponent>(spawnerEntity);

	SpawnerComponent& spawnerComp = m_entityRegistry.GetComponent<SpawnerComponent>(spawnerEntity);
	switch (m_LevelManager->m_activeLevel)
	{
	case 0:
		spawnerComp.Stats = GameSettings::Level1SpawnerStats;
		break;
	case 1:
		spawnerComp.Stats = GameSettings::Level2SpawnerStats;
		break;
	case 2:
		spawnerComp.Stats = GameSettings::Level3SpawnerStats;
		break;
	case 3:
		spawnerComp.Stats = GameSettings::Level4SpawnerStats;
		break;
	case 4:
		spawnerComp.Stats = GameSettings::Level5SpawnerStats;
		break;
	case 5:
		spawnerComp.Stats = GameSettings::Level6SpawnerStats;
		break;
	default:

		break;
	}
	glm::vec2 SpawnerGridPos = m_entityRegistry.GetComponent<GridComponent>(spawnerEntity).GridPos;
	createdSpawners++;
}

void EnemyEntityCreator::ClearCreatedEntities()
{
	entt::registry& enttReg = m_entityRegistry.GetEnTTRegistry();

	//Destroy Turrets
	enttReg.view<EnemyComponent>().each([this, &enttReg](const auto entity, EnemyComponent& enemyComp)
		{
			enttReg.destroy(entity);
		});

	//Destroy Bullets
	enttReg.view<SpawnerComponent>().each([this, &enttReg](const auto entity, SpawnerComponent& spawnerComp)
		{
			enttReg.destroy(entity);
		});
}

EEnemyType EnemyEntityCreator::GetEnemySpawnType(int type) {
	switch (type) {
	case 0: {
		return EEnemyType::Slow;
	}
		  break;
	case 1: {
		return EEnemyType::Normal;
	}break;
	case 2: {
		return EEnemyType::Fast;
	}break;
	default:
		//Default //TODO add error message
		return EEnemyType::Slow;
		;		break;
	}
}

void EnemyEntityCreator::SetLevelManager(LevelManager& levelManager)
{
	m_LevelManager = &levelManager;
}
