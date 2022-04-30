#include "OpenGLRenderer/pch.h"
#include "OpenGLRenderer/OpenGL_Renderer.h"

#include "Glm/gtx/io.hpp"
#include "OpenGLRenderer/InternalRenderingStructures/OpenGLSampler.h"

namespace TOR
{
	OpenGL_Renderer::OpenGL_Renderer() :
		m_spotLightCount(0),
		m_pointLightCount(0),
		m_SHADOW_WIDTH(2048 * 2),
		m_SHADOW_HEIGHT(2048 * 2)
	{}

	void OpenGL_Renderer::Init(const char* assetDirectory, unsigned viewportWidth, unsigned viewportHeight)
	{
		m_viewportWidth = viewportWidth;
		m_viewportHeight = viewportHeight;

#if defined (PLATFORM_WINDOWS)
		//Initialize glad again, since
		if (gladLoadGL() != GL_TRUE)
		{
			printf("Failed to instantiate openGL in OpenGLRenderer\n");
		}
#elif defined (PLATFORM_SWITCH)
		nngllInitializeGl();
#endif
#if CONFIG_DEBUG || CONFIG_DEVELOP
		InitDebugGlCallbacks();
#endif

		//Set the render default render settings.
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		glDepthFunc(GL_LESS);
		glEnable(GL_DEPTH_TEST);

		std::string shaderDirectory = std::string(std::string{ assetDirectory } + "Shaders/GLSL/");

		m_shaderMap.insert(std::make_pair(static_cast<int>(EShaderType::Diffuse), std::make_shared<OpenGLShader>(shaderDirectory + "VS/diffuse.vert", shaderDirectory + "PS/diffuse.frag")));
		m_shaderMap.insert(std::make_pair(static_cast<int>(EShaderType::DiffusePBR), std::make_shared<OpenGLShader>(shaderDirectory + "VS/diffusePBR.vert", shaderDirectory + "PS/diffusePBR.frag")));
		m_shaderMap.insert(std::make_pair(static_cast<int>(EShaderType::DiffuseWithNormalMapping), std::make_shared<OpenGLShader>(shaderDirectory + "VS/diffuseWithNormalMap.vert", shaderDirectory + "PS/diffuseWithNormalMap.frag")));
		m_shaderMap.insert(std::make_pair(static_cast<int>(EShaderType::ShadowMapping), std::make_shared<OpenGLShader>(shaderDirectory + "VS/shadowMapPass.vert", shaderDirectory + "PS/shadowMapPass.frag")));
		m_shaderMap.insert(std::make_pair(static_cast<int>(EShaderType::gaussianBlur), std::make_shared<OpenGLShader>(shaderDirectory + "CS/gaussianBlur.comp")));
		m_shaderMap.insert(std::make_pair(static_cast<int>(EShaderType::PostProcessFinal), std::make_shared<OpenGLShader>(shaderDirectory + "VS/postProcessFinal.vert", shaderDirectory + "PS/postProcessFinal.frag")));
		m_shaderMap.insert(std::make_pair(static_cast<int>(EShaderType::Particle), std::make_shared<OpenGLShader>(shaderDirectory + "VS/diffuseParticle.vert", shaderDirectory + "PS/diffuseParticle.frag")));

		InitializeShadowRenderPass();
		CreateBloomPassResources();
		InitializePostProcessingPass();
		CreateMSAAResources();
	}

	void OpenGL_Renderer::InitDebugGlCallbacks()
	{
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGL_Renderer::DebugCallbackFunc, nullptr);

		//glDebugMessageControl(GL_DEBUG_SOURCE_API,
		//	GL_DEBUG_TYPE_ERROR,
		//	GL_DEBUG_SEVERITY_HIGH,
		//	0, nullptr, GL_TRUE);
		//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	}

	void OpenGL_Renderer::Setting(const char* name, float value)
	{
	}

	void OpenGL_Renderer::SetCameraProperties(unsigned meshId, const glm::mat4& worldTransform, const TOR::Camera& camera)
	{
		auto shader = m_meshMap[meshId].GetShader();
		glm::mat4x4 projectionViewMatrix = camera.ProjectionMat * glm::inverse(worldTransform);

		shader->GetParameter("u_vpMat")->SetValue(projectionViewMatrix);
		shader->GetParameter("u_cameraPos")->SetValue(glm::vec3(
			worldTransform[3].x,
			worldTransform[3].y,
			worldTransform[3].z
		));
	}

	void OpenGL_Renderer::ActivateShader(unsigned meshId)
	{
		auto shader = m_meshMap[meshId].GetShader();
		shader->Activate();
	}

	void OpenGL_Renderer::SetMaterialProperties(unsigned meshId, Material& material)
	{
		auto shader = m_meshMap[meshId].GetShader();
		ActivateShader(meshId);
		int textureSlot = 0;

		//Albedo map.
		if (material.TexturesMaps[TOR::ETextureType::ALBEDO] != nullptr)
		{
			Texture& texture = *material.TexturesMaps[TOR::ETextureType::ALBEDO];
			m_textureMap[texture.Id].Bind(textureSlot);
			shader->GetParameter("u_material.AlbedoTexture")->SetValue(m_textureMap[texture.Id]);
			shader->GetParameter("u_hasAlbedoTexture")->SetValue(true);

			material.TexCoords[static_cast<int>(ETextureType::ALBEDO)]; // #TODO: see if we need this and how we are going to use it with other maps.
			textureSlot++;
		}
		else
		{
			shader->GetParameter("u_hasAlbedoTexture")->SetValue(false);
			shader->GetParameter("u_material.AlbedoFactor")->SetValue(material.AlbedoFactor);
		}

		//Normal map.
		if (material.TexturesMaps[TOR::ETextureType::NORMAL] != nullptr)
		{
			Texture& texture = *material.TexturesMaps[TOR::ETextureType::NORMAL];
			textureSlot = 1;
			m_textureMap[texture.Id].Bind(textureSlot);
			shader->GetParameter("u_material.NormalTexture")->SetValue(m_textureMap[texture.Id]);
		}

		//Metal roughness map.
		if (material.TexturesMaps[TOR::ETextureType::METALROUGHNESS] != nullptr)
		{
			Texture& texture = *material.TexturesMaps[TOR::ETextureType::METALROUGHNESS];
			textureSlot = 2;
			m_textureMap[texture.Id].Bind(textureSlot);
			shader->GetParameter("u_material.AOMROTexture")->SetValue(m_textureMap[texture.Id]);
			shader->GetParameter("u_hasRoughnessMetalnessTexture")->SetValue(true);
		}
		else {
			shader->GetParameter("u_hasRoughnessMetalnessTexture")->SetValue(false);
			shader->GetParameter("u_material.Metalness")->SetValue(material.MetallicFactor);
			shader->GetParameter("u_material.Roughness")->SetValue(material.RoughnessFactor);
		}

		//Emission.
		if (material.TexturesMaps[TOR::ETextureType::EMISSION] != nullptr)
		{
			Texture& texture = *material.TexturesMaps[TOR::ETextureType::EMISSION];
			textureSlot = 3;
			m_textureMap[texture.Id].Bind(textureSlot);
			shader->GetParameter("u_material.EmissionFactor")->SetValue(material.EmissionFactor);
			shader->GetParameter("u_material.EmissionTexture")->SetValue(m_textureMap[texture.Id]);
			shader->GetParameter("u_hasEmissiveTexture")->SetValue(true);
		}
		else
		{
			shader->GetParameter("u_hasEmissiveTexture")->SetValue(false);
		}

		//Set Fog Values
		shader->GetParameter("u_fogColor")->SetValue(m_renderSettings.FogProps.Color);
		shader->GetParameter("u_fogDensity")->SetValue(m_renderSettings.FogProps.Density);

		//Set the ambient occlusion.
		shader->GetParameter("u_useBakedAmbientOcclusion")->SetValue(material.HasOcclusionMap);
		shader->GetParameter("u_flipNormals")->SetValue(material.FlipNormals);
		shader->GetParameter("u_hasVertexColors")->SetValue(material.HasVertexColors);

		//Shadowmap.
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, m_depthTextureId);
		shader->GetParameter("u_material.ShadowTexture")->SetValue(4);
		shader->GetParameter("u_lightSpaceMatrix")->SetValue(m_lightViewMatrix);
	}

	void OpenGL_Renderer::SetDirectionalLightProperties(unsigned meshId, const glm::mat4& lightWorldMatrix, const DirectionalLight& light)
	{
		auto shader = m_meshMap[meshId].GetShader();
		shader->GetParameter("u_directionalLightDirections[" + std::to_string(m_directionalLightCount) + "].Direction")->SetValue(glm::vec3(lightWorldMatrix[2]));
		shader->GetParameter("u_directionalLights[" + std::to_string(m_directionalLightCount) + "].Color")->SetValue(light.Color);
		shader->GetParameter("u_directionalLights[" + std::to_string(m_directionalLightCount) + "].Intensity")->SetValue(light.Intensity);
		++m_directionalLightCount;

		shader->GetParameter("u_directionalLightCount")->SetValue(m_directionalLightCount);
	}

	void OpenGL_Renderer::SetPointLightProperties(unsigned meshId, const glm::mat4& lightWorldMatrix, const PointLight& light)
	{
		auto shader = m_meshMap[meshId].GetShader();

		shader->GetParameter("u_pointLightPositions[" + std::to_string(m_pointLightCount) + "].Position")->SetValue(glm::vec3(lightWorldMatrix[3]));
		shader->GetParameter("u_pointLights[" + std::to_string(m_pointLightCount) + "].Intensity")->SetValue(light.Intensity / 1000);
		shader->GetParameter("u_pointLights[" + std::to_string(m_pointLightCount) + "].Color")->SetValue(light.Color);
		shader->GetParameter("u_pointLights[" + std::to_string(m_pointLightCount) + "].Range")->SetValue(light.Radius);
		++m_pointLightCount;

		shader->GetParameter("u_pointLightCount")->SetValue(m_pointLightCount);
	}

	void OpenGL_Renderer::SetSpotLightProperties(unsigned meshId, const glm::mat4& lightWorldMatrix, const SpotLight& light)
	{
		++m_spotLightCount;
		printf("WARNING: Spotlights are not supported in the renderer yet!\n");
	}

	void OpenGL_Renderer::SetMeshProperties(unsigned meshId, const glm::mat4& modelTransform)
	{
		auto shader = m_meshMap[meshId].GetShader();
		shader->Activate();
		shader->GetParameter("u_modelMat")->SetValue(modelTransform);
		shader->GetParameter("u_normalMat")->SetValue(glm::transpose(glm::inverse(glm::mat3(modelTransform))));
	}

	void OpenGL_Renderer::Render(unsigned meshId)
	{
		m_meshMap[meshId].Draw();
		m_pointLightCount = 0; //#TODO: Implement a setup renderpass method, that handles the counters for these.
		m_spotLightCount = 0;
		m_directionalLightCount = 0;
	}

	void OpenGL_Renderer::RenderInstanced(unsigned meshId, unsigned instanceCount)
	{
		m_meshMap[meshId].DrawInstanced(instanceCount);
		m_pointLightCount = 0; //#TODO: Implement a setup renderpass method, that handles the counters for these.
		m_spotLightCount = 0;
		m_directionalLightCount = 0;
	}

	void OpenGL_Renderer::Present()
	{
		glViewport(0, 0, m_viewportWidth, m_viewportHeight);

		// Blit the HDR map of the scene into a new FBO that isn't multisampled, so the shader can actually process it correctly.
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_hdrFBO);
		glReadBuffer(GL_COLOR_ATTACHMENT0);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_hdrBlitFBO);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, m_viewportWidth, m_viewportHeight, 0, 0, m_viewportWidth, m_viewportHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		//Clear the screen and activate the final shader.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_shaderMap[static_cast<int>(EShaderType::PostProcessFinal)]->Activate();

		//Binding the PBR scene pass output.
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_blitColorBuffer);
		m_shaderMap[static_cast<int>(EShaderType::PostProcessFinal)]->GetParameter("u_pbrPass")->SetValue(static_cast<int>(0));

		//Binding the bloompass output.
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_pingpongColorbuffers[0]); //bind the vertical buffer, since this one got blurred last time.
		m_shaderMap[static_cast<int>(EShaderType::PostProcessFinal)]->GetParameter("u_bloomPass")->SetValue(static_cast<int>(1));

		//Blur settings.
		m_shaderMap[static_cast<int>(EShaderType::PostProcessFinal)]->GetParameter("u_mipLevels")->SetValue(m_finalMipMapLevel);
		m_shaderMap[static_cast<int>(EShaderType::PostProcessFinal)]->GetParameter("u_startMipLevel")->SetValue(m_renderSettings.BloomProps.StartMipLevel);
		m_shaderMap[static_cast<int>(EShaderType::PostProcessFinal)]->GetParameter("u_bloomIntensity")->SetValue(m_renderSettings.BloomProps.Intensity);

		//Tonemap settings.
		m_shaderMap[static_cast<int>(EShaderType::PostProcessFinal)]->GetParameter("u_exposure")->SetValue(m_renderSettings.ToneMappingProps.Exposure);
		m_shaderMap[static_cast<int>(EShaderType::PostProcessFinal)]->GetParameter("u_gamma")->SetValue(m_renderSettings.ToneMappingProps.Gamma);


		glBindVertexArray(m_quadVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

#if defined(CONFIG_DEBUG) || defined(CONFIG_DEVELOP)
		m_shaderMap[static_cast<int>(EShaderType::PostProcessFinal)]->Deactivate();
#endif
	}

	void OpenGL_Renderer::CreateMesh(Mesh& meshStructToFillIn)
	{
		static int MeshIdCounter = 0;
		OpenGLMesh mesh;
		mesh.Initialize();
		m_meshMap.insert(std::make_pair(MeshIdCounter, mesh));
		meshStructToFillIn.Id = MeshIdCounter;
		++MeshIdCounter;
	}

	void OpenGL_Renderer::DestroyMesh(unsigned meshId)
	{
		TOR_ASSERT(m_meshMap.find(meshId) == m_meshMap.end());
		m_meshMap.erase(meshId);
	}

	void OpenGL_Renderer::BindMeshForRecording(unsigned meshId)
	{
		m_meshMap[meshId].Bind();
	}

	void OpenGL_Renderer::UnbindMesh(unsigned meshId)
	{
		m_meshMap[meshId].Unbind();
	}

	void OpenGL_Renderer::BufferAttributeData(unsigned meshId, EVertexAttributes attrType, uint32_t byteLength, uint32_t ByteStride, bool normalized, int attrElementCount, const void* bufferStart, int overwriteVertexAttribArrayPosition /*= -1*/, EValueTypes DataType /*= EValueTypes::FLOAT*/, EDrawTypes DrawMode /*= EDrawTypes::STATIC_DRAW*/)
	{
		unsigned long glDataType = ParseValueTypeToGL(DataType);
		unsigned long glDrawMode = ParseBufferTypeToGL(DrawMode);
		m_meshMap[meshId].BufferAttributeData(attrType, byteLength, ByteStride, normalized, attrElementCount, bufferStart, overwriteVertexAttribArrayPosition, glDataType, glDrawMode);
	}

	void OpenGL_Renderer::BufferInstanceAttributeData(unsigned meshId, uint32_t byteLength, const void* bufferStart, EDrawTypes DrawMode /*= EDrawTypes::STATIC_DRAW*/)
	{
		unsigned long glDrawMode = ParseBufferTypeToGL(DrawMode);
		m_meshMap[meshId].BufferInstanceAttributeData(EVertexAttributes::MODELTRANSFORM, byteLength, bufferStart, glDrawMode);
	}

	void OpenGL_Renderer::BufferInstanceNormalAttributeData(unsigned meshId, uint32_t byteLength, const void* bufferStart, EDrawTypes DrawMode /*= EDrawTypes::STATIC_DRAW*/)
	{
		unsigned long glDrawMode = ParseBufferTypeToGL(DrawMode);
		m_meshMap[meshId].BufferInstanceAttributeData(EVertexAttributes::NORMALMATRIX, byteLength, bufferStart, glDrawMode);
	}

	void OpenGL_Renderer::BufferAnyInstancedAttributeData(unsigned meshId, uint32_t byteLength, const void* bufferStart, int AttributeLocation, int elementCount, EDrawTypes DrawMode /*= EDrawTypes::STATIC_DRAW*/)
	{
		unsigned long glDrawMode = ParseBufferTypeToGL(DrawMode);
		m_meshMap[meshId].BufferInstanceWildcardAttributeData(AttributeLocation, byteLength, bufferStart, elementCount, glDrawMode);
	}

	void OpenGL_Renderer::BufferElementData(unsigned meshId, uint32_t byteLength, void* bufferStart, unsigned int numberOfElements, EDrawTypes DrawMode /*= EDrawTypes::STATIC_DRAW*/)
	{
		unsigned long glDrawMode = ParseBufferTypeToGL(DrawMode);
		m_meshMap[meshId].BufferElementData(byteLength, bufferStart, numberOfElements, glDrawMode);
	}

	void OpenGL_Renderer::CreateTexture(Texture& texture)
	{
		OpenGLTexture glTexture;
		static int textureIdCounter = 0;
		m_textureMap.insert(std::make_pair(textureIdCounter, glTexture));
		texture.Id = textureIdCounter;
		++textureIdCounter;
	}

	void OpenGL_Renderer::LoadTexture(unsigned textureId, const Image& image, unsigned char* source, int sourceCountInElements, const Sampler& sampler, int format)
	{
		OpenGLSampler glSampler = ParseSamplerToGLSampler(sampler);
		m_textureMap[textureId].Initialize(image, glSampler, source, sourceCountInElements, format);
	}

	void OpenGL_Renderer::DestroyTexture(unsigned textureId)
	{
		TOR_ASSERT(m_textureMap.find(textureId) != m_textureMap.end());
		m_textureMap.erase(textureId);
	}

	void OpenGL_Renderer::BindTexture(unsigned textureId, unsigned int slot)
	{
		m_textureMap[textureId].Bind(slot);
	}

	void OpenGL_Renderer::UnbindTexture(unsigned textureId)
	{
		m_textureMap[textureId].Unbind();
	}

	void OpenGL_Renderer::AssignShaderToMesh(unsigned meshId, EShaderType type)
	{
		m_meshMap[meshId].SetShader(m_shaderMap[static_cast<int>(type)]);
	}

	void OpenGL_Renderer::ApplyBloomPass()
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_hdrFBO);
		glReadBuffer(GL_COLOR_ATTACHMENT1);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_pingpongFBO);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		glBlitFramebuffer(0, 0, m_viewportWidth, m_viewportHeight, 0, 0, m_viewportWidth, m_viewportHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_pingpongColorbuffers[2]);
		glGenerateMipmap(GL_TEXTURE_2D);

		int currentImageWidth = m_viewportWidth;
		int currentImageHeight = m_viewportHeight;

		//Loop over the requested mip levels which should have a blur implemented.
		for (int mipLevel = 0; mipLevel < m_maxMipMapLevel; ++mipLevel)
		{
			if(m_renderSettings.BloomProps.StartMipLevel > mipLevel)
			{
				currentImageWidth *= 0.5;
				currentImageHeight *= 0.5;
				continue;
			}

			m_shaderMap[static_cast<int>(EShaderType::gaussianBlur)]->GetParameter("u_imageDimensions")->SetValue(glm::vec2(currentImageWidth, currentImageHeight));
			m_shaderMap[static_cast<int>(EShaderType::gaussianBlur)]->GetParameter("u_mipLevel")->SetValue(mipLevel);

			//Calculate the dispatch groups for the compute shader, based on the current image width and height.
			GLuint num_groups_x = GLuint((currentImageWidth / m_renderSettings.ComputeProps.DispatchUnitSizeX) + 1);
			GLuint num_groups_y = GLuint((currentImageHeight / m_renderSettings.ComputeProps.DispatchUnitSizeY) + 1);

			for (unsigned int i = 0; i < 2; ++i)
			{
				glBindTexture(GL_TEXTURE_2D, m_pingpongState ? m_pingpongColorbuffers[2] : m_pingpongColorbuffers[1]);
				glBindImageTexture(0, m_pingpongColorbuffers[m_pingpongState], mipLevel, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);

				m_shaderMap[static_cast<int>(EShaderType::gaussianBlur)]->GetParameter("u_horizontal")->SetValue(m_pingpongState);

				//Check if the maximum amount of group invocations exceeds the current image resolution, if so ignore advanced dispatching and go through it 1 by 1.
				if(currentImageWidth * currentImageHeight > m_maxGroupInvocations)
				{
					glDispatchCompute(num_groups_x, num_groups_y, 1);
				}
				else
				{
					glDispatchCompute(1, 1, 1);
				}

				//Using a memory buffer so the bloom updates are completed before moving on to the rendering of it.
				glMemoryBarrier(
					GL_UNIFORM_BARRIER_BIT | 
					GL_TEXTURE_FETCH_BARRIER_BIT | 
					GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | 
					GL_FRAMEBUFFER_BARRIER_BIT | 
					GL_SHADER_STORAGE_BARRIER_BIT
				);

				m_pingpongState = !m_pingpongState;
			}

			//Each new mipmap cycle make sure to half the resolution.
			currentImageWidth *= 0.5f;
			currentImageHeight *= 0.5f;

			//Any smaller mipmaps that this can be ignored. (just an arbitrary number was used here).
			if (currentImageWidth < m_renderSettings.BloomProps.SmallestMipWidth)
			{
				m_finalMipMapLevel = mipLevel;
				break;
			}
		}
	}

	void OpenGL_Renderer::SetupShadowPass(const glm::mat4x4& shadowViewMatrix)
	{
		m_shaderMap[static_cast<int>(EShaderType::ShadowMapping)]->Activate();
		m_shaderMap[static_cast<int>(EShaderType::ShadowMapping)]->GetParameter("u_lightSpaceMatrix")->SetValue(shadowViewMatrix, false);

		m_lightViewMatrix = shadowViewMatrix;
		glViewport(0, 0, m_SHADOW_WIDTH, m_SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		//glCullFace(GL_FRONT);
	}

	void OpenGL_Renderer::SetMeshShadowProperties(unsigned meshId, const glm::mat4& modelTransform)
	{
		//m_shaderMap[static_cast<int>(EShaderType::ShadowMapping)]->GetParameter("u_modelMat")->SetValue(modelTransform);
	}

	void OpenGL_Renderer::EndShadowPass()
	{
		//glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, m_viewportWidth, m_viewportHeight);
		
#if CONFIG_DEBUG || CONFIG_DEVELOP
		m_shaderMap[static_cast<int>(EShaderType::ShadowMapping)]->Deactivate();
#endif
	}

	void OpenGL_Renderer::SetupScenePass()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);
		//glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaaFBO);
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_hdrFBO);
		//glBlitFramebuffer(0, 0, m_viewportWidth, m_viewportHeight, 0, 0, m_viewportWidth, m_viewportHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGL_Renderer::EndScenePass()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
#if CONFIG_DEBUG || CONFIG_DEVELOP
		m_shaderMap[static_cast<int>(EShaderType::DiffusePBR)]->Deactivate();
#endif
	}

	void OpenGL_Renderer::SetupPostProcessingPass()
	{
		m_shaderMap[static_cast<int>(EShaderType::gaussianBlur)]->Activate();
	}

	void OpenGL_Renderer::EndPostProcessingPass()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

#if defined(CONFIG_DEBUG) || defined(CONFIG_DEVELOP)
		m_shaderMap[static_cast<int>(EShaderType::gaussianBlur)]->Deactivate();
#endif
	}

	void OpenGL_Renderer::ResizeViewport(unsigned width, unsigned height)
	{
		m_viewportWidth = width;
		m_viewportHeight = height;

		if (width == 0 || height == 0)
		{
			m_viewportWidth = 1;
			m_viewportHeight = 1;
		}

		CreateBloomPassResources();
		CreateMSAAResources();
	}

	unsigned long OpenGL_Renderer::ParseValueTypeToGL(EValueTypes valueType)
	{
		switch (valueType)
		{
		case EValueTypes::BYTE:
			return GL_BYTE;
		case EValueTypes::UNSIGNED_BYTE:
			return GL_UNSIGNED_BYTE;
		case EValueTypes::UNSIGNED_SHORT:
			return GL_UNSIGNED_SHORT;
		case EValueTypes::INT:
			return GL_INT;
		case EValueTypes::UNSIGNED_INT:
			return GL_UNSIGNED_INT;
		case EValueTypes::FLOAT:
			return GL_FLOAT;
		default:
			printf("ERROR: Unspecified value type: %i\n.", static_cast<int>(valueType));
		}

		assert(false);
		return 0;
	}

	unsigned long OpenGL_Renderer::ParseBufferTypeToGL(EDrawTypes drawTypes)
	{
		switch (drawTypes)
		{
		case EDrawTypes::STREAM_DRAW:
			return GL_STREAM_DRAW;
		case EDrawTypes::STREAM_READ:
			return GL_STREAM_READ;
		case EDrawTypes::STREAM_COPY:
			return GL_STREAM_COPY;
		case EDrawTypes::STATIC_DRAW:
			return GL_STATIC_DRAW;
		case EDrawTypes::STATIC_READ:
			return GL_STATIC_READ;
		case EDrawTypes::STATIC_COPY:
			return GL_STATIC_COPY;
		case EDrawTypes::DYNAMIC_DRAW:
			return GL_DYNAMIC_DRAW;
		case EDrawTypes::DYNAMIC_READ:
			return GL_DYNAMIC_READ;
		case EDrawTypes::DYNAMIC_COPY:
			return GL_DYNAMIC_COPY;
		default:
			printf("ERROR: Unspecified value type: %i\n.", static_cast<int>(drawTypes));
		}

		assert(false);
		return 0;
	}

	void OpenGL_Renderer::InitializeShadowRenderPass()
	{
		glGenFramebuffers(1, &m_depthMapFBO);

		glGenTextures(1, &m_depthTextureId);
		glBindTexture(GL_TEXTURE_2D, m_depthTextureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_SHADOW_WIDTH, m_SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTextureId, 0);
		glDrawBuffer(GL_NONE); //We're not rendering color data.
		glReadBuffer(GL_NONE); //We're not rendering color data.
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGL_Renderer::CreateBloomPassResources()
	{
		if (m_hdrFBO != 0)
		{
			glDeleteFramebuffers(1, &m_hdrFBO);
			glDeleteRenderbuffers(1, &m_rboDepthFBO);
			glDeleteFramebuffers(1, &m_pingpongFBO);
			glDeleteTextures(3, m_pingpongColorbuffers);
		}

		// set up floating point framebuffer to render scene to
		glGenFramebuffers(1, &m_hdrFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);

		//Create the color output targets.
		glGenTextures(2, m_colorBuffers);

		//The color buffer.
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_colorBuffers[0]);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_renderSettings.MSAAProps.SampleCount, GL_RGBA16F, m_viewportWidth, m_viewportHeight, GL_TRUE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_colorBuffers[0], 0);

		//The emission buffer.
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_colorBuffers[1]);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_renderSettings.MSAAProps.SampleCount, GL_RGBA16F, m_viewportWidth, m_viewportHeight, GL_TRUE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, m_colorBuffers[1], 0);

		//Set the mipmap level for the emission buffer.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, int(std::log2(std::max(m_viewportWidth, m_viewportHeight))));
		glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, &m_maxMipMapLevel);
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &m_maxGroupInvocations);

		//Create render buffer.
		glGenRenderbuffers(1, &m_rboDepthFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, m_rboDepthFBO);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_renderSettings.MSAAProps.SampleCount, GL_DEPTH_COMPONENT, m_viewportWidth, m_viewportHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rboDepthFBO);

		//Set the render ouput buffers.
		glDrawBuffers(2, m_hdrBFOAttachments);

		//After completing the process check the status of the fbo.
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			TOR_ASSERT("The frame buffer for the post processing failed to instantiate.");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//Create the 2 pingpong color buffers.
		glGenTextures(3, m_pingpongColorbuffers);
		for (unsigned int i = 0; i < 2; i++)
		{
			glBindTexture(GL_TEXTURE_2D, m_pingpongColorbuffers[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_viewportWidth, m_viewportHeight, 0, GL_RGBA, GL_FLOAT, NULL);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		// ping-pong-textures for blurring
		glGenFramebuffers(1, &m_pingpongFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO);

		//Create an unsampled texture for the extraction buffer of the bloom.
		glBindTexture(GL_TEXTURE_2D, m_pingpongColorbuffers[2]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_viewportWidth, m_viewportHeight, 0, GL_RGBA, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pingpongColorbuffers[2], 0);
		
		//also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			TOR_ASSERT("The frame buffer for the post processing failed to instantiate.");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGL_Renderer::CreateMSAAResources()
	{
		if (m_hdrBlitFBO != 0)
		{
			glDeleteFramebuffers(1, &m_hdrBlitFBO);
			glDeleteTextures(1, &m_blitColorBuffer);
		}

		// Create temporary FBO to blit HDR Image to.
		glGenFramebuffers(1, &m_hdrBlitFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_hdrBlitFBO);
		
		glGenTextures(1, &m_blitColorBuffer);
		glBindTexture(GL_TEXTURE_2D, m_blitColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_viewportWidth, m_viewportHeight, 0, GL_RGBA, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_blitColorBuffer, 0);

		// also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			TOR_ASSERT("The frame buffer for the post processing failed to instantiate.");
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGL_Renderer::InitializePostProcessingPass()
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

	void OpenGL_Renderer::Shutdown()
	{
	}

#if defined (PLATFORM_WINDOWS)
	void APIENTRY
#elif defined (PLATFORM_SWITCH)
	void NN_GLL_GL_APIENTRY
#endif
		OpenGL_Renderer::DebugCallbackFunc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam)
	{
		//Exclude some logs on release.
#if defined(CONFIG_RELEASE)
		switch (id)
		{
			//A debug context shouldn't exist on release.
		case 131218:	// http://stackoverflow.com/questions/12004396/opengl-debug-context-performance-warning
			return;
		}
#endif

		switch (id)
		{
		case 131185: //nonimportant error calls.
			return;
		}

		std::string sourceString;
		std::string typeString;
		std::string severityString;

		switch (source)
		{
		case GL_DEBUG_SOURCE_API:
		{
			sourceString = "API";
			break;
		}
		case GL_DEBUG_SOURCE_APPLICATION:
		{
			sourceString = "Application";
			break;
		}
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		{
			sourceString = "Window System";
			break;
		}
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
		{
			sourceString = "Shader Compiler";
			break;
		}
		case GL_DEBUG_SOURCE_THIRD_PARTY:
		{
			sourceString = "Third Party";
			break;
		}
		case GL_DEBUG_SOURCE_OTHER:
		{
			sourceString = "Other";
			break;
		}
		default:
		{
			sourceString = "Unknown";
			break;
		}
		}

		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:
		{
			typeString = "Error";
			break;
		}
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		{
			typeString = "Deprecated Behavior";
			break;
		}
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		{
			typeString = "Undefined Behavior";
			break;
		}
		case GL_DEBUG_TYPE_PERFORMANCE:
		{
			typeString = "Performance";
			break;
		}
		case GL_DEBUG_TYPE_OTHER:
		{
			typeString = "Other";
			break;
		}
		default: {
			typeString = "Unknown";
			break;
		}
		}

		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:
		{
			severityString = "High";
			break;
		}
		case GL_DEBUG_SEVERITY_MEDIUM:
		{
			severityString = "Medium";
			break;
		}
		case GL_DEBUG_SEVERITY_LOW:
		{
			severityString = "Low";
			break;
		}
		case GL_DEBUG_SEVERITY_NOTIFICATION:
		{
			return;
		}
		default:
		{
			severityString = "Unknown";
			break;
		}
		}

		spdlog::error("GL Debug Callback:");
		spdlog::error("  source:       {source}:{sourceString}", fmt::arg("source", source), fmt::arg("sourceString", sourceString.c_str()));
		spdlog::error("  type:         {source}:{typeString}", fmt::arg("source", type), fmt::arg("typeString", typeString.c_str()));
		spdlog::error("  id:           {id}", fmt::arg("id", id));
		spdlog::error("  severity:     {severity}:{severityString}", fmt::arg("severity", severity), fmt::arg("severityString", severityString.c_str()));
		spdlog::error("  message:      {message}", fmt::arg("message", message));
		spdlog::error("------------------------------------------------------------------");
		TOR_ASSERT(type != GL_DEBUG_TYPE_ERROR);
	}
}