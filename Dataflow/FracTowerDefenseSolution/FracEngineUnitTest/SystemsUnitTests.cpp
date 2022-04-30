#include <gtest/gtest.h>

#include "Core/EngineCore.h"
#include <Core/ISystem.h>
class TestSystem: public Frac::ISystem
{
public:
	TestSystem(Frac::eStage stage = Frac::eStage::OnUpdate):ISystem{ stage }
	{}
	bool run = false;
	void Update(float DeltaTime) override
	{
		run = true;
		Frac::EngineCore::GetInstance().SetRunState(false);
	}
};


TEST(SystemTests,Creating_a_system)
{

	TestSystem sut{};
	Frac::EngineCore::GetInstance().SetRunState(true);
	Frac::EngineCore::GetInstance().Run();

	ASSERT_TRUE(sut.run);
}

TEST(SystemTests, disabled_systems_should_not_run)
{
	TestSystem sut{};
	auto info = sut.GetSystemInfo();
	info.enabled = false;
	sut.SetSystemInfo(info);
	Frac::EngineCore::GetInstance().SetRunState(true);
	Frac::EngineCore::GetInstance().Run();

	ASSERT_FALSE(sut.run);
}