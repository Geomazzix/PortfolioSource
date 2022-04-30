#pragma once
#pragma once

#include "CoreRenderAPI/RenderAPI.h"
#include "CoreRenderAPI/Instancing/PrimitiveCache.h"
#include "CoreRenderAPI/Components/Particle.h"

#include "Core/FileIO.h"
#include "Core/EntityRegistry.h"
#include "Core/ISystem.h"

namespace TOR
{
	struct Material;
	struct MaterialBatch;
}

namespace Frac
{
	enum class EmitShape {
		FOUNTAIN,
		LINE,
		RAIN,
		CIRCLE
	};

	enum class EmitMode {
		INSTANT,
		OVER_TIME
	};

	struct ParticleEmitterSettings {
		//bool EmitsLight = false; #TODO: maybe something to mess with for later.
		bool IsEmissive = false;
		bool HasGravity = false;
		glm::vec4 StartColor = glm::vec4(1, 1, 0, 1);
		glm::vec4 EndColor = glm::vec4(0, 1, 1, 1);

		glm::vec3 StartSize = glm::vec3(1, 1, 1);
		glm::vec3 Endsize = glm::vec3(0, 0, 0);

		TOR::PrimitiveShape PrimitiveShapeToEmit = TOR::PrimitiveShape::CUBE;

		float ParticleSpeed = 1.0f;
		float ParticleLifetime = 1.0f;
		float TimeToEmit = 3.0f;
		float TimePerEmit = 0.2f;
	};

	struct LineParticleEmitterSettings : ParticleEmitterSettings {
		glm::vec3 DestinationFromStartPosition = glm::vec3(0, 0, 1);
	};

	struct FountainParticleEmitterSettings : ParticleEmitterSettings {
		// How much degree's should the initial launch of the particle be offset from the world up vector.
		float InitialAngleOffset = 10;
		float FountainHeight = 2;
	};

	struct RainParticleEmitterSettings : ParticleEmitterSettings {
		float fallSpeed = 1.f;
		float rainCircleRadius = 1.f;
	};

	struct CircleParticleEmitterSettings : ParticleEmitterSettings
	{
		float radius;
	};

	/// <summary>
	/// pure Virtual emitter that emits 3D primitives to simulate particles.
	/// </summary>
	class ParticleEmitter {
	public:
		ParticleEmitter(const glm::vec3& in_offsetFromParent, Frac::Entity* in_emitterParent = nullptr);

		virtual ~ParticleEmitter() = default;

		virtual void Update(float in_dt, EntityRegistry& in_entityRegister);
	protected:
		virtual void Emit(int in_amountOfParticlesToEmit, EntityRegistry& in_entityRegister) = 0;

		bool m_updatePositionFromParent = false;

		int m_particleEmitCount;
		int m_emitterID;

		entt::entity m_parent = entt::null;
		float m_updateTime;
		float m_age = 0;
		float m_emitTime;
		float m_lastEmitTime = 0;

		glm::vec3 m_position;

		static int ParticleEmitterCount;
	};

	class LineEmitter : public ParticleEmitter {
	public:
		LineEmitter(const LineParticleEmitterSettings& in_particleEmitterSettings, const glm::vec3& in_positionToStartEmitting, Frac::Entity* in_parentOfEmitter = nullptr);

		~LineEmitter() = default;

		void Update(float in_dt, EntityRegistry& in_entityRegister) override;
	private:

		void Emit(int in_amountOfParticlesToEmit, EntityRegistry& in_entityRegister) override;

		LineParticleEmitterSettings m_settings;
	};

	class FountainEmitter : public ParticleEmitter {
	public:
		FountainEmitter(const FountainParticleEmitterSettings& in_particleEmitterSettings, const glm::vec3& in_positionToStartEmitting, Frac::Entity* in_parentOfEmitter = nullptr);
		~FountainEmitter() = default;

		void Update(float in_dt, EntityRegistry& in_entityRegister) override;
	private:

		void Emit(int in_amountOfParticlesToEmit, EntityRegistry& in_entityRegister) override;

		FountainParticleEmitterSettings m_settings;
	};

	class RainEmitter : public ParticleEmitter {
	public:
		RainEmitter(const RainParticleEmitterSettings& in_particleEmitterSettings, const glm::vec3& in_positionToStartEmitting, Frac::Entity* in_parentOfEmitter = nullptr);

		~RainEmitter() = default;

		void Update(float in_dt, EntityRegistry& in_entityRegister) override;
	private:

		void Emit(int in_amountOfParticlesToEmit, EntityRegistry& in_entityRegister) override;

		RainParticleEmitterSettings m_settings;
	};

	class CircleEmitter : public ParticleEmitter {
	public:
		CircleEmitter(const CircleParticleEmitterSettings& in_particleEmitterSettings, const glm::vec3& in_positionToStartEmitting, Frac::Entity* in_parentOfEmitter = nullptr);
		~CircleEmitter() = default;

		void Update(float in_dt, EntityRegistry& in_entityRegister) override;
	private:

		void Emit(int in_amountOfParticlesToEmit, EntityRegistry& in_entityRegister) override;

		CircleParticleEmitterSettings m_settings;
	};

	/// <summary>
	/// Particle system that updates all the particle emitters and grants access to functions to emit new particles.
	/// </summary>
	class ParticleSystem : public ISystem
	{
	public:
		ParticleSystem() = delete;
		ParticleSystem(EntityRegistry& in_entityRegistry, const Renderer& in_mainRenderer);
		~ParticleSystem() = default;

		void Update(float dt) override;

		virtual void Initialize();
		virtual void Shutdown();

		void Reset();
		ParticleSystem(const ParticleSystem&) = delete;
		ParticleSystem& operator=(const ParticleSystem&) = delete;

		/// <summary>
		/// Spawn emitter at location, delete emitter after timeToEmit + particleLifetime has passed
		/// </summary>
		/// <param name="in_particleEmitterSettings"> settings of particle emitter</param>
		/// <param name="in_positionToSpawnEmitter"> position to spawn emitter</param>
		void SpawnLineEmitter(const LineParticleEmitterSettings& in_particleEmitterSettings, const glm::vec3& in_positionToSpawnEmitter);

		/// <summary>
		/// Spawn emitter at location of entity, delete emitter after entity is deleted OR after timeToEmit + particleLifetime has passed
		/// </summary>
		void SpawnLineEmitter(const LineParticleEmitterSettings& in_particleEmitterSettings, Frac::Entity& in_EntityToParentEmitterTo);

		/// <summary>
		/// Spawn emitter at location, delete emitter after timeToEmit + particleLifetime has passed
		/// </summary>
		/// <param name="in_particleEmitterSettings"> settings of particle emitter</param>
		/// <param name="in_positionToSpawnEmitter"> position to spawn emitter</param>
		void SpawnFountainEmitter(const FountainParticleEmitterSettings& in_particleEmitterSettings, const glm::vec3& in_positionToSpawnEmitter);

		/// <summary>
		/// Spawn emitter at location of entity, delete emitter after entity is deleted OR after timeToEmit + particleLifetime has passed
		/// </summary>
		void SpawnFountainEmitter(const FountainParticleEmitterSettings& in_particleEmitterSettings, Frac::Entity& in_EntityToParentEmitterTo);

		/// <summary>
		/// Spawn emitter at location, delete emitter after timeToEmit + particleLifetime has passed
		/// </summary>
		/// <param name="in_particleEmitterSettings"> settings of particle emitter</param>
		/// <param name="in_positionToSpawnEmitter"> position to spawn emitter</param>
		void SpawnRainEmitter(const RainParticleEmitterSettings& in_particleEmitterSettings, const glm::vec3& in_positionToSpawnEmitter);

		/// <summary>
		/// Spawn emitter at location of entity, delete emitter after entity is deleted OR after timeToEmit + particleLifetime has passed
		/// </summary>
		void SpawnRainEmitter(const RainParticleEmitterSettings& in_particleEmitterSettings, Frac::Entity& in_EntityToParentEmitterTo);

		/// <summary>
		/// Spawn emitter at location, delete emitter after timeToEmit + particleLifetime has passed
		/// </summary>
		/// <param name="in_particleEmitterSettings"> settings of particle emitter</param>
		/// <param name="in_positionToSpawnEmitter"> position to spawn emitter</param>
		void SpawnCircleEmitter(const CircleParticleEmitterSettings& in_particleEmitterSettings, const glm::vec3& in_positionToSpawnEmitter);

		/// <summary>
		/// Spawn emitter at location of entity, delete emitter after entity is deleted OR after timeToEmit + particleLifetime has passed
		/// </summary>
		void SpawnCircleEmitter(const CircleParticleEmitterSettings& in_particleEmitterSettings, Frac::Entity& in_EntityToParentEmitterTo);

	private:

		EntityRegistry& m_entityRegistry;
		const Renderer& m_mainRenderer;
		TOR::RenderAPI* m_renderAPI;
		Frac::Entity* m_activeCamera;

		std::vector<std::pair<float, LineEmitter>> m_lineEmitters;
		std::vector<std::pair<float, FountainEmitter>> m_fountainEmitters;
		std::vector<std::pair<float, RainEmitter>> m_rainEmitters;
		std::vector<std::pair<float, CircleEmitter>> m_circleEmitters;
	};
}