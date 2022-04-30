#pragma once
#include "Platform/IDevice.h"
#include <glm\vec3.hpp>
#include "Graphics/RenderSystem.h"

struct GLFWwindow;
struct GLFWvidmode;

namespace Frac
{
	class TextRenderSystem;

	class PCDevice : public IDevice
	{
	public:
		PCDevice(Renderer& renderSystem, TextRenderSystem& TextRenderSystem);
		~PCDevice();

		void Initialize(unsigned width, unsigned height) override;

		GLFWwindow* GetWindow() const;

		void SwapBuffers() override;
		void PollEvents() override;
		bool ShouldClose() override;

		unsigned GetWindowWidth() const override;
		unsigned GetWindowHeight() const override;

		unsigned GetVideoResolutionWidth() const;
		unsigned GetVideoResolutionHeight() const;

		static void Framebuffer_size_callback(GLFWwindow* window, int width, int height);

	private:
		unsigned m_width;
		unsigned m_height;

		GLFWwindow* m_window;
		const GLFWvidmode* m_videoMode;
		Renderer& m_renderSystem;
		TextRenderSystem& m_textRenderSystem;
		glm::vec3 m_backgroundColor;
	};
} // namespace Frac