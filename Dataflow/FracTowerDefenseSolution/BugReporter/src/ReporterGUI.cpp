#include "ReporterGUI.h"
#include <GLFW/glfw3.h>

#include <stdio.h>

#include "JsonParser.h"

namespace BugReporter
{
	ReporterGUI::ReporterGUI(GLFWwindow& a_window, const int& a_windowWidth, const int& a_windowHeight) :
        m_windowWidth(a_windowWidth),
        m_windowHeight(a_windowHeight),
        m_shouldCloseWindow(false),
        m_guiScale(1.5f)
	{
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.Fonts->AddFontFromFileTTF("Assets/Fonts/NotoMono-Regular.ttf", roundf(15.0f * m_guiScale));

        // Setup Dear ImGui style
        SetupImGuiStyle();
        m_style->ScaleAllSizes(m_guiScale);

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(&a_window, true);
        // GL 3.0 + GLSL 130
        const char* glsl_version = "#version 130";
        ImGui_ImplOpenGL3_Init(glsl_version);
	}

	void ReporterGUI::Update()
	{
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            bool p_open{ true };
            int imguiID = 0;
            ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
            ImGui::SetNextWindowSize(ImVec2(m_windowWidth, m_windowHeight));
            ImGui::Begin("BugReport", &p_open, ImGuiWindowFlags_NoCollapse || ImGuiWindowFlags_NoTitleBar || ImGuiWindowFlags_NoResize || ImGuiWindowFlags_NoMove || ImGuiWindowFlags_NoSavedSettings);

            ImGui::Text("Please use this form to report a bug that was experienced while playing the DataFlow TowerDefense game");
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("Summary");
            ImGui::SameLine();
            HelpMarker("Tell us in short what the bug is about. (e.g.: The game crashed)");
            ImGui::PushID(imguiID++);
            ImGui::InputText("", m_summary, IM_ARRAYSIZE(m_summary));
            ImGui::PopID();
            ImGui::Spacing();
            ImGui::Text("Reproduction steps");
            ImGui::SameLine();
            HelpMarker("Tell us in detail what steps you took right before the bug happened.");
            ImGui::PushID(imguiID++);
            ImGui::InputTextMultiline("", m_reproductionSteps, IM_ARRAYSIZE(m_reproductionSteps));
            ImGui::PopID();
            ImGui::Spacing();
            ImGui::Text("Expected outcome");
            ImGui::SameLine();
            HelpMarker("What should have happened.");
            ImGui::PushID(imguiID++);
            ImGui::InputText("", m_expectedOutcome, IM_ARRAYSIZE(m_expectedOutcome));
            ImGui::PopID();
            ImGui::Spacing();
            ImGui::Text("Actual outcome");
            ImGui::SameLine();
            HelpMarker("What actually happened.");
            ImGui::PushID(imguiID++);
            ImGui::InputText("", m_actualOutcome, IM_ARRAYSIZE(m_actualOutcome));
            ImGui::PopID();
            ImGui::Spacing();
            ImGui::Text("Latest game logs will be automatically supplied with this BugReport");
            ImGui::Spacing();
            if (ImGui::Button("Submit")) 
            {
                m_shouldCloseWindow = true;
            }
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
	}

    bool ReporterGUI::CheckIsEmptyReport()
    {
        if (m_summary[0] == *"" && m_reproductionSteps[0] == *"" && m_expectedOutcome[0] == *"" && m_actualOutcome[0] == *"") {
            return true;
        }
        return false;
    }

    void ReporterGUI::ProcessReport()
    {
        JsonParser jsonParser;
        jsonParser.SetBugReportData(m_summary, m_reproductionSteps, m_expectedOutcome, m_actualOutcome);
        jsonParser.WriteDataToFile();
    }

    bool ReporterGUI::ShouldCloseWindow()
    {
        return m_shouldCloseWindow;
    }

    // Helper to display a little (?) mark which shows a tooltip when hovered.
    void ReporterGUI::HelpMarker(const char* desc)
    {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    // Style from https://github.com/ocornut/imgui/issues/707
    void ReporterGUI::SetupImGuiStyle()
    {
        m_style = &ImGui::GetStyle();
        ImVec4* colors = m_style->Colors;

        colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
        colors[ImGuiCol_Border] = ImVec4(0.266f, 0.266f, 0.266f, 1.000f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_CheckMark] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_Button] = ImVec4(1.000f, 1.000f, 1.000f, 0.000f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
        colors[ImGuiCol_ButtonActive] = ImVec4(1.000f, 1.000f, 1.000f, 0.391f);
        colors[ImGuiCol_Header] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
        colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_Tab] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
        colors[ImGuiCol_TabActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
        colors[ImGuiCol_DockingPreview] = ImVec4(1.000f, 0.391f, 0.000f, 0.781f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_NavHighlight] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);

        m_style->ChildRounding = 4.0f;
        m_style->FrameBorderSize = 1.0f;
        m_style->FrameRounding = 2.0f;
        m_style->GrabMinSize = 7.0f;
        m_style->PopupRounding = 2.0f;
        m_style->ScrollbarRounding = 12.0f;
        m_style->ScrollbarSize = 13.0f;
        m_style->TabBorderSize = 1.0f;
        m_style->TabRounding = 0.0f;
        m_style->WindowRounding = 4.0f;
    }
} // namespace BugReporter
