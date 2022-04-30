#include "TDpch.h"
#include "DeathAnimationSystem.h"

DeathAnimationSystem::DeathAnimationSystem():ISystem{Frac::eStage::OnUpdate}
{
}

DeathAnimationSystem::~DeathAnimationSystem()
{
}

void DeathAnimationSystem::Update(float DeltaTime)
{
	entt::registry& registry{ Frac::EngineCore::GetInstance().GetEnttRegistry() };
	registry.view<TOR::Transform, DeathComponent>().each([DeltaTime,&registry](const auto entity,TOR::Transform& transform, DeathComponent& deathComponent)
	{
		deathComponent.Duration -= DeltaTime;
		glm::quat newRot = glm::slerp(transform.Orientation, deathComponent.deathRotation, glm::clamp(DeltaTime, 0.f, 1.f));
		transform.Orientation = newRot;
		transform.Position.y -= DeltaTime * 0.3f;
		transform.Scale *= ((1.f - DeltaTime));
		registry.patch<TOR::Transform>(entity);
		if(deathComponent.Duration <= 0.f)
		{
			registry.destroy(entity);
		}
	});
}

void DeathAnimationSystem::clear()
{
	Frac::EngineCore::GetInstance().GetEnttRegistry().view<DeathComponent>().each([](entt::entity e, const DeathComponent& deathComponent)
		{
			Frac::EngineCore::GetInstance().GetEnttRegistry().destroy(e);
		});
}
