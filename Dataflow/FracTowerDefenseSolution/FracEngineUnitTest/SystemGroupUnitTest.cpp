#include <gtest/gtest.h>

#include "Core/EngineCore.h"
#include <Core/ISystem.h>
#include <Core/SystemGroup.h>
class TestSystem : public Frac::ISystem
{
public:
	TestSystem(Frac::eStage stage = Frac::eStage::OnUpdate, bool DisableOnRun = true) :ISystem{ stage }
	{
		m_disableOnRun = DisableOnRun;
	}
	bool run = false;
	void Update(float DeltaTime) override
	{
		run = true;
		if(m_disableOnRun)
			Frac::EngineCore::GetInstance().SetRunState(false);
	}
private:
	bool m_disableOnRun;
};

TEST(SystemGroup, making_a_group)
{
	TestSystem s1{Frac::eStage::PreUpdate,false};
	TestSystem s2{Frac::eStage::OnUpdate,false};
	TestSystem s3{Frac::eStage::PostUpdate,true};
	Frac::SystemInfo info = s2.GetSystemInfo();
	info.enabled = false;
	s2.SetSystemInfo(info);
	Frac::SystemGroup sut{};
	sut.AddSystem(&s1);
	sut.AddSystem(&s2);
	sut.AddSystem(&s3);

	sut.EnableAll();
	Frac::EngineCore::GetInstance().SetRunState(true);
	Frac::EngineCore::GetInstance().Run();

	ASSERT_TRUE(s2.run);
}