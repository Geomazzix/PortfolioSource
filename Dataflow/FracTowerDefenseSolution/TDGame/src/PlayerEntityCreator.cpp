#include "TDpch.h"
#include "PlayerEntityCreator.h"
#include "Graphics/Components/TextComponent.h"

#include "Audio/AudioComponent.h"
#include "Audio/AudioManager.h"
#include "Graphics/ParticleRenderSystem/ParticleSystem.h"
PlayerEntityCreator::PlayerEntityCreator(TileManager& tileManager, Frac::EntityRegistry& entityReg, Frac::SceneManager& sceneManager) : IEntityCreator(tileManager, entityReg, sceneManager)
{
}

PlayerEntityCreator::~PlayerEntityCreator() {
}

void PlayerEntityCreator::CreatePlayerBase(int id, const glm::ivec2& position, const glm::vec3& rotation, float coreHealth, float coreTargetData)
{
	m_activeBaseEntity = new Frac::Entity(m_entityRegistry, "Base");
	m_activeBaseEntity->SetParent((m_sceneEntity));

	m_entityRegistry.AddComponent<BoxComponent>(*m_activeBaseEntity, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), true);
	m_entityRegistry.AddComponent<GridComponent>(*m_activeBaseEntity, position);
	m_entityRegistry.AddComponent<TOR::Transform>(*m_activeBaseEntity, m_entityRegistry.GetEnTTRegistry().get<TOR::Transform>(m_tileManager.GetTile(position.x, position.y)).Position, glm::vec3(0.25f, 0.25f, 0.25f));
	m_entityRegistry.AddComponent<DirectionComponent>(*m_activeBaseEntity);
	m_entityRegistry.AddComponent<BaseComponent>(*m_activeBaseEntity);
	m_entityRegistry.AddComponent<HealthComponent>(*m_activeBaseEntity, 100.0f, coreHealth);
	glm::vec2 BaseGridPos = m_entityRegistry.GetComponent<GridComponent>(*m_activeBaseEntity).GridPos;
	m_tileManager.SetTileOccupier(m_tileManager.GetTile((int)BaseGridPos.x, (int)BaseGridPos.y), m_activeBaseEntity->GetHandle());
	m_sceneManager.LoadModelOnEntityOnceLoaded(*m_activeBaseEntity, Frac::EModelFileType::GLTF, "Models/Core/core.gltf");

	//healthbar UI elements
	Frac::TextComponent textComp{ "Y", 1.f,{0.f,1.f,0.f},Frac::TextComponent::FontFamily::Icons };
	textComp.noBlend = true;

	HeathBarComponent healthBarComp;
	healthBarComp.ComponentSize = glm::vec3(0.06f, 0.002f, 1.0f);
	healthBarComp.fullColor = glm::vec3(1.f, 0.f, 0.f);
	healthBarComp.emptyColor = glm::vec3(0.f, 1.f, 0.f);
	healthBarComp.overrideScale = true;
	
	TOR::Transform anchorTransform{{0.0f, 2.5f, 4.5f}, healthBarComp.ComponentSize};
	anchorTransform.Orientation = glm::quat(glm::vec3(0.f, glm::radians<float>(90.f), 0.f));

	Frac::Entity* healthBar = new Frac::Entity(m_entityRegistry, "HealthBar");
	m_entityRegistry.AddComponent<TOR::Transform>(*healthBar, TOR::Transform(anchorTransform));
	m_entityRegistry.AddComponent<Frac::TextComponent>(*healthBar, Frac::TextComponent());
	m_entityRegistry.AddComponent<HeathBarComponent>(*healthBar, healthBarComp);

	healthBar->SetParent(*m_activeBaseEntity);
	
	Frac::Entity* healthInnerGraphic = new Frac::Entity(m_entityRegistry, "healthInnerGraphic");
	m_entityRegistry.AddComponent<TOR::Transform>(*healthInnerGraphic, TOR::Transform{ {214.0f, 0.0f, 0.0f} });
	m_entityRegistry.AddComponent<Frac::TextComponent>(*healthInnerGraphic, textComp);

	healthInnerGraphic->SetParent(*healthBar);
}

void PlayerEntityCreator::CreateTower(int id, const glm::ivec2& gridPos)
{
	Frac::Entity& TurretEntity = *new Frac::Entity(m_entityRegistry, "Turret " + std::to_string(m_createdTurrets));
	TurretEntity.SetParent((m_sceneEntity));

	m_entityRegistry.AddComponent<BoxComponent>(TurretEntity, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), true);
	m_entityRegistry.AddComponent<GridComponent>(TurretEntity, gridPos);
	m_entityRegistry.AddComponent<TOR::Transform>(TurretEntity, m_entityRegistry.GetEnTTRegistry().get<TOR::Transform>(m_tileManager.GetTile(gridPos.x, gridPos.y)).Position, glm::vec3(0.0f, 0.0f, 0.0f));
	m_entityRegistry.AddComponent<DirectionComponent>(TurretEntity);
	m_entityRegistry.AddComponent<TurretComponent>(TurretEntity);
	m_entityRegistry.AddComponent<VelocityComponent>(TurretEntity);
	m_entityRegistry.AddComponent<ResourceComponent>(TurretEntity);
	m_tileManager.SetTileOccupier(m_tileManager.GetTile(gridPos.x, gridPos.y), TurretEntity.GetHandle());
	m_createdTurrets++;

	TurretComponent& turComp = m_entityRegistry.GetComponent<TurretComponent>(TurretEntity);
	ResourceComponent& resComp = m_entityRegistry.GetComponent<ResourceComponent>(TurretEntity);

	m_entityRegistry.AddComponent<Frac::AudioSourceComponent>(TurretEntity, Frac::AudioSourceComponent());
	auto& audioSourceComponent = m_entityRegistry.GetComponent<Frac::AudioSourceComponent>(TurretEntity);
	Frac::EngineCore::GetInstance().GetAudioManager().AddEvent(audioSourceComponent, "event:/Tower_Shoot");

	Frac::CircleParticleEmitterSettings settings;
	settings.radius = 0.5f;
	settings.TimePerEmit = 0.005f;
	settings.StartSize = glm::vec3(0.03f);
	settings.Endsize = glm::vec3(0.015f);
	settings.StartColor = glm::vec4(glm::vec3(0x73 / 255.f, 0xDC / 255.f, 0xF3 / 255.f), 1);
	settings.EndColor = glm::vec4(glm::vec3(0x73 / 255.f, 0xDC / 255.f, 0xF3 / 255.f) * 0.25f, 1);
	settings.IsEmissive = true;
	settings.ParticleSpeed = 1.f;
	settings.TimeToEmit = 1.75f;
	settings.ParticleLifetime = 0.25f;
	settings.HasGravity = false;
	Frac::EngineCore::GetInstance().GetParticleSystem().SpawnCircleEmitter(settings, TurretEntity);

	/*Frac::LineParticleEmitterSettings settings;
	settings.TimePerEmit = 0.4f;
	settings.StartSize = glm::vec3(0.03f);
	settings.Endsize = glm::vec3(0.05f);
	settings.StartColor = glm::vec4(0, 1, 0, 1);
	settings.EndColor = glm::vec4(0, 1, 1, 1);
	settings.IsEmissive = true;
	settings.DestinationFromStartPosition = glm::vec3(0, 0.75f, -.25f);
	settings.ParticleSpeed = 7;
	settings.TimeToEmit = 1.5f;
	settings.ParticleLifetime = 0.6f;
	Frac::EngineCore::GetInstance().GetParticleSystem().SpawnLineEmitter(settings, TurretEntity);*/

	switch (ETurretType(id))
	{
	case ETurretType::Sniper:
	{
		turComp.Stats = GameSettings::Sniper;
		resComp.ResourcesWorth = turComp.Stats.ResourcesWorth;

		m_sceneManager.LoadModelOnEntity(TurretEntity, Frac::EModelFileType::GLTF, turComp.Stats.Model);
		break;
	}
	case ETurretType::Machinegun:
	{
		turComp.Stats = GameSettings::Machinegun;
		resComp.ResourcesWorth = turComp.Stats.ResourcesWorth;

		m_sceneManager.LoadModelOnEntity(TurretEntity, Frac::EModelFileType::GLTF, turComp.Stats.Model);
		break;
	}
	case ETurretType::RocketLauncher:
	{
		turComp.Stats = GameSettings::RocketLauncher;
		resComp.ResourcesWorth = turComp.Stats.ResourcesWorth;

		m_sceneManager.LoadModelOnEntity(TurretEntity, Frac::EModelFileType::GLTF, turComp.Stats.Model);
		break;
	}
	}
}

void PlayerEntityCreator::CreatePlayerBullet(const entt::entity& target, const ETurretType& type, const glm::vec3& position, const glm::vec2& gridPosition, const glm::vec3& direction)
{
	Frac::Entity& BulletEntity = *new Frac::Entity(m_entityRegistry, "Bullet" + std::to_string(m_createdBullets));
	BulletEntity.SetParent((m_sceneEntity));

	m_entityRegistry.AddComponent<GridComponent>(BulletEntity, gridPosition);
	m_entityRegistry.AddComponent<TOR::Transform>(BulletEntity, position, glm::vec3(1.5f, 1.5f, 1.5f));
	m_entityRegistry.GetComponent<TOR::Transform>(BulletEntity).Orientation = glm::quatLookAt(-direction, glm::vec3(0, 1, 0));
	//m_entityRegistry.GetComponent<TOR::Transform>(BulletEntity).Orientation *= glm::quat(glm::vec3(0, glm::radians(180.0f), 0));

	m_entityRegistry.AddComponent<BoxComponent>(BulletEntity, glm::vec4(0.2f, 0.5f, 1.0f, 1.0f), true);
	m_entityRegistry.AddComponent<BulletComponent>(BulletEntity);
	m_entityRegistry.AddComponent<DirectionComponent>(BulletEntity, direction);
	m_entityRegistry.AddComponent<VelocityComponent>(BulletEntity);
	m_createdBullets++;

	BulletComponent& bulletComp = m_entityRegistry.GetComponent<BulletComponent>(BulletEntity);
	bulletComp.Target = target;
	VelocityComponent& velComp = m_entityRegistry.GetComponent<VelocityComponent>(BulletEntity);

	TurretStats turretStats = GameSettings::IndexToTurretStats(static_cast<int>(type));

	if (type == ETurretType::Sniper)	//Bullet (Sniper Turret)
	{
		bulletComp.Stats = GameSettings::SniperProjectile;
		velComp.Speed = bulletComp.Stats.Speed * 2.f;

		m_sceneManager.LoadModelOnEntity(BulletEntity, Frac::EModelFileType::GLTF, bulletComp.Stats.Model);

		Frac::FountainParticleEmitterSettings settings;
		settings.FountainHeight = 1;
		settings.TimePerEmit = 0.0015f;
		settings.StartSize = glm::vec3(0.02f);
		settings.Endsize = glm::vec3(0.02f);
		settings.StartColor = glm::vec4(0xF0 / 255.f, 0xE0 / 255.f, 0x4D / 255.f, 1);
		settings.EndColor = glm::vec4(glm::vec3(0xF0 / 255.f, 0xE0 / 255.f, 0x4D / 255.f) * .25f, 1);
		settings.IsEmissive = true;
		settings.ParticleSpeed = 1;
		settings.HasGravity = false;
		settings.InitialAngleOffset = 10.f;
		settings.TimeToEmit = 6.0f;
		settings.ParticleLifetime = 0.25f;
		Frac::EngineCore::GetInstance().GetParticleSystem().SpawnFountainEmitter(settings, BulletEntity);
	}
	else if (type == ETurretType::Machinegun) //Bullet (Machinegun Turret)
	{
		bulletComp.Stats = GameSettings::MachinegunProjectile;
		velComp.Speed = bulletComp.Stats.Speed * 2.f;

		m_sceneManager.LoadModelOnEntity(BulletEntity, Frac::EModelFileType::GLTF, bulletComp.Stats.Model);

		Frac::FountainParticleEmitterSettings settings;
		settings.FountainHeight = 1;
		settings.TimePerEmit = 0.0025f;
		settings.StartSize = glm::vec3(0.01f);
		settings.Endsize = glm::vec3(0.03f);
		settings.StartColor = glm::vec4(glm::vec3(0x73 / 255.f, 0xDC / 255.f, 0xF3 / 255.f), 1);
		settings.EndColor = glm::vec4(glm::vec3(0x73 / 255.f, 0xDC / 255.f, 0xF3 / 255.f) * 0.25f, 1);
		settings.IsEmissive = true;
		settings.ParticleSpeed = 1;
		settings.HasGravity = false;
		settings.InitialAngleOffset = 10.f;
		settings.TimeToEmit = 6.0f;
		settings.ParticleLifetime = 0.1f;
		Frac::EngineCore::GetInstance().GetParticleSystem().SpawnFountainEmitter(settings, BulletEntity);
	}
	else if (type == ETurretType::RocketLauncher) //Missile (RocketLauncher Turret)
	{
		bulletComp.Stats = GameSettings::RocketLauncherProjectile;
		velComp.Speed = bulletComp.Stats.Speed;

		m_sceneManager.LoadModelOnEntity(BulletEntity, Frac::EModelFileType::GLTF, bulletComp.Stats.Model);

		Frac::FountainParticleEmitterSettings settings;
		settings.FountainHeight = 1;
		settings.TimePerEmit = 0.002f;
		settings.StartSize = glm::vec3(0.02f);
		settings.Endsize = glm::vec3(0.06f);
		settings.StartColor = glm::vec4(glm::vec3(0xF5 / 255.f, 0x78 / 255.f, 0xAA / 255.f), 1);
		settings.EndColor = glm::vec4(glm::vec3(0.2f, 0.2f, 0.2f), 0.5f);
		settings.IsEmissive = true;
		settings.ParticleSpeed = 1;
		settings.InitialAngleOffset = 10.f;
		settings.TimeToEmit = 6.0f;
		settings.ParticleLifetime = 0.4f;
		Frac::EngineCore::GetInstance().GetParticleSystem().SpawnFountainEmitter(settings, BulletEntity);
	}
}

void PlayerEntityCreator::ClearCreatedEntities()
{
	entt::registry& enttReg = m_entityRegistry.GetEnTTRegistry();

	//Destroy Turrets
	enttReg.view<TurretComponent>().each([this, &enttReg](const auto entity, TurretComponent& turComp)
		{
			enttReg.destroy(entity);
		});

	//Destroy Bullets
	enttReg.view<BulletComponent>().each([this, &enttReg](const auto entity, BulletComponent& bulletComp)
		{
			enttReg.destroy(entity);
		});

	//Destroy Bases
	enttReg.view<BaseComponent>().each([this, &enttReg](const auto entity, BaseComponent& baseComp)
		{
			enttReg.destroy(entity);
		});
	
	m_activeBaseEntity = nullptr;
}

const Frac::Entity& PlayerEntityCreator::GetActivePlayerBase()
{
	if (m_activeBaseEntity != nullptr) {
		return *m_activeBaseEntity;
	}
	else {
		LOGINFO("Player Base is null");
	}
}

glm::vec2 PlayerEntityCreator::GetBasePosition() const
{
	return m_basePos;
}