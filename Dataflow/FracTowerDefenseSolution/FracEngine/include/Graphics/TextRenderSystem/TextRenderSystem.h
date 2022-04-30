#pragma once
#include "Core/ISystem.h"
#include "TextShader.h"

#include "Graphics/TextRenderSystem/Font.h"
#include "Graphics/Components/TextComponent.h"

#include <entt/entt.hpp>

namespace Frac
{
	class FileIO;
	class EngineCore;
	struct TextMesh;
	struct TextComponent;

	class TextRenderSystem : public ISystem {
	public:
		TextRenderSystem(EngineCore* a_engineCore, entt::registry& a_registry);
		~TextRenderSystem() = default;

		void Update(float deltatime) override;

		void SetTextMeshToEntityHandle(Frac::TextComponent& a_textComp);

		void SetViewProjection(glm::mat4 a_vp);

		void InitializePostProcessingPass();
		void CreateMSAAResources();
		void CreateFramebuffer();
		void ResizeViewport(unsigned newWidth, unsigned newHeight);

		void Initialize(unsigned in_windowWidth, unsigned in_windowHeight);

		void FaceActiveCamera(entt::entity activeCamEnt, TOR::Transform& transform, entt::entity entity);
	private:
		void DrawTextBundle(int index, unsigned textureID);
		void BufferTextMesh(Frac::TextComponent& text, const glm::mat4x4 worldTransform);

		void ConfigFontAtlases();

		Frac::Entity* GetActiveCameraEnt();

		// https://stackoverflow.com/questions/18172388/glm-quaternion-lookat-function
		glm::quat SafeQuatLookAt(
			glm::vec3 const& lookFrom,
			glm::vec3 const& lookTo,
			glm::vec3 const& up,
			glm::vec3 const& alternativeUp);

		EngineCore* m_engineCore;
		entt::registry& m_registry;

		unsigned int m_textRenderRBO = 0;
		unsigned int m_textRenderFBO = 0;
		unsigned int m_textRenderColorbuffer = 0;
		unsigned int m_textRenderBlitFBO = 0;
		unsigned int m_textRenderBlitColorbuffer = 0;

		unsigned int m_quadVAO = 0;
		unsigned int m_quadVBO = 0;
		unsigned int m_quadEBO = 0;

		std::array<unsigned, 2 * (static_cast<int>(TextComponent::FontFamily::Count))> m_textVAOs = { 0,0,0,0 };
		std::array<unsigned, 2 * (static_cast<int>(TextComponent::FontFamily::Count))> m_textVBOs = { 0,0,0,0 };
		std::array<int, 2 * (static_cast<int>(TextComponent::FontFamily::Count))> m_textBufferedVertices = { 0,0,0,0 };
		std::array<bool, 2 * (static_cast<int>(TextComponent::FontFamily::Count))> m_vboResets = { true,true,true,true };

		std::shared_ptr<Frac::TextShader> m_textShader;
		std::shared_ptr<Frac::TextShader> m_blitShader;
		std::unordered_map<Frac::TextComponent::FontFamily, std::unique_ptr<Frac::FontAtlas>> m_fontAtlases;
		glm::mat4 m_viewProjection;

		int m_maxSampleCount = 4;
		unsigned m_viewportWidth = 1;
		unsigned m_viewportHeight = 1;
	};
} // namespace Frac