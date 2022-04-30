#pragma once
#include "Glm/glm.hpp"
#include "CoreRenderAPI/Instancing/PrimitiveCache.h"
#include "Glm/gtx/quaternion.hpp"

namespace TOR
{
	struct Particle {
		Particle(/*TOR::PrimitiveShape in_particleShape, float in_particleLifetime,
			const glm::vec4& in_startColor, const glm::vec4& in_endColor,
			const glm::vec3& in_startSize, const glm::vec3& in_endSize,
			const glm::vec3& in_initialMovementDirection, float in_initialMovementSpeed) :
			ParticleShape(in_particleShape), TotalLifetime(in_particleLifetime), StartColor(in_startColor),
			EndColor(in_endColor), StartSize(in_startSize), EndSize(in_endSize), MovementDirection(in_initialMovementDirection),
			MovementSpeed(in_initialMovementSpeed*/)
		{}

		TOR::PrimitiveShape ParticleShape;

		bool HasGravity = false;

		bool IsEmissive = false;

		float Age = 0.0f;
		float TotalLifetime;
		float MovementSpeed;

		glm::vec4 StartColor;
		glm::vec4 EndColor;
		glm::vec3 StartSize;
		glm::vec3 EndSize;

		glm::vec3 MovementDirection;

		// Launch velocity, this should slowly go to 1 or stay below 1

		glm::vec3 Position;
		glm::quat Rotation = glm::identity<glm::quat>();
	};
}