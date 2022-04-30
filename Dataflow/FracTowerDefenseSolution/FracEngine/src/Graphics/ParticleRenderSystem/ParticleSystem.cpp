#include "pch.h"
#include "Graphics/ParticleRenderSystem/ParticleSystem.h"
#include "Graphics/RenderSystem.h"
#include "CoreRenderAPI/Components/Particle.h"

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>

namespace Frac
{
	int ParticleEmitter::ParticleEmitterCount = 0;

	// Particle system runs post update so that gameplay's actions can be updated consecutively.
	ParticleSystem::ParticleSystem(EntityRegistry& in_entityRegistry, const Renderer& in_mainRenderer) : ISystem(eStage::PostUpdate)
		, m_entityRegistry(in_entityRegistry), m_mainRenderer(in_mainRenderer)
		, m_renderAPI(&in_mainRenderer.GetRenderAPI()), m_activeCamera(nullptr)
	{
	}

	void ParticleSystem::SpawnLineEmitter(const LineParticleEmitterSettings& in_particleEmitterSettings, const glm::vec3& in_positionToSpawnEmitter)
	{
#if defined(CONFIG_DEBUG) || defined (CONFIG_DEVELOP)
		if (in_particleEmitterSettings.PrimitiveShapeToEmit != TOR::PrimitiveShape::CUBE)
		{
			FRAC_ASSERT(false, "Only cubes are supported for the particle emitters right now");
		}
#endif
		float lifeTimeOfEmitter = in_particleEmitterSettings.ParticleLifetime + in_particleEmitterSettings.TimeToEmit;

		m_lineEmitters.emplace_back(std::make_pair(lifeTimeOfEmitter, LineEmitter(in_particleEmitterSettings, in_positionToSpawnEmitter, nullptr)));
	}

	void ParticleSystem::SpawnLineEmitter(const LineParticleEmitterSettings& in_particleEmitterSettings, Frac::Entity& in_EntityToParentEmitterTo)
	{
#if defined(CONFIG_DEBUG) || defined (CONFIG_DEVELOP)
		if (in_particleEmitterSettings.PrimitiveShapeToEmit != TOR::PrimitiveShape::CUBE)
		{
			FRAC_ASSERT(false, "Only cubes are supported for the particle emitters right now");
		}
#endif
		float lifeTimeOfEmitter = in_particleEmitterSettings.ParticleLifetime + in_particleEmitterSettings.TimeToEmit;

		m_lineEmitters.emplace_back(std::make_pair(lifeTimeOfEmitter, LineEmitter(in_particleEmitterSettings, glm::vec3(0, 0, 0), &in_EntityToParentEmitterTo)));
	}

	void ParticleSystem::SpawnFountainEmitter(const FountainParticleEmitterSettings& in_particleEmitterSettings, const glm::vec3& in_positionToSpawnEmitter)
	{
#if defined(CONFIG_DEBUG) || defined (CONFIG_DEVELOP)
		if (in_particleEmitterSettings.PrimitiveShapeToEmit != TOR::PrimitiveShape::CUBE)
		{
			FRAC_ASSERT(false, "Only cubes are supported for the particle emitters right now");
		}
#endif
		float lifeTimeOfEmitter = in_particleEmitterSettings.ParticleLifetime + in_particleEmitterSettings.TimeToEmit;

		m_fountainEmitters.emplace_back(std::make_pair(lifeTimeOfEmitter, FountainEmitter(in_particleEmitterSettings, in_positionToSpawnEmitter, nullptr)));
	}

	void ParticleSystem::SpawnFountainEmitter(const FountainParticleEmitterSettings& in_particleEmitterSettings, Frac::Entity& in_EntityToParentEmitterTo)
	{
#if defined(CONFIG_DEBUG) || defined (CONFIG_DEVELOP)
		if (in_particleEmitterSettings.PrimitiveShapeToEmit != TOR::PrimitiveShape::CUBE)
		{
			FRAC_ASSERT(false, "Only cubes are supported for the particle emitters right now");
		}
#endif
		float lifeTimeOfEmitter = in_particleEmitterSettings.ParticleLifetime + in_particleEmitterSettings.TimeToEmit;

		m_fountainEmitters.emplace_back(std::make_pair(lifeTimeOfEmitter, FountainEmitter(in_particleEmitterSettings, glm::vec3(0, 0, 0), &in_EntityToParentEmitterTo)));
	}

	void ParticleSystem::SpawnRainEmitter(const RainParticleEmitterSettings& in_particleEmitterSettings, const glm::vec3& in_positionToSpawnEmitter)
	{
#if defined(CONFIG_DEBUG) || defined (CONFIG_DEVELOP)
		if (in_particleEmitterSettings.PrimitiveShapeToEmit != TOR::PrimitiveShape::CUBE)
		{
			FRAC_ASSERT(false, "Only cubes are supported for the particle emitters right now");
		}
#endif
		float lifeTimeOfEmitter = in_particleEmitterSettings.ParticleLifetime + in_particleEmitterSettings.TimeToEmit;

		m_rainEmitters.emplace_back(std::make_pair(lifeTimeOfEmitter, RainEmitter(in_particleEmitterSettings, in_positionToSpawnEmitter, nullptr)));
	}

	void ParticleSystem::SpawnRainEmitter(const RainParticleEmitterSettings& in_particleEmitterSettings, Frac::Entity& in_EntityToParentEmitterTo)
	{
#if defined(CONFIG_DEBUG) || defined (CONFIG_DEVELOP)
		if (in_particleEmitterSettings.PrimitiveShapeToEmit != TOR::PrimitiveShape::CUBE)
		{
			FRAC_ASSERT(false, "Only cubes are supported for the particle emitters right now");
		}
#endif
		float lifeTimeOfEmitter = in_particleEmitterSettings.ParticleLifetime + in_particleEmitterSettings.TimeToEmit;

		m_rainEmitters.emplace_back(std::make_pair(lifeTimeOfEmitter, RainEmitter(in_particleEmitterSettings, glm::vec3(0, 0, 0), &in_EntityToParentEmitterTo)));
	}

	void ParticleSystem::SpawnCircleEmitter(const CircleParticleEmitterSettings& in_particleEmitterSettings,
		const glm::vec3& in_positionToSpawnEmitter)
	{
#if defined(CONFIG_DEBUG) || defined (CONFIG_DEVELOP)
		if (in_particleEmitterSettings.PrimitiveShapeToEmit != TOR::PrimitiveShape::CUBE)
		{
			FRAC_ASSERT(false, "Only cubes are supported for the particle emitters right now");
		}
#endif
		float lifeTimeOfEmitter = in_particleEmitterSettings.ParticleLifetime + in_particleEmitterSettings.TimeToEmit;

		m_circleEmitters.emplace_back(std::make_pair(lifeTimeOfEmitter, CircleEmitter(in_particleEmitterSettings, in_positionToSpawnEmitter, nullptr)));
	}

	void ParticleSystem::SpawnCircleEmitter(const CircleParticleEmitterSettings& in_particleEmitterSettings,
		Frac::Entity& in_EntityToParentEmitterTo)
	{
#if defined(CONFIG_DEBUG) || defined (CONFIG_DEVELOP)
		if (in_particleEmitterSettings.PrimitiveShapeToEmit != TOR::PrimitiveShape::CUBE)
		{
			FRAC_ASSERT(false, "Only cubes are supported for the particle emitters right now");
		}
#endif
		float lifeTimeOfEmitter = in_particleEmitterSettings.ParticleLifetime + in_particleEmitterSettings.TimeToEmit;

		m_circleEmitters.emplace_back(std::make_pair(lifeTimeOfEmitter, CircleEmitter(in_particleEmitterSettings, glm::vec3(0, 0, 0), &in_EntityToParentEmitterTo)));
	}

	void ParticleSystem::Update(float dt)
	{
		std::vector<int> indexesToRemove;
		int count = 0;
		// Update all Rain emitters
		for (auto& emitterPair : m_rainEmitters)
		{
			float& timeToLive = emitterPair.first;
			RainEmitter& particleEmitter = emitterPair.second;

			particleEmitter.Update(dt, m_entityRegistry);

			timeToLive -= dt;
			if (timeToLive <= 0)
			{
				indexesToRemove.push_back(count);
			}
			count++;
		}

		// Remove emitters that have died.
		for (int i = indexesToRemove.size(); i > 0; i--)
		{
			m_rainEmitters.erase(m_rainEmitters.begin() + indexesToRemove[i - 1]);
		}
		count = 0;
		indexesToRemove.clear();

		// Update all Fountain emitters
		for (auto& emitterPair : m_fountainEmitters)
		{
			float& timeToLive = emitterPair.first;
			FountainEmitter& particleEmitter = emitterPair.second;

			particleEmitter.Update(dt, m_entityRegistry);

			timeToLive -= dt;
			if (timeToLive <= 0)
			{
				indexesToRemove.push_back(count);
			}
			count++;
		}

		// Remove emitters that have died.
		for (int i = indexesToRemove.size(); i > 0; i--)
		{
			m_fountainEmitters.erase(m_fountainEmitters.begin() + indexesToRemove[i - 1]);
		}
		count = 0;
		indexesToRemove.clear();

		// Update all Line emitters
		for (auto& emitterPair : m_lineEmitters)
		{
			float& timeToLive = emitterPair.first;
			LineEmitter& particleEmitter = emitterPair.second;

			particleEmitter.Update(dt, m_entityRegistry);

			timeToLive -= dt;
			if (timeToLive <= 0)
			{
				indexesToRemove.push_back(count);
			}
			count++;
		}

		// Remove emitters that have died.
		for (int i = indexesToRemove.size(); i > 0; i--)
		{
			m_lineEmitters.erase(m_lineEmitters.begin() + indexesToRemove[i - 1]);
		}
		count = 0;
		indexesToRemove.clear();

		// Update all circle emitters
		for (auto& emitterPair : m_circleEmitters)
		{
			float& timeToLive = emitterPair.first;
			CircleEmitter& particleEmitter = emitterPair.second;

			particleEmitter.Update(dt, m_entityRegistry);

			timeToLive -= dt;
			if (timeToLive <= 0)
			{
				indexesToRemove.push_back(count);
			}
			count++;
		}

		// Remove emitters that have died.
		for (int i = indexesToRemove.size(); i > 0; i--)
		{
			m_circleEmitters.erase(m_circleEmitters.begin() + indexesToRemove[i - 1]);
		}
		count = 0;
		indexesToRemove.clear();

		// Update all particles
		entt::registry& entityRegister = m_entityRegistry.GetEnTTRegistry();
		auto renderableView = entityRegister.view<TOR::Particle>();
		std::vector<entt::entity> entitiesToDelete;

		renderableView.each([this, dt, &entitiesToDelete, &entityRegister](entt::entity handle, TOR::Particle& particle)
			{
				particle.Age += dt;
				if (particle.TotalLifetime < particle.Age)
				{
					// delete
					entitiesToDelete.push_back(handle);
				}
				//  Update movement
				else
				{
					glm::vec3 newMovementVector = particle.MovementDirection;
					newMovementVector += particle.HasGravity ? glm::vec3(0, -9.81f, 0) * dt : glm::vec3(0);
					particle.MovementDirection = newMovementVector;
					particle.Position += newMovementVector * dt * particle.MovementSpeed;

					glm::vec3 rotationAxis = glm::cross(newMovementVector, glm::vec3(0, 1, 0));
					particle.Rotation = glm::rotate(particle.Rotation, dt, rotationAxis);
				}
			});

		while (!entitiesToDelete.empty())
		{
			entt::entity toDelete = entitiesToDelete[entitiesToDelete.size() - 1];
			entitiesToDelete.pop_back();
			m_entityRegistry.GetEnTTRegistry().destroy(toDelete);
		}
	}

	void ParticleSystem::Initialize()
	{
	}

	void ParticleSystem::Shutdown()
	{
	}

	void ParticleSystem::Reset()
	{
		m_lineEmitters.clear();
		m_fountainEmitters.clear();
		m_rainEmitters.clear();
		m_circleEmitters.clear();
		entt::registry& entityRegister = m_entityRegistry.GetEnTTRegistry();

		auto renderableView = entityRegister.view<TOR::Particle>();
		renderableView.each([this, &entityRegister](entt::entity handle, TOR::Particle& particle)
			{
				particle.Age = 999999999;
				particle.Position = glm::vec3(-10000, -10000, -10000);
			});
	}

	ParticleEmitter::ParticleEmitter(const glm::vec3& in_offsetFromParent, Frac::Entity* in_emitterParent /*= nullptr*/)
	{
		if (in_emitterParent != nullptr)
		{
			m_updatePositionFromParent = true;
			m_parent = in_emitterParent->GetHandle();
		}
		m_position = in_offsetFromParent;
		m_emitterID = ++ParticleEmitterCount;
	}

	void ParticleEmitter::Update(float in_dt, EntityRegistry& in_entityRegister)
	{
		m_updateTime = m_age + in_dt > m_emitTime ? m_emitTime - m_age : in_dt;
		m_age += in_dt;
	}

	LineEmitter::LineEmitter(const LineParticleEmitterSettings& in_particleEmitterSettings, const glm::vec3& in_positionToStartEmitting, Frac::Entity* in_parentOfEmitter /*= nullptr*/)
		: ParticleEmitter(in_positionToStartEmitting, in_parentOfEmitter), m_settings(in_particleEmitterSettings)
	{
		m_emitTime = in_particleEmitterSettings.TimeToEmit;
	}

	void LineEmitter::Update(float in_dt, EntityRegistry& in_entityRegister)
	{
		if (!m_updatePositionFromParent || m_updatePositionFromParent && m_parent != entt::null && in_entityRegister.GetEnTTRegistry().valid(m_parent))
		{
			ParticleEmitter::Update(in_dt, in_entityRegister);
			int amountOfEmits = glm::floor((m_age - m_lastEmitTime) / m_settings.TimePerEmit);
			m_lastEmitTime = m_lastEmitTime + amountOfEmits * m_settings.TimePerEmit;
			Emit(amountOfEmits, in_entityRegister);
		}
	}

	void LineEmitter::Emit(int in_amountOfParticlesToEmit, EntityRegistry& in_entityRegister)
	{
		for (int i = 0; i < in_amountOfParticlesToEmit; ++i)
		{
			Frac::Entity* newParticle = new Frac::Entity(in_entityRegister, "LineParticle_EmitterID:" + std::to_string(m_emitterID) + "_ParticleID:" + std::to_string(++m_particleEmitCount));
			TOR::Particle& newParticleStruct = in_entityRegister.GetComponent<TOR::Particle>(in_entityRegister.AddComponent<TOR::Particle>(*newParticle));

			newParticleStruct.StartColor = m_settings.StartColor;
			newParticleStruct.EndColor = m_settings.EndColor;
			newParticleStruct.StartSize = m_settings.StartSize;
			newParticleStruct.EndSize = m_settings.Endsize;

			newParticleStruct.TotalLifetime = m_settings.ParticleLifetime;
			newParticleStruct.HasGravity = m_settings.HasGravity;

			newParticleStruct.Age = 0;

			newParticleStruct.ParticleShape = m_settings.PrimitiveShapeToEmit;
			newParticleStruct.IsEmissive = m_settings.IsEmissive;

			newParticleStruct.MovementSpeed = m_settings.ParticleSpeed;
			newParticleStruct.MovementDirection = glm::normalize(m_settings.DestinationFromStartPosition);
			if (m_updatePositionFromParent && in_entityRegister.HasComponent<TOR::Transform>(in_entityRegister.TranslateENTTEntity(m_parent)))
			{
				newParticleStruct.Position = m_position + (m_updatePositionFromParent ? glm::vec3(in_entityRegister.GetComponent<TOR::Transform>(in_entityRegister.TranslateENTTEntity(m_parent)).ModelTransformData[3]) : glm::vec3(-50, -50, -50));
			}
			else
			{
				if (m_updatePositionFromParent)
				{
					m_age = 500.f;
					newParticleStruct.Position = glm::vec3(-50, -50, -50);
					in_entityRegister.GetEnTTRegistry().destroy(newParticle->GetHandle());
					break;
				}
				else
				{
					newParticleStruct.Position = m_position;
				}
			}
		}
	}

	FountainEmitter::FountainEmitter(const FountainParticleEmitterSettings& in_particleEmitterSettings, const glm::vec3& in_positionToStartEmitting, Frac::Entity* in_parentOfEmitter /*= nullptr*/)
		: ParticleEmitter(in_positionToStartEmitting, in_parentOfEmitter), m_settings(in_particleEmitterSettings)
	{
		m_emitTime = in_particleEmitterSettings.TimeToEmit;
	}

	void FountainEmitter::Update(float in_dt, EntityRegistry& in_entityRegister)
	{
		if (!m_updatePositionFromParent || (m_updatePositionFromParent && m_parent != entt::null && in_entityRegister.GetEnTTRegistry().valid(m_parent)))
		{
			ParticleEmitter::Update(in_dt, in_entityRegister);
			int amountOfEmits = glm::floor((m_age - m_lastEmitTime) / m_settings.TimePerEmit);
			m_lastEmitTime = m_lastEmitTime + amountOfEmits * m_settings.TimePerEmit;
			Emit(amountOfEmits, in_entityRegister);
		}
	}

	void FountainEmitter::Emit(int in_amountOfParticlesToEmit, EntityRegistry& in_entityRegister)
	{
		for (int i = 0; i < in_amountOfParticlesToEmit; ++i)
		{
			Frac::Entity* newParticle = new Frac::Entity(in_entityRegister, "FountainParticle_EmitterID:" + std::to_string(m_emitterID) + "_ParticleID:" + std::to_string(++m_particleEmitCount));
			TOR::Particle& newParticleStruct = in_entityRegister.GetComponent<TOR::Particle>(in_entityRegister.AddComponent<TOR::Particle>(*newParticle));

			newParticleStruct.StartColor = m_settings.StartColor;
			newParticleStruct.EndColor = m_settings.EndColor;
			newParticleStruct.StartSize = m_settings.StartSize;
			newParticleStruct.EndSize = m_settings.Endsize;

			newParticleStruct.TotalLifetime = m_settings.ParticleLifetime;
			newParticleStruct.HasGravity = m_settings.HasGravity;
			newParticleStruct.Age = 0;

			newParticleStruct.ParticleShape = m_settings.PrimitiveShapeToEmit;
			newParticleStruct.IsEmissive = m_settings.IsEmissive;

			newParticleStruct.MovementSpeed = m_settings.ParticleSpeed;
			newParticleStruct.MovementDirection = m_settings.FountainHeight * glm::rotateY(glm::rotateZ(glm::vec3(0, 1, 0), glm::radians(m_settings.InitialAngleOffset)), static_cast<float>(rand()) / (static_cast <float> (RAND_MAX / (glm::pi<float>() * 2))));//glm::normalize(m_settings.DestinationFromStartPosition);
			if (m_updatePositionFromParent && in_entityRegister.HasComponent<TOR::Transform>(in_entityRegister.TranslateENTTEntity(m_parent))) {
				newParticleStruct.Position = m_position + (m_updatePositionFromParent ? glm::vec3(in_entityRegister.GetComponent<TOR::Transform>(in_entityRegister.TranslateENTTEntity(m_parent)).ModelTransformData[3]) : glm::vec3(-50, -50, -50));
				glm::vec3 test = newParticleStruct.Position;
			}
			else
			{
				if (m_updatePositionFromParent)
				{
					m_age = 500.f;
					newParticleStruct.Position = glm::vec3(-50, -50, -50);
					in_entityRegister.GetEnTTRegistry().destroy(newParticle->GetHandle());
					break;
				}
				else
				{
					newParticleStruct.Position = m_position;
				}
			}
		}
	}

	RainEmitter::RainEmitter(const RainParticleEmitterSettings& in_particleEmitterSettings, const glm::vec3& in_positionToStartEmitting, Frac::Entity* in_parentOfEmitter /*= nullptr*/)
		: ParticleEmitter(in_positionToStartEmitting, in_parentOfEmitter), m_settings(in_particleEmitterSettings)
	{
		m_emitTime = in_particleEmitterSettings.TimeToEmit;
	}

	void RainEmitter::Update(float in_dt, EntityRegistry& in_entityRegister)
	{
		if (!m_updatePositionFromParent || m_updatePositionFromParent && m_parent != entt::null && in_entityRegister.GetEnTTRegistry().valid(m_parent))
		{
			ParticleEmitter::Update(in_dt, in_entityRegister);
			int amountOfEmits = glm::floor((m_age - m_lastEmitTime) / m_settings.TimePerEmit);
			m_lastEmitTime = m_lastEmitTime + amountOfEmits * m_settings.TimePerEmit;
			Emit(amountOfEmits, in_entityRegister);
		}
	}

	void RainEmitter::Emit(int in_amountOfParticlesToEmit, EntityRegistry& in_entityRegister)
	{
		for (int i = 0; i < in_amountOfParticlesToEmit; ++i)
		{
			Frac::Entity* newParticle = new Frac::Entity(in_entityRegister, "RainParticle_EmitterID:" + std::to_string(m_emitterID) + "_ParticleID:" + std::to_string(++m_particleEmitCount));
			TOR::Particle& newParticleStruct = in_entityRegister.GetComponent<TOR::Particle>(in_entityRegister.AddComponent<TOR::Particle>(*newParticle));

			newParticleStruct.StartColor = m_settings.StartColor;
			newParticleStruct.EndColor = m_settings.EndColor;
			newParticleStruct.StartSize = m_settings.StartSize;
			newParticleStruct.EndSize = m_settings.Endsize;

			newParticleStruct.TotalLifetime = m_settings.ParticleLifetime;
			newParticleStruct.HasGravity = m_settings.HasGravity;

			float randomZeroTo2PiValue = static_cast<float>(rand()) / (static_cast <float> (RAND_MAX / (2 * glm::pi<float>())));
			float randomLength = static_cast<float>(rand()) / (static_cast <float> (RAND_MAX / m_settings.rainCircleRadius));
			glm::vec3 RandomDiskSpace = randomLength * (glm::cos(randomZeroTo2PiValue) * glm::vec3(1, 0, 0) + glm::sin(randomZeroTo2PiValue) * glm::vec3(0, 0, -1));

			newParticleStruct.Age = 0;

			newParticleStruct.ParticleShape = m_settings.PrimitiveShapeToEmit;
			newParticleStruct.IsEmissive = m_settings.IsEmissive;

			newParticleStruct.MovementSpeed = m_settings.ParticleSpeed;
			// #TODO: Maybe make it possible for users to set a rain angle?
			newParticleStruct.MovementDirection = glm::vec3(0, -1, 0);

			if (m_updatePositionFromParent && in_entityRegister.HasComponent<TOR::Transform>(in_entityRegister.TranslateENTTEntity(m_parent)))
			{
				newParticleStruct.Position = RandomDiskSpace + m_position + (m_updatePositionFromParent ? glm::vec3(in_entityRegister.GetComponent<TOR::Transform>(in_entityRegister.TranslateENTTEntity(m_parent)).ModelTransformData[3]) : glm::vec3(-50, -50, -50));
			}
			else
			{
				if (m_updatePositionFromParent)
				{
					m_age = 500.f;
					newParticleStruct.Position = glm::vec3(-50, -50, -50);
					in_entityRegister.GetEnTTRegistry().destroy(newParticle->GetHandle());
					break;
				}
				else
				{
					newParticleStruct.Position = m_position;
				}
			}
		}
	}

	CircleEmitter::CircleEmitter(const CircleParticleEmitterSettings& in_particleEmitterSettings,
		const glm::vec3& in_positionToStartEmitting, Frac::Entity* in_parentOfEmitter)
		: ParticleEmitter(in_positionToStartEmitting, in_parentOfEmitter), m_settings(in_particleEmitterSettings)
	{
		m_emitTime = in_particleEmitterSettings.TimeToEmit;
	}

	void CircleEmitter::Update(float in_dt, EntityRegistry& in_entityRegister)
	{
		if (!m_updatePositionFromParent || m_updatePositionFromParent && m_parent != entt::null && in_entityRegister.GetEnTTRegistry().valid(m_parent))
		{
			ParticleEmitter::Update(in_dt, in_entityRegister);
			int amountOfEmits = glm::floor((m_age - m_lastEmitTime) / m_settings.TimePerEmit);
			m_lastEmitTime = m_lastEmitTime + amountOfEmits * m_settings.TimePerEmit;
			Emit(amountOfEmits, in_entityRegister);
		}
	}

	void CircleEmitter::Emit(int in_amountOfParticlesToEmit, EntityRegistry& in_entityRegister)
	{
		for (int i = 0; i < in_amountOfParticlesToEmit; ++i)
		{
			Frac::Entity* newParticle = new Frac::Entity(in_entityRegister, "LineParticle_EmitterID:" + std::to_string(m_emitterID) + "_ParticleID:" + std::to_string(++m_particleEmitCount));
			TOR::Particle& newParticleStruct = in_entityRegister.GetComponent<TOR::Particle>(in_entityRegister.AddComponent<TOR::Particle>(*newParticle));

			newParticleStruct.StartColor = m_settings.StartColor;
			newParticleStruct.EndColor = m_settings.EndColor;
			newParticleStruct.StartSize = m_settings.StartSize;
			newParticleStruct.EndSize = m_settings.Endsize;
			newParticleStruct.TotalLifetime = m_settings.ParticleLifetime;
			newParticleStruct.HasGravity = m_settings.HasGravity;
			glm::vec2 diskrand = glm::circularRand(m_settings.radius);
			glm::vec3 diskrandoff = glm::vec3{ diskrand.x,0.f,diskrand.y };

			newParticleStruct.ParticleShape = m_settings.PrimitiveShapeToEmit;
			newParticleStruct.IsEmissive = m_settings.IsEmissive;

			newParticleStruct.MovementSpeed = m_settings.ParticleSpeed;
			newParticleStruct.MovementDirection = glm::normalize(diskrandoff);
			if (m_updatePositionFromParent && in_entityRegister.HasComponent<TOR::Transform>(in_entityRegister.TranslateENTTEntity(m_parent)))
			{
				newParticleStruct.Position = diskrandoff + m_position + (m_updatePositionFromParent ? glm::vec3(in_entityRegister.GetComponent<TOR::Transform>(in_entityRegister.TranslateENTTEntity(m_parent)).ModelTransformData[3]) : glm::vec3(0, 0, 0));
			}
			else
			{
				if (m_updatePositionFromParent)
				{
					m_age = 500.f;
					newParticleStruct.Position = glm::vec3(-50, -50, -50);
					in_entityRegister.GetEnTTRegistry().destroy(newParticle->GetHandle());
					break;
				}
				else
				{
					newParticleStruct.Position = m_position;
				}
			}
		}
	}
}