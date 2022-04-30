#pragma once
#include <Resources/Material.h>
#include <Resources/Mesh.h>
#include <Resources/Texture.h>
#include <Resources/Shader.h>

#include <unordered_map>
#include <memory>
#include <string>

namespace mrpoc
{
	/// <summary>
	/// Handles the ownership of the resources of the renderer so they can be reused later on.
	/// </summary>
	class ResourceHandler
	{
	public:
		ResourceHandler();
		~ResourceHandler();

		bool ContainsMesh(const std::string& filepath);
		bool ContainsShader(const std::string& filepath);
		bool ContainsTexture(const std::string& filepath);
		bool ContainsMaterial(const std::string& filepath);
		bool ContainsSampler(const std::string& filepath);

		void CacheMesh(const std::string& filepath, std::shared_ptr<Mesh> mesh);
		void CacheShader(const std::string& filepath, std::shared_ptr<Shader> shader);
		void CacheTexture(const std::string& filepath, std::shared_ptr<Texture> texture);
		void CacheMaterial(const std::string& filepath, std::shared_ptr<Material> material);
		void CacheSampler(const std::string& filepath, std::shared_ptr<Sampler> sampler);

		std::weak_ptr<Mesh> GetMesh(const std::string& filepath);
		std::weak_ptr<Shader> GetShader(const std::string& filepath);
		std::weak_ptr<Texture> GetTexture(const std::string& filepath);
		std::weak_ptr<Material> GetMaterial(const std::string& filepath);
		std::weak_ptr<Sampler> GetSampler(const std::string& filepath);

		void UnloadMesh(const std::string& filepath);
		void UnloadShader(const std::string& filepath);
		void UnloadTexture(const std::string& filepath);
		void UnloadMaterial(const std::string& filepath);
		void UnloadSampler(const std::string& filepath);

		void Clear();

	private:
		std::unordered_map<std::string, std::shared_ptr<Mesh>> m_meshStorage;
		std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaderStorage;
		std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureStorage;
		std::unordered_map<std::string, std::shared_ptr<Material>> m_materialStorage;
		std::unordered_map<std::string, std::shared_ptr<Sampler>> m_SamplerStorage;
	};
}