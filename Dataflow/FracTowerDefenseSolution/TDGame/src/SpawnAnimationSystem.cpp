#include "TDpch.h"
#include "SpawnAnimationSystem.h"

SpawnAnimationSystem::SpawnAnimationSystem(): ISystem{Frac::eStage::OnUpdate}
{
}

SpawnAnimationSystem::~SpawnAnimationSystem()
{
}

void SpawnAnimationSystem::Update(float DeltaTime)
{
	entt::registry& reg{ Frac::EngineCore::GetInstance().GetEnttRegistry() };
	reg.view<TOR::Transform, SpawnComponent>().each([&reg,DeltaTime](entt::entity entity,TOR::Transform& transform, SpawnComponent& spawnComponent)
	{
		spawnComponent.timer += DeltaTime;
		transform.Scale = glm::mix(spawnComponent.beginScale, spawnComponent.endScale, glm::clamp(spawnComponent.timer / spawnComponent.duration, 0.f, 1.f));
	
		if(spawnComponent.timer >= spawnComponent.duration)
		{
			transform.Scale = spawnComponent.endScale;
			reg.remove<SpawnComponent>(entity);
		}
		reg.patch<TOR::Transform>(entity);
		
	});
}
