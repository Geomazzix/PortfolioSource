#pragma once
#include "Core/ISystem.h"

class SpawnAnimationSystem : public Frac::ISystem
{
public:
	SpawnAnimationSystem();
	~SpawnAnimationSystem();
	void Update(float DeltaTime) override;
};




