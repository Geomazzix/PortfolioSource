#pragma once
#include <vector>
#include <CoreRenderAPI/Components/Mesh.h>
#include <CoreRenderAPI/Components/Texture.h>
#include <CoreRenderAPI/Components/Light.h>
#include <CoreRenderAPI/Components/Material.h>
#include <CoreRenderAPI/Components/Transform.h>
#include <CoreRenderAPI/Components/Sampler.h>
#include <CoreRenderAPI/Components/Camera.h>

namespace TOR
{
	/// <summary>
	/// The supported values within the opengl api.
	/// </summary>
	enum class EValueTypes
	{
		BYTE = 0x1400,		//1 byte
		UNSIGNED_BYTE = 0x1401,		//1 byte
		SHORT = 0x1402,		//2 bytes
		UNSIGNED_SHORT = 0x1403,		//2 bytes
		INT = 0x1404,		//4 bytes
		UNSIGNED_INT = 0x1405,		//4 bytes
		FLOAT = 0x1406		//4 bytes
	};

	/// <summary>
	/// Draw types used to to draw EBOs in different manners.
	/// </summary>
	enum class EDrawTypes
	{
		STREAM_DRAW = 0x88E0,
		STREAM_READ = 0x88E1,
		STREAM_COPY = 0x88E2,
		STATIC_DRAW = 0x88E4,
		STATIC_READ = 0x88E5,
		STATIC_COPY = 0x88E6,
		DYNAMIC_DRAW = 0x88E8,
		DYNAMIC_READ = 0x88E9,
		DYNAMIC_COPY = 0x88EA
	};

	/// <summary>
	/// The system states are used to actually check/read the performance from the renderer.
	/// </summary>
	struct SystemStats
	{
		float sceneUpdateTime = 0;
	};

	/// <summary>
	/// The low level API is used as the abstract base class for the external graphics API.
	/// </summary>
	class LowLevelAPI
	{
	public:
		//External graphics API creation.
		static LowLevelAPI* CreateCoreAPI(const char* dllName);

		//Renderer core methods.
		virtual void Init(const char* assetDirectory, unsigned viewportWidth, unsigned viewportHeight) = 0;
		virtual void Setting(const char* name, float value) = 0;
		virtual void Render(unsigned meshId) = 0;
		virtual void RenderInstanced(unsigned meshId, unsigned instanceCount) = 0;
		virtual void Present() = 0;
		virtual void Shutdown() = 0;

		//Shader property methods.
		virtual void ActivateShader(unsigned meshId) = 0;
		virtual void SetMeshProperties(unsigned meshId, const glm::mat4& modelTransform) = 0;
		virtual void SetMaterialProperties(unsigned meshId, Material& material) = 0;
		virtual void SetCameraProperties(unsigned meshId, const glm::mat4& worldTransform, const TOR::Camera& camera) = 0;
		virtual void SetDirectionalLightProperties(unsigned meshId, const glm::mat4& lightWorldMatrix, const DirectionalLight& light) = 0;
		virtual void SetPointLightProperties(unsigned meshId, const glm::mat4& lightWorldMatrix, const PointLight& light) = 0;
		virtual void SetSpotLightProperties(unsigned meshId, const glm::mat4& lightWorldMatrix, const SpotLight& light) = 0;
		virtual void SetMeshShadowProperties(unsigned meshId, const glm::mat4& modelTransform) = 0;

		//Mesh methods.
		virtual void CreateMesh(Mesh& meshStructToFillIn) = 0;
		virtual void BindMeshForRecording(unsigned meshId) = 0;
		virtual void UnbindMesh(unsigned meshId) = 0;
		virtual void BufferElementData(unsigned meshId, uint32_t byteLength, void* bufferStart, unsigned int numberOfElements, EDrawTypes drawType = EDrawTypes::DYNAMIC_DRAW) = 0;
		virtual void BufferAttributeData(unsigned meshId, EVertexAttributes attrType, uint32_t byteLength, uint32_t ByteStride, bool normalized, int attrElementCount,
			const void* bufferStart, int overwriteVertexAttribArrayPosition = -1, EValueTypes DataType = EValueTypes::FLOAT, EDrawTypes drawType = EDrawTypes::STATIC_DRAW) = 0;

		/// <summary>
		/// Sets up instancing data (Model transforms) for consequent draw calls
		/// </summary>
		virtual void BufferInstanceAttributeData(unsigned meshId, uint32_t byteLength, const void* bufferStart, EDrawTypes DrawMode = EDrawTypes::STATIC_DRAW) = 0;
		virtual void BufferInstanceNormalAttributeData(unsigned meshId, uint32_t byteLength, const void* bufferStart, EDrawTypes DrawMode = EDrawTypes::STATIC_DRAW) = 0;
		virtual void BufferAnyInstancedAttributeData(unsigned meshId, uint32_t byteLength, const void* bufferStart, int AttributeLocation, int elementCount, EDrawTypes DrawMode = EDrawTypes::STATIC_DRAW) = 0;

		virtual void DestroyMesh(unsigned meshId) = 0;

		//Texture methods.
		virtual void CreateTexture(Texture& texture) = 0;
		virtual void LoadTexture(unsigned textureId, const Image& image, unsigned char* source, int sourceCountInElements, const Sampler& sampler, int format) = 0;
		virtual void DestroyTexture(unsigned textureId) = 0;
		virtual void BindTexture(unsigned textureId, unsigned int slot) = 0;
		virtual void UnbindTexture(unsigned textureId) = 0;
		virtual void AssignShaderToMesh(unsigned meshId, EShaderType type) = 0;

		virtual void ApplyBloomPass() = 0;

		virtual void ResizeViewport(unsigned viewportWidth, unsigned viewportHeight) = 0;
		virtual void SetupShadowPass(const glm::mat4x4& shadowViewMatrix) = 0;
		virtual void EndShadowPass() = 0;
		virtual void SetupScenePass() = 0;
		virtual void EndScenePass() = 0;
		virtual void SetupPostProcessingPass() = 0;
		virtual void EndPostProcessingPass() = 0;

	private:
		static LowLevelAPI* m_lowLevelRenderer;
	};
}