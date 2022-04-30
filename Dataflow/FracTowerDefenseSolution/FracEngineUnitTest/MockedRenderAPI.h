#pragma once
#include <CoreRenderAPI/RenderAPI.h>
#include "gmock/gmock.h"

using namespace TOR;

class MockedRenderAPI : public RenderAPI {
public:
	MockedRenderAPI();
	MOCK_METHOD3(Initialize, void(const std::string& assetDirectory, unsigned viewportWidth, unsigned viewportHeight));
	MOCK_METHOD0(Shutdown, void());

	//Mesh methods.
	MOCK_METHOD1(CreateMesh, void(Mesh& meshStructToFill));
	MOCK_METHOD1(DestroyMesh, void(unsigned meshId));
	MOCK_METHOD1(BindMeshForRecording, void(unsigned meshId));
	MOCK_METHOD1(UnbindMesh, void(unsigned meshId));
	MOCK_METHOD8(BufferAttributeData, void(unsigned meshId, EVertexAttributes& attrType, uint32_t byteLength, uint32_t byteStride, int attrElementCount, const void* bufferStart, EValueTypes DataType, EDrawTypes DrawMode));
	MOCK_METHOD4(BufferElementData, void(unsigned meshId, uint32_t byteLength, void* bufferStart, EDrawTypes DrawMode));

	//Texture met
	MOCK_METHOD1(CreateTexture, void(Texture& texture));
	MOCK_METHOD6(LoadTexture, void(unsigned textureId, const Image& image, unsigned char* source, int sourceCountInElements, const Sampler& sampler, int format));
	MOCK_METHOD1(DestroyTexture, void(unsigned textureId));
	MOCK_METHOD2(BindTexture, void(unsigned textureId, unsigned int slot));
	MOCK_METHOD1(UnbindTexture, void(unsigned textureId));

	//Material me.
	MOCK_METHOD3(SetCameraProperties, void(unsigned meshId, const glm::mat4& worldTransform, TOR::Camera& camera));
	MOCK_METHOD2(SetMaterialProperties, void(unsigned meshId, PBRMaterial& material));


	MOCK_METHOD4(SetDirectionalLightProperties, void(unsigned meshId, const glm::mat4& lightWorldMatrix, DirectionalLight& light, Material& material));
	MOCK_METHOD4(SetPointlightProperties, void(unsigned meshId, const glm::mat4& lightWorldMatrix, PointLight& light, Material& material));
	MOCK_METHOD4(SetSpotlightProperties, void(unsigned meshId, const glm::mat4& lightWorldMatrix, SpotLight& light, Material& material));

	MOCK_METHOD2(SetMeshProperties, void(unsigned meshId, const glm::mat4& modelTransform));

	MOCK_METHOD2(AssignShaderToMesh, void(unsigned meshId, EShaderType type));

	//Actual rend methods.
	MOCK_METHOD2(ResizeViewport, void(unsigned viewportWidth, unsigned viewportHeight));
	MOCK_METHOD2(Render, void(unsigned meshId, unsigned indexCount));
	MOCK_METHOD2(SetMeshShadowProperties, void(unsigned meshId, const glm::mat4& modelTransform));
	MOCK_METHOD2(SetupShadowPass, void(const glm::vec3& lightPosition, const glm::vec3& lightDirection));
	MOCK_METHOD0(EndShadowPass, void());
	MOCK_METHOD0(SetupScenePass, void());
	MOCK_METHOD0(EndScenePass, void());

};