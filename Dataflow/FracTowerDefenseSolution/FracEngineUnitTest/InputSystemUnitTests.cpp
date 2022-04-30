#include <gtest/gtest.h>

#ifdef PLATFORM_WINDOWS

#include "Platform/PC/GLFWInput.h"

std::unique_ptr<Frac::InputSystem> CreateUniqueInputSystem()
{
	return std::make_unique<Frac::GLFWInput>(nullptr);
}

#elif PLATFORM_SWITCH

#include "Platform/Switch/SwitchInput.h"

std::unique_ptr<Frac::InputSystem> CreateUniqueInputSystem()
{
	return std::make_unique<Frac::SwitchInput>();
}

#endif // PLATFORM_SWITCH

TEST(input_system, get_active_joysticks)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	sut->AddVirtualJoystick();
	
	EXPECT_FALSE(sut->GetActiveJoysticks().empty());
}

TEST(input_system, get_default_joystick)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	sut->AddVirtualJoystick();

	EXPECT_TRUE(sut->GetDefaultJoystick().IsValid());
}

TEST(input_system, get_down)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	const std::pair<Frac::Joystick, Frac::JoystickState&> joyPair = sut->AddVirtualJoystick();
	joyPair.second.Buttons[Frac::JOYSTICK_BUTTON_DPAD_DOWN] = true;
	
	EXPECT_TRUE(sut->GetDown(joyPair.first));
}

TEST(input_system, get_up)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	const std::pair<Frac::Joystick, Frac::JoystickState&> joyPair = sut->AddVirtualJoystick();
	joyPair.second.Buttons[Frac::JOYSTICK_BUTTON_DPAD_UP] = true;

	EXPECT_TRUE(sut->GetUp(joyPair.first));
}

TEST(input_system, get_right)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	const std::pair<Frac::Joystick, Frac::JoystickState&> joyPair = sut->AddVirtualJoystick();
	joyPair.second.Buttons[Frac::JOYSTICK_BUTTON_DPAD_RIGHT] = true;

	EXPECT_TRUE(sut->GetRight(joyPair.first));
}

TEST(input_system, get_left)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	const std::pair<Frac::Joystick, Frac::JoystickState&> joyPair = sut->AddVirtualJoystick();
	joyPair.second.Buttons[Frac::JOYSTICK_BUTTON_DPAD_LEFT] = true;

	EXPECT_TRUE(sut->GetLeft(joyPair.first));
}

TEST(input_system, get_joystick_axis)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	const std::pair<Frac::Joystick, Frac::JoystickState&> joyPair = sut->AddVirtualJoystick();
	joyPair.second.Axes[Frac::JOYSTICK_AXIS_LEFT_THUMB_HORIZONTAL] = 0.5f;

	EXPECT_GE(sut->GetJoystickAxis(joyPair.first, Frac::JoystickAxes::JOYSTICK_AXIS_LEFT_THUMB_HORIZONTAL), 0.0f);
}

TEST(input_system, get_any_joystick_axis)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	const std::pair<Frac::Joystick, Frac::JoystickState&> joyPair = sut->AddVirtualJoystick();
	joyPair.second.Axes[Frac::JOYSTICK_AXIS_LEFT_THUMB_HORIZONTAL] = 0.5f;

	EXPECT_GE(sut->GetAnyJoystickAxis(Frac::JoystickAxes::JOYSTICK_AXIS_LEFT_THUMB_HORIZONTAL), 0.0f);
}

TEST(input_system, get_any_joystick_button)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	sut->AddVirtualJoystick();
	const std::pair<Frac::Joystick, Frac::JoystickState&> joyPair = sut->AddVirtualJoystick();
	joyPair.second.Buttons[Frac::JOYSTICK_BUTTON_A] = true;

	int called = 0, i = 0;
	while(i++ < 2)
	{
		if (sut->GetAnyJoystickButton(Frac::JOYSTICK_BUTTON_A))
		{
			called++;
		}
	}
	EXPECT_EQ(called, 2);
}

TEST(input_system, get_any_joystick_button_once)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	sut->AddVirtualJoystick();
	const std::pair<Frac::Joystick, Frac::JoystickState&> joyPair = sut->AddVirtualJoystick();
	joyPair.second.Buttons[Frac::JOYSTICK_BUTTON_A] = true;

	int called = 0, i = 0;
	while(i++ < 2)
	{
		if (sut->GetAnyJoystickButtonOnce(Frac::JOYSTICK_BUTTON_A))
		{
			called++;
			joyPair.second.LastButtons[Frac::JOYSTICK_BUTTON_A] = true;
		}
	}
	EXPECT_EQ(called, 1);
}

TEST(input_system, get_joystick_combo_once)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	const std::pair<Frac::Joystick, Frac::JoystickState&> joyPair = sut->AddVirtualJoystick();
	joyPair.second.Buttons[Frac::JOYSTICK_BUTTON_LB]	= true;
	joyPair.second.Buttons[Frac::JOYSTICK_BUTTON_A]		= true;

	int called = 0, i = 0;
	while(i++ < 2)
	{
		if (sut->GetJoystickComboOnce(joyPair.first.GetJoystickID(), Frac::JOYSTICK_BUTTON_LB, Frac::JOYSTICK_BUTTON_A))
		{
			called++;
			joyPair.second.LastButtons[Frac::JOYSTICK_BUTTON_LB]	= true;
			joyPair.second.LastButtons[Frac::JOYSTICK_BUTTON_A]		= true;
		}
	}
	EXPECT_EQ(called, 1);
}

TEST(input_system, get_key)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	
	EXPECT_FALSE(
		sut->GetKeyOnce(Frac::KEY_O) &&
		sut->GetKeyOnce(Frac::KEY_I));
}

TEST(input_system, get_key_once)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	
	EXPECT_FALSE(
		sut->GetKeyOnce(Frac::KEY_N) &&
		sut->GetKeyOnce(Frac::KEY_D));
}

TEST(input_system, get_mouse)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	const Frac::MouseState& mouse = sut->GetMouse();
	
	EXPECT_TRUE(mouse.m_isValid);
}

TEST(input_system, get_type)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	const std::pair<Frac::Joystick, Frac::JoystickState&> joyPair = sut->AddVirtualJoystick();

	EXPECT_EQ(sut->GetType(joyPair.first.GetJoystickID()), Frac::JoystickType::JOYSTICK_TYPE_VIRTUAL);
}

TEST(input_system, get_joystick_button)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	const std::pair<Frac::Joystick, Frac::JoystickState&> joyPair = sut->AddVirtualJoystick();
	joyPair.second.Buttons[Frac::JOYSTICK_BUTTON_A] = true;

	int called = 0, i = 0;
	while(i++ < 10)
	{
		if (sut->GetJoystickButton(joyPair.first.GetJoystickID(), Frac::JOYSTICK_BUTTON_A))
		{
			called++;
		}
	}
	EXPECT_EQ(called, 10);
}

TEST(input_system, get_joystick_button_once)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	const std::pair<Frac::Joystick, Frac::JoystickState&> joyPair = sut->AddVirtualJoystick();
	joyPair.second.Buttons[Frac::JOYSTICK_BUTTON_A]		= true;

	int called = 0, i = 0;
	while(i++ < 10)
	{
		if (sut->GetJoystickButtonOnce(joyPair.first.GetJoystickID(), Frac::JOYSTICK_BUTTON_A))
		{
			called++;
			joyPair.second.LastButtons[Frac::JOYSTICK_BUTTON_A] = true;
		}
	}
	EXPECT_EQ(called, 1);
}

TEST(input_system, add_virtual_joystick)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	const std::pair<Frac::Joystick, Frac::JoystickState&> joyPair = sut->AddVirtualJoystick();
	
	EXPECT_TRUE(joyPair.first.IsValid() && joyPair.first.IsVirtual());
	EXPECT_EQ(sut->GetType(joyPair.first.GetJoystickID()), Frac::JoystickType::JOYSTICK_TYPE_VIRTUAL);
}

// Joystick
TEST(joystick, get_joystick_id)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();
	sut->AddVirtualJoystick();
	
	const Frac::Joystick joyStick = sut->GetDefaultJoystick();
	
	EXPECT_NE(joyStick.GetJoystickID(), -1);
}

TEST(joystick, is_valid)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();

	sut->AddVirtualJoystick();
	const Frac::Joystick joyStick = sut->GetDefaultJoystick();

	EXPECT_TRUE(joyStick.IsValid());
}

TEST(joystick, is_virtual)
{
	std::unique_ptr<Frac::InputSystem> sut = CreateUniqueInputSystem();

	const std::pair<Frac::Joystick, Frac::JoystickState&> joyPair = sut->AddVirtualJoystick();

	EXPECT_TRUE(joyPair.first.IsVirtual());
}