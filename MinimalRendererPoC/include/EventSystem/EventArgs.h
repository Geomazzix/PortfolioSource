#pragma once
#include <glfw/glfw3.h>

namespace mrpoc
{
	struct EventArgs
	{
	public:
		EventArgs() = default;
		virtual ~EventArgs() = default;
	};
	struct KeyEventArgs : public EventArgs
	{
	public:
		enum class KeyState
		{
			Released = 0,
			Pressed = 1
		};

		int			    Key;    // The Key Code that was pressed or released.
		unsigned int    Char;   // The 32-bit character code that was pressed. This value will be 0 if it is a non-printable character.
		KeyState        State;  // Was the key pressed or released?
		bool            Control;// Is the Control modifier pressed
		bool            Shift;  // Is the Shift modifier pressed
		bool            Alt;    // Is the Alt modifier pressed
	};

	class MouseMotionEventArgs : public EventArgs
	{
	public:
		typedef EventArgs base;
		MouseMotionEventArgs(bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y)
			: LeftButton(leftButton)
			, MiddleButton(middleButton)
			, RightButton(rightButton)
			, Control(control)
			, Shift(shift)
			, X(x)
			, Y(y)
		{}

		bool LeftButton;    // Is the left mouse button down?
		bool MiddleButton;  // Is the middle mouse button down?
		bool RightButton;   // Is the right mouse button down?
		bool Control;       // Is the CTRL key down?
		bool Shift;         // Is the Shift key down?

		int X;              // The X-position of the cursor relative to the upper-left corner of the client area.
		int Y;              // The Y-position of the cursor relative to the upper-left corner of the client area.
		int RelX{};           // How far the mouse moved since the last event.
		int RelY{};           // How far the mouse moved since the last event.

	};

	class MouseButtonEventArgs : public EventArgs
	{
	public:
		enum class MouseButton
		{
			None = 0,
			Left = 1,
			Right = 2,
			Middel = 3
		};
		enum class ButtonState
		{
			Released = 0,
			Pressed = 1
		};

		typedef EventArgs base;
		MouseButtonEventArgs(MouseButton buttonID, ButtonState state, bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y)
			: Button(buttonID)
			, State(state)
			, LeftButton(leftButton)
			, MiddleButton(middleButton)
			, RightButton(rightButton)
			, Control(control)
			, Shift(shift)
			, X(x)
			, Y(y)
		{}

		MouseButton Button; // The mouse button that was pressed or released.
		ButtonState State;  // Was the button pressed or released?
		bool LeftButton;    // Is the left mouse button down?
		bool MiddleButton;  // Is the middle mouse button down?
		bool RightButton;   // Is the right mouse button down?
		bool Control;       // Is the CTRL key down?
		bool Shift;         // Is the Shift key down?

		int X;              // The X-position of the cursor relative to the upper-left corner of the client area.
		int Y;              // The Y-position of the cursor relative to the upper-left corner of the client area.
	};

	class MouseWheelEventArgs : public EventArgs
	{
	public:
		typedef EventArgs base;
		MouseWheelEventArgs(float wheelDelta, bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y, int deltaX, int deltaY)
			: WheelDelta(wheelDelta)
			, LeftButton(leftButton)
			, MiddleButton(middleButton)
			, RightButton(rightButton)
			, Control(control)
			, Shift(shift)
			, X(x)
			, Y(y)
		{}

		float WheelDelta;   // How much the mouse wheel has moved. A positive value indicates that the wheel was moved to the right. A negative value indicates the wheel was moved to the left.
		bool LeftButton;    // Is the left mouse button down?
		bool MiddleButton;  // Is the middle mouse button down?
		bool RightButton;   // Is the right mouse button down?
		bool Control;       // Is the CTRL key down?
		bool Shift;         // Is the Shift key down?

		int X;              // The X-position of the cursor relative to the upper-left corner of the client area.
		int Y;              // The Y-position of the cursor relative to the upper-left corner of the client area.
		int DeltaX;
		int deltaY;

	};

	class ResizeEventArgs : public EventArgs
	{
	public:
		typedef EventArgs base;
		ResizeEventArgs(int width, int height)
			: Width(width)
			, Height(height)
		{}

		// The new width of the window
		int Width;
		// The new height of the window.
		int Height;

	};

	class UserEventArgs : public EventArgs
	{
	public:
		typedef EventArgs base;
		UserEventArgs(int code, void* data1, void* data2)
			: Code(code)
			, Data1(data1)
			, Data2(data2)
		{}

		int     Code;
		void* Data1;
		void* Data2;
	};
}