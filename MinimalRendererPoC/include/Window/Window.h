#pragma once

namespace mrpoc
{
	/// <summary>
	/// Handles GLFW input and window ownership.
	/// </summary>
	class Window
	{
	public:
		Window() = default;
		~Window() = default;

		void Initialize(const std::string& windowTitle, int clientWidth, int clientHeight);
		void Destroy();

		bool CheckWindowCloseRequest();

		void SwapBuffers();
		void PollWindowEvents();

		bool GetKeyState(int glfwKeyCode);

		double GetDeltaMouseXPos() const { return m_deltaMouseXPos; }
		double GetDeltaMouseYPos() const { return m_deltaMouseYPos; }

		void ResetDeltaMousePositions() {m_deltaMouseXPos = 0; m_deltaMouseYPos = 0;}
	private:
		static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
		static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
		
		
		double m_mouseXPos;
		double m_deltaMouseXPos;
		double m_mouseYPos;
		double m_deltaMouseYPos;

		std::unordered_map<int, bool> m_KeyMap;

		GLFWwindow* m_Window;
	};
}