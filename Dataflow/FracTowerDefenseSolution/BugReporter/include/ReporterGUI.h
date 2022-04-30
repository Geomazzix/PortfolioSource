#pragma once
#include <DearImGui\imgui.h>
#include <DearImGui\imgui_impl_glfw.h>
#include <DearImGui\imgui_impl_opengl3.h>

struct GLFWwindow;

namespace BugReporter 
{
	class ReporterGUI 
	{
	public:
		ReporterGUI(GLFWwindow& a_window, const int& a_windowWidth,const int &a_windowHeight);
		~ReporterGUI() = default;

		void Update();

		/**
		* Goes over the bug report data and checks if it's empty
		* @return: a boolean with the state whether the report is empty
		*/
		bool CheckIsEmptyReport();

		/**
		* Processes all the data from the BugReport to the JSON parser
		*/
		void ProcessReport();

		/**
		* A getter that states whether the BugReporter window should close
		* @return: a boolean with the state
		*/
		bool ShouldCloseWindow();

	private:

		void HelpMarker(const char* desc);

		const int m_windowWidth;
		const int m_windowHeight;

		char m_summary[256]{ 0 };
		char m_reproductionSteps[1024]{ 0 };
		char m_expectedOutcome[256]{ 0 };
		char m_actualOutcome[256]{ 0 };

		bool m_shouldCloseWindow;

		ImGuiStyle* m_style;

		float m_guiScale;

		void SetupImGuiStyle();
	};

} // namespace BugReporter