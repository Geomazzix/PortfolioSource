#include "pch.h"
#include "Core/ResourceBank.h"
#include "Utility/FracAssert.h"
namespace Frac
{
	ResourceManager::ResourceManager(TOR::RenderAPI& renderAPI, EntityRegistry& registry) :
		m_entityRegistry(registry),
		m_renderAPI(renderAPI),
		m_sceneBank(),
		m_materialBank(),
		m_meshBank(),
		m_textureBank(),
		m_imageBank(),
		m_samplerBank(),
		m_cameraBank()
	{
	}

	ResourceManager::~ResourceManager()
	{
		m_materialBank.clear();
		m_textureBank.clear();
		m_meshBank.clear();
		m_imageBank.clear();
		m_samplerBank.clear();
		m_sceneBank.clear();
		m_cameraBank.clear();
	}

	TOR::Material& ResourceManager::LoadMaterial(const std::string& name)
	{
		m_materialBank.insert(std::make_pair(name, TOR::Material()));
		return m_materialBank[name];
	}

	TOR::Texture& ResourceManager::LoadTexture(const std::string& name)
	{
		m_textureBank.insert(std::make_pair(name, TOR::Texture()));
		return m_textureBank[name];
	}

	TOR::Sampler& ResourceManager::LoadSampler(const std::string& name)
	{
		m_samplerBank.insert(std::make_pair(name, TOR::Sampler()));
		return m_samplerBank[name];
	}

	TOR::Image& ResourceManager::LoadImage(const std::string& name)
	{
		m_imageBank.insert(std::make_pair(name, TOR::Image()));
		return m_imageBank[name];
	}

	TOR::Scene& ResourceManager::LoadScene(const std::string& name)
	{
		m_sceneBank.insert(std::make_pair(name, TOR::Scene()));
		return m_sceneBank[name];
	}

	TOR::Mesh& ResourceManager::LoadMesh(const std::string& name)
	{
		m_meshBank.insert(std::make_pair(name, TOR::Mesh()));
		return m_meshBank[name];
	}

	TOR::Camera& ResourceManager::LoadCamera(const std::string& name)
	{
		m_cameraBank.insert(std::make_pair(name, TOR::Camera()));
		return m_cameraBank[name];
	}

	TOR::PointLight& ResourceManager::LoadPointLight(const std::string& name)
	{
		m_pointLightBank.insert(std::make_pair(name, TOR::PointLight()));
		return m_pointLightBank[name];
	}

	TOR::DirectionalLight& ResourceManager::LoadDirectionalLight(const std::string& name)
	{
		m_directionalLightBank.insert(std::make_pair(name, TOR::DirectionalLight()));
		return m_directionalLightBank[name];
	}

	TOR::SpotLight& ResourceManager::LoadSpotLight(const std::string& name)
	{
		m_spotLightBank.insert(std::make_pair(name, TOR::SpotLight()));
		return m_spotLightBank[name];
	}

	Frac::GLTFBuildNode& ResourceManager::LoadGltfModelNode(const std::string& name)
	{
		m_modelNodeBank.insert(std::make_pair(name, GLTFBuildNode()));
		return m_modelNodeBank[name];
	}

	TOR::Material& ResourceManager::GetMaterial(const std::string& name)
	{
		FRAC_ASSERT(m_materialBank.find(name) != m_materialBank.end(), "ASSERT: GetMaterial non existent material requested");
		return m_materialBank[name];
	}

	TOR::Texture& ResourceManager::GetTexture(const std::string& name)
	{
		FRAC_ASSERT(m_textureBank.find(name) != m_textureBank.end(), "ASSERT: GetTexture non existent texture requested");
		return m_textureBank[name];
	}

	TOR::Sampler& ResourceManager::GetSampler(const std::string& name)
	{
		FRAC_ASSERT(m_samplerBank.find(name) != m_samplerBank.end(), "ASSERT: GetSampler non existent sampler requested");
		return m_samplerBank[name];
	}

	TOR::Image& ResourceManager::GetImage(const std::string& name)
	{
		FRAC_ASSERT(m_imageBank.find(name) != m_imageBank.end(), "ASSERT: GetImage non existent image requested");
		return m_imageBank[name];
	}

	TOR::Scene& ResourceManager::GetScene(const std::string& name)
	{
		FRAC_ASSERT(m_sceneBank.find(name) != m_sceneBank.end(), "ASSERT: GetScene non existent scene requested");
		return m_sceneBank[name];
	}

	TOR::Mesh& ResourceManager::GetMesh(const std::string& name)
	{
		FRAC_ASSERT(m_meshBank.find(name) != m_meshBank.end(), "ASSERT: GetMesh non existent mesh requested");
		return m_meshBank[name];
	}

	TOR::Camera& ResourceManager::GetCamera(const std::string& name)
	{
		FRAC_ASSERT(m_cameraBank.find(name) != m_cameraBank.end(), "ASSERT: GetCamera non existent camera requested");
		return m_cameraBank[name];
	}

	TOR::PointLight& ResourceManager::GetPointLight(const std::string& name)
	{
		FRAC_ASSERT(m_pointLightBank.find(name) != m_pointLightBank.end(), "ASSERT: GetPointLight non existent pointlight requested");
		return m_pointLightBank[name];
	}

	TOR::DirectionalLight& ResourceManager::GetDirectionalLight(const std::string& name)
	{
		FRAC_ASSERT(m_directionalLightBank.find(name) != m_directionalLightBank.end(), "ASSERT: GetDirectionalLight non existent directionallight requested");
		return m_directionalLightBank[name];
	}

	TOR::SpotLight& ResourceManager::GetSpotLight(const std::string& name)
	{
		FRAC_ASSERT(m_spotLightBank.find(name) != m_spotLightBank.end(), "ASSERT: GetSpotLight non existent spotlight requested");
		return m_spotLightBank[name];
	}

	Frac::GLTFBuildNode& ResourceManager::GetGltfModelNode(const std::string& name)
	{
		FRAC_ASSERT(m_modelNodeBank.find(name) != m_modelNodeBank.end(), "ASSERT: GetGltfModelNode non existent gltfNode requested");
		return m_modelNodeBank[name];
	}

	void ResourceManager::UnLoadMaterial(const std::string& name)
	{
		FRAC_ASSERT(m_materialBank.find(name) == m_materialBank.end(), "ASSERT: UnLoadMaterial non existent material deleted");
		m_materialBank.erase(name);
	}

	void ResourceManager::UnLoadTexture(const std::string& name)
	{
		FRAC_ASSERT(m_textureBank.find(name) == m_textureBank.end(), "ASSERT: UnLoadTexture non existent texture requested");
		TOR::Texture& texture = m_textureBank[name];
		m_renderAPI.DestroyTexture(texture.Id);
		m_textureBank.erase(name);
	}

	void ResourceManager::UnLoadSampler(const std::string& name)
	{
		FRAC_ASSERT(m_samplerBank.find(name) == m_samplerBank.end(), "ASSERT: UnLoadSampler non existent sampler requested");
		m_samplerBank.erase(name);
	}

	void ResourceManager::UnLoadImage(const std::string& name)
	{
		FRAC_ASSERT(m_imageBank.find(name) == m_imageBank.end(), "ASSERT: UnLoadImage non existent image requested");
		m_imageBank.erase(name);
	}

	void ResourceManager::UnLoadScene(const std::string& name)
	{
		FRAC_ASSERT(m_sceneBank.find(name) == m_sceneBank.end(), "ASSERT: UnLoadScene non existent scene requested");
		m_sceneBank.erase(name);
	}

	void ResourceManager::UnLoadMesh(const std::string& name)
	{
		FRAC_ASSERT(m_materialBank.find(name) == m_materialBank.end(), "ASSERT: UnLoadMesh non existent mesh requested");
		TOR::Mesh& mesh = m_meshBank[name];
		m_renderAPI.DestroyMesh(mesh.Id);
		m_meshBank.erase(name);
	}

	void ResourceManager::UnLoadCamera(const std::string& name)
	{
		FRAC_ASSERT(m_cameraBank.find(name) == m_cameraBank.end(), "ASSERT: UnLoadCamera non existent camera requested");
		m_cameraBank.erase(name);
	}

	void ResourceManager::UnloadGltfFile(const std::string& name)
	{
		FRAC_ASSERT(m_modelNodeBank.find(name) == m_modelNodeBank.end(), "ASSERT: UnloadGltfFile non existent gltfFile requested");
		m_modelNodeBank.erase(name);
	}

	bool ResourceManager::IsMaterialLoaded(const std::string& name)
	{
		if (m_materialBank.size() > 0)
			return m_materialBank.find(name) != m_materialBank.end();
		return false;
	}

	bool ResourceManager::IsTextureLoaded(const std::string& name)
	{
		if (m_textureBank.size() > 0)
			return m_textureBank.find(name) != m_textureBank.end();
		return false;
	}

	bool ResourceManager::IsSamplerLoaded(const std::string& name)
	{
		if (m_samplerBank.size() > 0)
			return m_samplerBank.find(name) != m_samplerBank.end();
		return false;
	}

	bool ResourceManager::IsImageLoaded(const std::string& name)
	{
		if (m_imageBank.size() > 0)
			return m_imageBank.find(name) != m_imageBank.end();
		return false;
	}

	bool ResourceManager::IsSceneLoaded(const std::string& name)
	{
		if (m_sceneBank.size() > 0)
			return m_sceneBank.find(name) != m_sceneBank.end();
		return false;
	}

	bool ResourceManager::IsMeshLoaded(const std::string& name)
	{
		if (m_meshBank.size() > 0)
			return m_meshBank.find(name) != m_meshBank.end();
		return false;
	}

	bool ResourceManager::IsCameraLoaded(const std::string& name)
	{
		if (m_cameraBank.size() > 0)
			return m_cameraBank.find(name) != m_cameraBank.end();
		return false;
	}

	bool ResourceManager::IsGltfFileLoaded(const std::string& name)
	{
		if (m_modelNodeBank.size() > 0)
			return m_modelNodeBank.find(name) != m_modelNodeBank.end();
		return false;
	}

	bool ResourceManager::IsPointLightLoaded(const std::string& name)
	{
		if (m_pointLightBank.size() > 0)
			return m_pointLightBank.find(name) != m_pointLightBank.end();
		return false;
	}

	bool ResourceManager::IsDirectionalLightLoaded(const std::string& name)
	{
		if (m_directionalLightBank.size() > 0)
			return m_directionalLightBank.find(name) != m_directionalLightBank.end();
		return false;
	}

	bool ResourceManager::IsSpotLightLoaded(const std::string& name)
	{
		if (m_spotLightBank.size() > 0)
			return m_spotLightBank.find(name) != m_spotLightBank.end();
		return false;
	}
}