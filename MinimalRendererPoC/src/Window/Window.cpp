#include "MrpocPch.h"
#include "Window/Window.h"
#include "EventSystem/EventArgs.h"
#include "EventSystem/EventMessenger.h"

namespace mrpoc
{
	void Window::Initialize(const std::string& windowTitle, int clientWidth, int clientHeight)
	{
		m_mouseXPos = 0;
		m_deltaMouseXPos = 0;
		m_mouseYPos = 0;
		m_deltaMouseYPos = 0;
		if (!glfwInit())
		{
			spdlog::error("glfw failed to initialize!");
			return;
		}
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		glfwWindowHint(GLFW_SAMPLES, 8);

#if _DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
		m_Window = glfwCreateWindow(clientWidth, clientHeight, windowTitle.c_str(), NULL, NULL);
		if (m_Window == nullptr)
		{
			spdlog::error("Failed to create the GLFW window.");
			glfwTerminate();
			return;
		}

		glfwSetWindowUserPointer(m_Window, this);

		glfwMakeContextCurrent(m_Window);
		glfwSetFramebufferSizeCallback(m_Window, Window::framebuffer_size_callback);
		glfwSetKeyCallback(m_Window, Window::key_callback);
		glfwSetCursorPosCallback(m_Window, cursor_position_callback);
		glfwSetMouseButtonCallback(m_Window,mouse_button_callback);

		if (glfwRawMouseMotionSupported())
			glfwSetInputMode(m_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

		glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		if (!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress))
		{
			spdlog::error("glad failed to initialize!");
			return;
		}
			
		spdlog::info("Welcome to the Suberb Interdashing MrPoc Proof of concept!");
		spdlog::info(std::string(fmt::sprintf("GLFW %s", glfwGetVersionString())));
		spdlog::info(std::string(fmt::sprintf("OpenGLES %d.%d", GLVersion.major, GLVersion.minor)));
	}

	void Window::Destroy()
	{
		glfwDestroyWindow(m_Window);
	}

	bool Window::CheckWindowCloseRequest()
	{
		return glfwWindowShouldClose(m_Window);
	}

	void Window::SwapBuffers()
	{
		glfwSwapBuffers(m_Window);
	}

	void Window::PollWindowEvents()
	{
		glfwPollEvents();
	}

	bool Window::GetKeyState(int glfwKeyCode)
	{
		return m_KeyMap[glfwKeyCode];
	}

	void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
		ResizeEventArgs resizeArgs(width, height);
		EventMessenger::GetInstance().EvokeMessenger("OnWindowResize", resizeArgs);
	}

	void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		Window* owner = static_cast<Window*>(glfwGetWindowUserPointer(window));

		switch (action)
		{
		case GLFW_PRESS:
			owner->m_KeyMap[key] = true;
			break;
		case GLFW_RELEASE:
			owner->m_KeyMap[key] = false;
			break;
		}
	}

	void Window::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
	{
		Window* owner = static_cast<Window*>(glfwGetWindowUserPointer(window));

		owner->m_deltaMouseXPos = owner->m_mouseXPos - xpos;
		owner->m_deltaMouseYPos = owner->m_mouseYPos - ypos;
		owner->m_mouseXPos = xpos;
		owner->m_mouseYPos = ypos;
	}

	void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		Window* owner = static_cast<Window*>(glfwGetWindowUserPointer(window));

		switch (action)
		{
		case GLFW_PRESS:
			owner->m_KeyMap[button] = true;
			break;
		case GLFW_RELEASE:
			owner->m_KeyMap[button] = false;
			break;
		}
	}
}