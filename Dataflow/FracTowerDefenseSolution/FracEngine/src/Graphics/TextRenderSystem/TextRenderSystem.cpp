#include "pch.h"

#include "Graphics/TextRenderSystem/TextRenderSystem.h"

#include "CoreRenderAPI/Components/Transform.h"
#include "Graphics/TextRenderSystem/TextMesh.h"

#include "Core/EngineCore.h"

#include "Graphics/OpenGL.h"

namespace Frac
{
	const std::string c_TextVertShader{ R"(#version 430 core
		layout(location = 0) in vec4 pos;
		layout(location = 1) in vec2 tex;
		layout(location = 2) in vec3 color;
		out vec2 TexCoords;
		out vec3 Colors;
		void main()
		{
			gl_Position = pos;
			TexCoords = tex;
			Colors = color;
		}  )"
	};

	const std::string c_TextFragShader{ R"(#version 430 core

		#define CUTOFF 0.2

		in vec2 TexCoords;
		in vec3 Colors;
		out vec4 color;

		uniform sampler2D u_textTexture;

		void main()
		{
			float alpha = texture(u_textTexture, TexCoords).r;
			alpha = (alpha - CUTOFF) / max(fwidth(alpha), 0.0001) + 0.5;

		    color = vec4(Colors, alpha);
		}
		)" };

	const std::string c_textVertShaderQuad{ R"(#version 430 core
		layout (location = 0) in vec3 a_position;
		layout (location = 1) in vec2 a_texCoords;

		out VS_OUT
		{
			vec2 TexCoords;
		} OUT;

		void main()
		{
			OUT.TexCoords = a_texCoords;
			gl_Position = vec4(a_position, 1.0);
		}	)" };

	const std::string c_textFragShaderQuad{ R"(#version 430 core
		out vec4 FragColor;

		in VS_OUT
		{
			vec2 TexCoords;
		} IN;

		uniform sampler2D u_UIBuffer;

		void main()
		{
			vec4 rgb = texture(u_UIBuffer, IN.TexCoords);
			if(rgb.a < 0.6) discard;
			FragColor = vec4(rgb);
		})" };

	TextRenderSystem::TextRenderSystem(Frac::EngineCore* a_engineCore, entt::registry& a_registry) : ISystem(eStage::OnStore),
		m_engineCore(a_engineCore)
		, m_registry(a_registry)
		, m_textShader(std::make_shared<Frac::TextShader>())
		, m_blitShader(std::make_shared<Frac::TextShader>())
		, m_viewProjection(glm::mat4(0.f))
	{
		m_textBufferedVertices = { 0 };
	}

	void TextRenderSystem::Update(float deltatime)
	{
		auto& fracReg = Frac::EngineCore::GetInstance().GetFracRegistry();

		Frac::Entity* activeCameraEnt = GetActiveCameraEnt();

		auto textView = m_registry.view<TOR::Transform, Frac::TextComponent>();

		glBindFramebuffer(GL_FRAMEBUFFER, m_textRenderFBO);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0, 0, 0, 1);

		textView.each([&textView, &fracReg, &activeCameraEnt, this](const auto entity, TOR::Transform& transformComponent, Frac::TextComponent& textComponent)
			{
				if (!textComponent.isActive)
				{
					return;
				}

				//facing active camera to a certain location might not be relevant here
				if (textComponent.faceActiveCamera && activeCameraEnt != nullptr)
				{
					FaceActiveCamera(activeCameraEnt->GetHandle(), transformComponent, entity);
				}

				if (textComponent.overrideOrientation)
				{
					Frac::Entity& p = fracReg.TranslateENTTEntity(entity).GetParent();
					if (fracReg.HasComponent<TOR::Transform>(p))
					{
						TOR::Transform& pTransform = Frac::EngineCore::GetInstance().GetFracRegistry().GetComponent<TOR::Transform>(p);
						transformComponent.Orientation = glm::inverse(pTransform.Orientation) * glm::quat(transformComponent.EulerOrientation);
					}
				}

				if (textComponent.text.empty())
				{
					return;
				}

				if (textComponent.dirtyText != textComponent.text || textComponent.mesh.m_verts.size() == 0 || textComponent.color != textComponent.mesh.m_verts[0].TextColor)
				{
					SetTextMeshToEntityHandle(textComponent);
				}

				BufferTextMesh(textComponent, m_viewProjection * transformComponent.ModelTransformData);
			});
		m_textShader->Use();

		m_textShader->setInt("u_textTexture", 0);
		glDisable(GL_CULL_FACE);
		glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		DrawTextBundle(static_cast<int>(TextComponent::FontFamily::Regular), static_cast<TextTexture>(*m_fontAtlases[TextComponent::FontFamily::Regular]).GetID());
		DrawTextBundle(static_cast<int>(TextComponent::FontFamily::Icons), static_cast<TextTexture>(*m_fontAtlases[TextComponent::FontFamily::Icons]).GetID());

		glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		DrawTextBundle(2 + static_cast<int>(TextComponent::FontFamily::Regular), static_cast<TextTexture>(*m_fontAtlases[TextComponent::FontFamily::Regular]).GetID());
		DrawTextBundle(2 + static_cast<int>(TextComponent::FontFamily::Icons), static_cast<TextTexture>(*m_fontAtlases[TextComponent::FontFamily::Icons]).GetID());
		glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);

#if defined(CONFIG_DEBUG) || defined (CONFIG_DEVELOP)
		m_textShader->Unuse();
#endif

		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_textRenderFBO);
		glReadBuffer(GL_COLOR_ATTACHMENT0);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_textRenderBlitFBO);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, m_viewportWidth, m_viewportHeight, 0, 0, m_viewportWidth, m_viewportHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glClear(GL_DEPTH_BUFFER_BIT);

		// use simple quad shader
		m_blitShader->Use();
		// bind texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textRenderBlitColorbuffer);
		m_blitShader->setInt("u_UIBuffer", 0);
		glBindVertexArray(m_quadVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		m_blitShader->Unuse();
		glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);

		int count = (static_cast<int>(TextComponent::FontFamily::Count)) * 2;
		for (int i = 0; i < count; ++i)
		{
			m_vboResets[i] = true;
			m_textBufferedVertices[i] = 0;
		}
	}

	void TextRenderSystem::DrawTextBundle(int index, unsigned textureID)
	{
		if (m_textBufferedVertices[index] != 0)
		{
			glBindVertexArray(m_textVAOs[index]);
			glActiveTexture(GL_TEXTURE0); // activate proper texture unit before binding
			glBindTexture(GL_TEXTURE_2D, textureID);

			glBindBuffer(GL_ARRAY_BUFFER, m_textVBOs[index]);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex) + sizeof(float), (void*)0);
			// vertex texture coords
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex) + sizeof(float), reinterpret_cast<void*>(offsetof(Vertex, Texture) + sizeof(float)));
			// vertex color coords
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex) + sizeof(float), reinterpret_cast<void*>(offsetof(Vertex, TextColor) + sizeof(float)));

			glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(m_textBufferedVertices[index]));
		}
	}

	void TextRenderSystem::SetTextMeshToEntityHandle(Frac::TextComponent& a_textComp)
	{
		a_textComp.dirtyText = a_textComp.text;

		auto glyphs = m_fontAtlases[a_textComp.fontFamily]->GetGlyphs(a_textComp.text);
		const auto amountOfGlyphs = glyphs.size();
		std::vector<Frac::Vertex> vertices;

		float avgHeight = 0.0f;
		float width = 0.0f;
		for (size_t i = 0; i < amountOfGlyphs; ++i)
		{
			float glyphH = (-glyphs[i].Quad.y0);
			avgHeight += (glyphH - avgHeight) / (i + 1);
			width += glyphs[i].Advance + glyphs[i].Kerning;
		}

		float begin = std::numeric_limits<float>::max();
		switch (a_textComp.alignment)
		{
		case TextComponent::Alignment::Left:
			begin = 0.0f;
			break;
		case TextComponent::Alignment::Center:
			begin = (width + tan(a_textComp.angle) * avgHeight) * -0.5f;
			break;
		case TextComponent::Alignment::Right:
			begin = -(width + tan(a_textComp.angle) * avgHeight);
			break;
		}

		// map glyphs to vertices
		for (size_t i = 0; i < amountOfGlyphs; ++i)
		{
			const float glyphW = (glyphs[i].Quad.x1 - glyphs[i].Quad.x0);
			const float glyphH = (glyphs[i].Quad.y1 - glyphs[i].Quad.y0);
			unsigned short base = static_cast<unsigned short>(vertices.size());
			float slant = tan(a_textComp.angle) * glyphH;
			float underslant = tan(a_textComp.angle) * (glyphs[i].Quad.y1);

			// A sign, not a letter (private use section 0f unicode)
			if (glyphs[i].Codepoint > 0xE000 && glyphs[i].Codepoint < 0xF8FF)
			{
				slant = 0.0f;
				underslant = 0.0f;
			}

			// Create vertices
			Vertex verts[4] =
			{
				// Bottom
				{ glm::vec3(begin + glyphs[i].BearingX - underslant,(avgHeight / 2) + glyphs[i].Quad.y1,0.f),	 glm::vec2(glyphs[i].Quad.s0, glyphs[i].Quad.t1) }, //bottom left
				{ glm::vec3(begin + glyphs[i].BearingX + glyphW - underslant,(avgHeight / 2) + glyphs[i].Quad.y1,0.f), glm::vec2(glyphs[i].Quad.s1, glyphs[i].Quad.t1) }, //bottom right
				{ glm::vec3(begin + glyphs[i].BearingX + glyphW + slant - underslant,(avgHeight / 2) + glyphs[i].Quad.y0,0.f), glm::vec2(glyphs[i].Quad.s1, glyphs[i].Quad.t0) }, //top right
				{ glm::vec3(begin + glyphs[i].BearingX + slant - underslant,(avgHeight / 2) + glyphs[i].Quad.y0,0.f), glm::vec2(glyphs[i].Quad.s0, glyphs[i].Quad.t0) }  //top left
			};

			const glm::vec3 mutliplier = glm::vec3(-1.f, -1.f, 1.f);
			verts[0].Position *= mutliplier;
			verts[1].Position *= mutliplier;
			verts[2].Position *= mutliplier;
			verts[3].Position *= mutliplier;

			begin += ((glyphs[i].Advance + glyphs[i].Kerning) + a_textComp.customKerning * tan(a_textComp.angle));

			int count = 1;
			Frac::Vertex arrayOfVerts[6];
			//store vertices
			for (auto& v : verts)
			{
				float newFont = a_textComp.fontSize;
				if (a_textComp.fontFamily == TextComponent::FontFamily::Regular)
				{
					//newFont *= 0.5f;
				}
				// 4 1 2  4 2 3
				v.Position *= newFont;
				v.TextColor = a_textComp.color;

				if (count == 1)
				{
					arrayOfVerts[1] = v;
					count++;
				}
				else if (count == 2)
				{
					arrayOfVerts[2] = v;
					arrayOfVerts[4] = v;
					count++;
				}
				else if (count == 3)
				{
					arrayOfVerts[5] = v;
					count++;
				}
				else if (count == 4)
				{
					arrayOfVerts[0] = v;
					arrayOfVerts[3] = v;

					for (int i = 0; i < 6; ++i)
					{
						vertices.push_back(arrayOfVerts[i]);
						count = 0;
					}
				}
			}
		} // end for glyphs

		a_textComp.mesh.m_verts = vertices;
	}

	void TextRenderSystem::SetViewProjection(glm::mat4 a_vp)
	{
		m_viewProjection = a_vp;
	}

	void TextRenderSystem::InitializePostProcessingPass()
	{
		float vertices[] =
		{
			1.0f,  1.0f, 0.0f,		1.0f, 1.0f,
			1.0f, -1.0f, 0.0f,		1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,		0.0f, 1.0f
		};

		unsigned int indices[] =
		{
			3, 1, 0,
			3, 2, 1
		};

		// setup plane VAO
		glGenVertexArrays(1, &m_quadVAO);
		glBindVertexArray(m_quadVAO);

		//Setup the VBO.
		glGenBuffers(1, &m_quadVBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

		//Setup the EBO.
		glGenBuffers(1, &m_quadEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_quadEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		glBindVertexArray(0);
	}

	void TextRenderSystem::CreateMSAAResources()
	{
		if (m_textRenderBlitFBO != 0)
		{
			glDeleteFramebuffers(1, &m_textRenderBlitFBO);
			glDeleteTextures(1, &m_textRenderBlitColorbuffer);
		}
		// Create temporary FBO to blit HDR Image to.
		glGenFramebuffers(1, &m_textRenderBlitFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_textRenderBlitFBO);
		glGenTextures(1, &m_textRenderBlitColorbuffer);
		glBindTexture(GL_TEXTURE_2D, m_textRenderBlitColorbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_viewportWidth, m_viewportHeight, 0, GL_RGBA, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textRenderBlitColorbuffer, 0);

		// also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			FRAC_ASSERT(false, "The frame buffer for the post processing failed to instantiate.");
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void TextRenderSystem::CreateFramebuffer()
	{
		if (m_textRenderFBO != 0)
		{
			glDeleteFramebuffers(1, &m_textRenderFBO);
			glDeleteRenderbuffers(1, &m_textRenderRBO);
			glDeleteTextures(1, &m_textRenderColorbuffer);
		}

		// set up floating point framebuffer to render scene to
		glGenFramebuffers(1, &m_textRenderFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_textRenderFBO);

		//Create the color output targets.
		glGenTextures(1, &m_textRenderColorbuffer);

		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_textRenderColorbuffer);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_maxSampleCount, GL_RGBA16F, m_viewportWidth, m_viewportHeight, GL_TRUE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_textRenderColorbuffer, 0);

		//Create render buffer.
		glGenRenderbuffers(1, &m_textRenderRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, m_textRenderRBO);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_maxSampleCount, GL_DEPTH_COMPONENT, m_viewportWidth, m_viewportHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_textRenderRBO);

		//Set the render ouput buffers.
		unsigned int drawBuffer = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(1, &drawBuffer);

		//After completing the process check the status of the fbo.
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			FRAC_ASSERT(false, "The frame buffer for the text render system failed to instantiate.");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void TextRenderSystem::ResizeViewport(unsigned newWidth, unsigned newHeight)
	{
		m_viewportWidth = newWidth;
		m_viewportHeight = newHeight;
		CreateFramebuffer();
		CreateMSAAResources();
	}

	void TextRenderSystem::Initialize(unsigned in_windowWidth, unsigned in_windowHeight)
	{
		glGetIntegerv(GL_MAX_SAMPLES, &m_maxSampleCount);
		if (m_maxSampleCount > 16) m_maxSampleCount = 16;

		ResizeViewport(in_windowWidth, in_windowHeight);
		m_textShader->LoadFromString(c_TextVertShader, c_TextFragShader);
		m_blitShader->LoadFromString(c_textVertShaderQuad, c_textFragShaderQuad);
		InitializePostProcessingPass();
		ConfigFontAtlases();

		int count = static_cast<int>(TextComponent::FontFamily::Count);
		for (int i = 0; i < count * 2; ++i)
		{
			glGenVertexArrays(1, &m_textVAOs[i]);
			glBindVertexArray(m_textVAOs[i]);
			glGenBuffers(1, &m_textVBOs[i]);
			glBindVertexArray(0);
		}
	}

	void TextRenderSystem::BufferTextMesh(Frac::TextComponent& text, const glm::mat4x4 worldTransform)
	{
		int shouldBlend = 0;
		if (!text.noBlend) shouldBlend = 1;
		unsigned glBufferToSendDataTo = -1;
		bool shouldResetBuffer = false;
		int index = -1;
		switch (text.fontFamily)
		{
		case TextComponent::FontFamily::Icons:
		{
			index = 2 * shouldBlend + static_cast<int>(TextComponent::FontFamily::Icons);
			break;
		}
		case TextComponent::FontFamily::Regular:
		{
			index = 2 * shouldBlend + static_cast<int>(TextComponent::FontFamily::Regular);
			break;
		}
		}
		Frac::TextMesh& mesh = text.mesh;
		glBindVertexArray(m_textVAOs[index]);
		shouldResetBuffer = m_vboResets[index];
		m_vboResets[index] = false;
		glBindBuffer(GL_ARRAY_BUFFER, m_textVBOs[index]);

		struct worldSpaceVertex {
			glm::vec4 pos;
			glm::vec2 tex;
			glm::vec3 color;
		};
		std::vector<worldSpaceVertex> worldSpaceVertices;

		for (int i = 0; i < mesh.m_verts.size(); ++i)
		{
			glm::vec4 newPosition = worldTransform * glm::vec4(mesh.m_verts[i].Position, 1);
			worldSpaceVertex newVertex;
			newVertex.pos = newPosition;
			newVertex.color = mesh.m_verts[i].TextColor;
			newVertex.tex = mesh.m_verts[i].Texture;

			worldSpaceVertices.push_back(newVertex);
		}

		const int BufferVertexSize = sizeof(worldSpaceVertex) * 4000;
		if (shouldResetBuffer)
		{
			glBufferData(GL_ARRAY_BUFFER, BufferVertexSize, NULL, GL_DYNAMIC_DRAW);
		}
		int offset = m_textBufferedVertices[index] * sizeof(worldSpaceVertex);
		glBufferSubData(GL_ARRAY_BUFFER, offset, worldSpaceVertices.size() * sizeof(worldSpaceVertex), &worldSpaceVertices[0]);

		m_textBufferedVertices[index] += worldSpaceVertices.size();
	}

	void TextRenderSystem::ConfigFontAtlases()
	{
		m_fontAtlases[Frac::TextComponent::FontFamily::Regular] = std::make_unique<Frac::FontAtlas>("RegularFont", 4096, 4096);
		m_fontAtlases[Frac::TextComponent::FontFamily::Icons] = std::make_unique<Frac::FontAtlas>("IconFont", 4096, 4096);

		m_fontAtlases[Frac::TextComponent::FontFamily::Regular]
			->AddRange(FileIO::GetPathFromWildcard("[Fonts]Square.ttf"), 64.f, 0, 255);
		m_fontAtlases[Frac::TextComponent::FontFamily::Icons]
			->AddRange(FileIO::GetPathFromWildcard("[Fonts]ButtonIcons.ttf"), 512.f, 0, 255);

		m_fontAtlases[Frac::TextComponent::FontFamily::Regular]->Apply();
		m_fontAtlases[Frac::TextComponent::FontFamily::Icons]->Apply();
	}

	Frac::Entity* TextRenderSystem::GetActiveCameraEnt()
	{
		auto cameraView = m_registry.view<TOR::Transform, TOR::Camera>();

		for (auto entity : cameraView)
		{
			auto& cameraComponent = cameraView.get<TOR::Camera>(entity);

			if (cameraComponent.IsActive) {
				return &Frac::EngineCore::GetInstance().GetFracRegistry().TranslateENTTEntity(entity);
			}
		}

		return nullptr;
	}

	void TextRenderSystem::FaceActiveCamera(entt::entity activeCamEnt, TOR::Transform& transform, entt::entity entity)
	{
		if (activeCamEnt != entt::null)
		{
			auto& fracReg = Frac::EngineCore::GetInstance().GetFracRegistry();

			auto& camTrans = Frac::EngineCore::GetInstance().GetEnttRegistry().get<TOR::Transform>(activeCamEnt);

			Frac::Entity& e = fracReg.TranslateENTTEntity(entity);
			Frac::Entity& p = e.GetParent();

			if (fracReg.HasComponent<TOR::Transform>(p))
			{
				const TOR::Transform& pTransform = fracReg.GetComponent<TOR::Transform>(p);

				transform.Orientation = glm::inverse(pTransform.Orientation) * SafeQuatLookAt(
					transform.ModelTransformData[3], // pos calculated based on potential parents
					camTrans.Position,
					glm::vec3(0, 1, 0),
					glm::vec3(0, -1, 0));
			}
			else
			{
				transform.Orientation = SafeQuatLookAt(
					transform.ModelTransformData[3], // pos calculated based on potential parents
					camTrans.Position,
					glm::vec3(0, 1, 0),
					glm::vec3(0, -1, 0));
			}
		}
		else {
			LOGERROR("There is no active camera!");
		}
	}

	glm::quat TextRenderSystem::SafeQuatLookAt(glm::vec3 const& lookFrom, glm::vec3 const& lookTo, glm::vec3 const& up,
		glm::vec3 const& alternativeUp)
	{
		glm::vec3  direction = lookTo - lookFrom;
		float      directionLength = glm::length(direction);

		// Check if the direction is valid; Also deals with NaN
		if (!(directionLength > 0.0001))
			return glm::quat(1, 0, 0, 0); // Just return identity

		// Normalize direction
		direction /= directionLength;

		// Is the normal up (nearly) parallel to direction?
		if (glm::abs(glm::dot(direction, up)) > .9999f) {
			// Use alternative up
			return glm::quatLookAt(direction, alternativeUp);
		}
		else {
			return glm::quatLookAt(direction, up);
		}
	}
} // namespace Frac