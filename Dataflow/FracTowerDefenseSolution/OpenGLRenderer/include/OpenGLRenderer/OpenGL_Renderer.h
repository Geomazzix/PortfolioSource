#pragma once
#include <CoreRenderAPI/LowLevelAPI.h>
#include "OpenGLRenderer/InternalRenderingStructures/OpenGLMesh.h"
#include "OpenGLRenderer/InternalRenderingStructures/OpenGLTexture.h"
#include "OpenGLRenderer/InternalRenderingStructures/OpenGLShader.h"

#if defined(PLATFORM_SWITCH)
#define TOR_API
#elif defined(PLATFORM_WINDOWS)
#ifdef TOR_API_EXPORTS
#define TOR_API __declspec(dllexport)
#else
#define TOR_API __declspec(dllimport)
#endif
#endif

namespace TOR
{
	struct ComputeSettings
	{
		int DispatchUnitSizeX = 32;
		int DispatchUnitSizeY = 32;
	};

	struct BloomSettings
	{
		int StartMipLevel = 3;
		float Intensity = 0.4f;
		int SmallestMipWidth = 96;
	};

	struct FogSettings
	{
		float Density = 0.012f;
		glm::vec3 Color = glm::vec3(0xE7 / 255.f, 0x62 / 255.f, 0xE4 / 255.f);
	};

	struct MSAASettings
	{
		int SampleCount = 4;
	};

	struct ToneMappingSettings
	{
		float Exposure = 1.0f;
		float Gamma = 2.2f;
	};

	/// <summary>
	/// The render settings of the renderer.
	/// </summary>
	struct RenderSettings
	{
		MSAASettings MSAAProps;
		FogSettings FogProps;
		ComputeSettings ComputeProps;
		BloomSettings BloomProps;
		ToneMappingSettings ToneMappingProps;
	};

	struct Mesh;
	struct Texture;
	struct Camera;
	struct DirectionalLight;

	/// <summary>
	/// The OpenGL renderer implements the low level API functionality, thereby making sure that models can be rendered.
	/// </summary>
	class OpenGL_Renderer : public LowLevelAPI
	{
	public:
		OpenGL_Renderer();
		~OpenGL_Renderer() = default;

		void Init(const char* assetDirectory, unsigned viewportWidth, unsigned viewportHeight) override;
		void Shutdown() override;

		void InitDebugGlCallbacks();
		void Setting(const char* name, float value) override;

		void Render(unsigned meshId) override;
		void RenderInstanced(unsigned meshId, unsigned instanceCount) override;
		void Present() override;

		void SetCameraProperties(unsigned meshId, const glm::mat4& worldTransform, const TOR::Camera& camera) override;
		void ActivateShader(unsigned meshId);
		void SetMaterialProperties(unsigned meshId, Material& material) override;
		void SetDirectionalLightProperties(unsigned meshId, const glm::mat4& lightWorldMatrix, const DirectionalLight& light) override;
		void SetPointLightProperties(unsigned meshId, const glm::mat4& lightWorldMatrix, const PointLight& light) override;
		void SetSpotLightProperties(unsigned meshId, const glm::mat4& lightWorldMatrix, const SpotLight& light) override;
		void SetMeshProperties(unsigned meshId, const glm::mat4& modelTransform) override;
		void SetMeshShadowProperties(unsigned meshId, const glm::mat4& modelTransform) override;

		void CreateMesh(Mesh& meshStructToFillIn) override;
		void DestroyMesh(unsigned meshId) override;
		void BindMeshForRecording(unsigned meshId) override;
		void UnbindMesh(unsigned meshId) override;
		void BufferAttributeData(unsigned meshId, EVertexAttributes attrType, uint32_t byteLength, uint32_t ByteStride, bool normalized, int attrElementCount, const void* bufferStart, int overwriteVertexAttribArrayPosition = -1, EValueTypes DataType = EValueTypes::FLOAT, EDrawTypes DrawMode = EDrawTypes::STATIC_DRAW) override;
		void BufferInstanceAttributeData(unsigned meshId, uint32_t byteLength, const void* bufferStart, EDrawTypes DrawMode = EDrawTypes::STATIC_DRAW) override;
		void BufferInstanceNormalAttributeData(unsigned meshId, uint32_t byteLength, const void* bufferStart, EDrawTypes DrawMode = EDrawTypes::STATIC_DRAW) override;
		void BufferAnyInstancedAttributeData(unsigned meshId, uint32_t byteLength, const void* bufferStart, int AttributeLocation, int elementCount, EDrawTypes DrawMode /*= EDrawTypes::STATIC_DRAW*/);
		void BufferElementData(unsigned meshId, uint32_t byteLength, void* bufferStart, unsigned int numberOfElements, EDrawTypes DrawMode = EDrawTypes::STATIC_DRAW) override;

		void CreateTexture(Texture& texture) override;
		void LoadTexture(unsigned textureId, const Image& image, unsigned char* source, int sourceCountInElements, const Sampler& sampler, int format) override;
		void DestroyTexture(unsigned textureId) override;
		void BindTexture(unsigned textureId, unsigned int slot) override;
		void UnbindTexture(unsigned textureId) override;

		void AssignShaderToMesh(unsigned meshId, EShaderType type) override;

		void ApplyBloomPass() override;

		void SetupShadowPass(const glm::mat4x4& shadowViewMatrix) override;
		void EndShadowPass() override;
		void SetupScenePass() override;
		void EndScenePass() override;
		void SetupPostProcessingPass() override;
		void EndPostProcessingPass() override;

		void ResizeViewport(unsigned width, unsigned height) override;

	private:
		void InitializeShadowRenderPass();
		void CreateBloomPassResources();
		void CreateMSAAResources();
		void InitializePostProcessingPass();

#if defined (PLATFORM_WINDOWS)
		static void APIENTRY
#elif defined (PLATFORM_SWITCH)
		static void NN_GLL_GL_APIENTRY
#endif
			DebugCallbackFunc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);

		std::unordered_map<int, OpenGLMesh> m_meshMap;
		std::unordered_map<int, OpenGLTexture> m_textureMap;
		std::unordered_map<int, std::shared_ptr<OpenGLShader>> m_shaderMap;

		int m_directionalLightCount;
		int m_pointLightCount;
		int m_spotLightCount;

		unsigned m_depthMapFBO;
		unsigned m_depthTextureId;
		const unsigned m_SHADOW_WIDTH, m_SHADOW_HEIGHT;
		unsigned m_viewportWidth, m_viewportHeight;

		unsigned int m_hdrFBO;
		unsigned int m_hdrBlitFBO;
		unsigned int m_msaaFBO;
		unsigned int m_msaaTexture;
		unsigned int m_colorBuffers[2];
		unsigned int m_blitColorBuffer;
		unsigned int m_hdrBFOAttachments[2] =
		{
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1
		};

		unsigned int m_rboDepthFBO;
		unsigned int m_quadVAO;
		unsigned int m_quadVBO;
		unsigned int m_quadEBO;

		unsigned int m_pingpongFBO;
		unsigned int m_pingpongColorbuffers[3];
		bool m_pingpongState = true;

		int m_maxMipMapLevel;
		int m_finalMipMapLevel;
		int m_maxGroupInvocations;

		glm::mat4x4 m_lightViewMatrix;

		unsigned long ParseValueTypeToGL(EValueTypes valueType);
		unsigned long ParseBufferTypeToGL(EDrawTypes drawTypes);

		RenderSettings m_renderSettings;
	};
}