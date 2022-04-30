#pragma once
#include <CoreRenderAPI/Components/RenderComponents.h>
#include <CoreRenderAPI/RenderAPI.h>
#include "Core/Entity.h"

namespace Frac
{
	enum class EGLTFModelNode
	{
		EMPTY = 0,

		MATERIAL,
		TEXTURE,
		SAMPLER,
		IMAGE,
		SCENE,
		MESH,
		CAMERA,

		POINTLIGHT,
		DIRECTIONALLIGHT,
		SPOTLIGHT,

		COUNT
	};

	struct GLTFBuildNode
	{
		std::string Name;
		TOR::Transform Transform;
		EGLTFModelNode Type = EGLTFModelNode::EMPTY;
		void* Data = nullptr;
		std::vector<GLTFBuildNode> Children;
	};

	/// <summary>
	/// The resource manager manages the lifetime of resources.
	/// </summary>
	class ResourceManager
	{
	public:
		ResourceManager(TOR::RenderAPI& renderAPI, EntityRegistry& registry);
		~ResourceManager();

		TOR::Material& LoadMaterial(const std::string& name);
		TOR::Texture& LoadTexture(const std::string& name);
		TOR::Sampler& LoadSampler(const std::string& name);
		TOR::Image& LoadImage(const std::string& name);
		TOR::Scene& LoadScene(const std::string& name);
		TOR::Mesh& LoadMesh(const std::string& name);
		TOR::Camera& LoadCamera(const std::string& name);
		TOR::PointLight& LoadPointLight(const std::string& name);
		TOR::DirectionalLight& LoadDirectionalLight(const std::string& name);
		TOR::SpotLight& LoadSpotLight(const std::string& name);
		GLTFBuildNode& LoadGltfModelNode(const std::string& name);

		TOR::Material& GetMaterial(const std::string& name);
		TOR::Texture& GetTexture(const std::string& name);
		TOR::Sampler& GetSampler(const std::string& name);
		TOR::Image& GetImage(const std::string& name);
		TOR::Scene& GetScene(const std::string& name);
		TOR::Mesh& GetMesh(const std::string& name);
		TOR::Camera& GetCamera(const std::string& name);
		TOR::PointLight& GetPointLight(const std::string& name);
		TOR::DirectionalLight& GetDirectionalLight(const std::string& name);
		TOR::SpotLight& GetSpotLight(const std::string& name);
		GLTFBuildNode& GetGltfModelNode(const std::string& name);

		void UnLoadMaterial(const std::string& name);
		void UnLoadTexture(const std::string& name);
		void UnLoadSampler(const std::string& name);
		void UnLoadImage(const std::string& name);
		void UnLoadScene(const std::string& name);
		void UnLoadMesh(const std::string& name);
		void UnLoadCamera(const std::string& name);
		void UnloadGltfFile(const std::string& name);
		void UnLoadPointLight(const std::string& name);
		void UnLoadDirectionalLight(const std::string& name);
		void UnLoadSpotLight(const std::string& name);

		bool IsMaterialLoaded(const std::string& name);
		bool IsTextureLoaded(const std::string& name);
		bool IsSamplerLoaded(const std::string& name);
		bool IsImageLoaded(const std::string& name);
		bool IsSceneLoaded(const std::string& name);
		bool IsMeshLoaded(const std::string& name);
		bool IsCameraLoaded(const std::string& name);
		bool IsGltfFileLoaded(const std::string& name);
		bool IsPointLightLoaded(const std::string& name);
		bool IsDirectionalLightLoaded(const std::string& name);
		bool IsSpotLightLoaded(const std::string& name);

	private:
		std::unordered_map<std::string, TOR::Material> m_materialBank;
		std::unordered_map<std::string, TOR::Texture> m_textureBank;
		std::unordered_map<std::string, TOR::Mesh> m_meshBank;
		std::unordered_map<std::string, TOR::Image> m_imageBank;
		std::unordered_map<std::string, TOR::Sampler> m_samplerBank;
		std::unordered_map<std::string, TOR::Scene> m_sceneBank;
		std::unordered_map<std::string, TOR::Camera> m_cameraBank;
		std::unordered_map<std::string, TOR::PointLight> m_pointLightBank;
		std::unordered_map<std::string, TOR::DirectionalLight> m_directionalLightBank;
		std::unordered_map<std::string, TOR::SpotLight> m_spotLightBank;
		std::unordered_map<std::string, GLTFBuildNode> m_modelNodeBank;

		EntityRegistry& m_entityRegistry;
		TOR::RenderAPI& m_renderAPI;
	};
}