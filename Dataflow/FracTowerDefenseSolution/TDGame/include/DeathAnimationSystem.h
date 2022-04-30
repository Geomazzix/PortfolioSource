#pragma once
#include <Core/ISystem.h>
class DeathAnimationSystem: public Frac::ISystem
{
public:
	DeathAnimationSystem();
	~DeathAnimationSystem();
	void Update(float DeltaTime) override;
	void clear();
};

