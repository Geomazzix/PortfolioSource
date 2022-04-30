#include "MrpocPch.h"
#include <Resources/ResourceHandler.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/printf.h>

namespace mrpoc
{
	ResourceHandler::ResourceHandler()
	{
		std::shared_ptr<Shader> diffuse = std::make_shared<Shader>("resources/Shaders/vs/diffuse.vert", "resources/Shaders/fs/diffuse.frag");
		CacheShader("resources/Shaders/vs/diffuse.vert", diffuse);
		std::shared_ptr<Shader> diffusePBR = std::make_shared<Shader>("resources/Shaders/vs/diffusePBR.vert", "resources/Shaders/fs/diffusePBR.frag");
		CacheShader("resources/Shaders/vs/diffusePBR.vert", diffusePBR);
		std::shared_ptr<Shader> screenShader = std::make_shared<Shader>("resources/Shaders/vs/SampleScenePass.vert", "resources/Shaders/fs/SampleScenePass.frag");
		CacheShader("resources/Shaders/vs/SampleScenePass.vert", screenShader);
		std::shared_ptr<Shader> normalMapShader = std::make_shared<Shader>("resources/Shaders/vs/diffuseWithNormalMap.vert", "resources/Shaders/fs/diffuseWithNormalMap.frag");
		CacheShader("resources/Shaders/vs/diffuseWithNormalMap.vert", normalMapShader);
	}

	ResourceHandler::~ResourceHandler()
	{
		Clear();
	}

	bool ResourceHandler::ContainsMesh(const std::string& filepath)
	{
		if (m_meshStorage.find(filepath) != m_meshStorage.end())
		{
			return true;
		}
		return false;
	}

	bool ResourceHandler::ContainsShader(const std::string& filepath)
	{
		if (m_shaderStorage.find(filepath) != m_shaderStorage.end())
		{
			return true;
		}
		return false;
	}

	bool ResourceHandler::ContainsTexture(const std::string& filepath)
	{
		if (m_textureStorage.find(filepath) != m_textureStorage.end())
		{
			return true;
		}
		return false;
	}

	bool ResourceHandler::ContainsMaterial(const std::string& filepath)
	{
		if (m_materialStorage.find(filepath) != m_materialStorage.end())
		{
			return true;
		}
		return false;
	}

	bool ResourceHandler::ContainsSampler(const std::string& filepath)
	{
		if (m_SamplerStorage.find(filepath) != m_SamplerStorage.end())
		{
			return true;
		}
		return false;
	}

	void ResourceHandler::CacheMesh(const std::string& filepath, std::shared_ptr<Mesh> mesh)
	{
		m_meshStorage.emplace(filepath, mesh);
	}

	void ResourceHandler::CacheShader(const std::string& filepath, std::shared_ptr<Shader> shader)
	{
		m_shaderStorage.emplace(filepath, shader);
	}

	void ResourceHandler::CacheTexture(const std::string& filepath, std::shared_ptr<Texture> texture)
	{
		m_textureStorage.emplace(filepath, texture);
	}

	void ResourceHandler::CacheMaterial(const std::string& filepath, std::shared_ptr<Material> material)
	{
		m_materialStorage.emplace(filepath, material);
	}

	void ResourceHandler::CacheSampler(const std::string& filepath, std::shared_ptr<Sampler> sampler)
	{
		m_SamplerStorage.emplace(filepath, sampler);
	}

	std::weak_ptr<mrpoc::Mesh> ResourceHandler::GetMesh(const std::string& filepath)
	{
		if (m_meshStorage.find(filepath) == m_meshStorage.end())
		{
			spdlog::error(std::string(fmt::sprintf("%s is not cached and can therefor not be retrieved.", filepath.c_str())));
		}

		return m_meshStorage[filepath];
	}

	std::weak_ptr<mrpoc::Shader> ResourceHandler::GetShader(const std::string& filepath)
	{
		if (m_shaderStorage.find(filepath) == m_shaderStorage.end())
		{
			spdlog::error(std::string(fmt::sprintf("%s is not cached and can therefor not be retrieved.", filepath.c_str())));
		}

		return m_shaderStorage[filepath];
	}

	std::weak_ptr<mrpoc::Texture> ResourceHandler::GetTexture(const std::string& filepath)
	{
		if (m_textureStorage.find(filepath) == m_textureStorage.end())
		{
			spdlog::error(std::string(fmt::sprintf("%s is not cached and can therefor not be retrieved.", filepath.c_str())));
		}

		return m_textureStorage[filepath];
	}

	std::weak_ptr<mrpoc::Material> ResourceHandler::GetMaterial(const std::string& filepath)
	{
		if (m_materialStorage.find(filepath) == m_materialStorage.end())
		{
			spdlog::error(std::string(fmt::sprintf("%s is not cached and can therefor not be retrieved.", filepath.c_str())));
		}

		return m_materialStorage[filepath];
	}

	std::weak_ptr<Sampler> ResourceHandler::GetSampler(const std::string& filepath)
	{
		if (m_SamplerStorage.find(filepath) == m_SamplerStorage.end())
		{
			spdlog::error(std::string(fmt::sprintf("%s is not cached and can therefor not be retrieved.", filepath.c_str())));
		}

		return m_SamplerStorage[filepath];
	}

	void ResourceHandler::UnloadMesh(const std::string& filepath)
	{
		if (m_meshStorage.find(filepath) == m_meshStorage.end())
		{
			spdlog::error(std::string(fmt::sprintf("Failed to unload mesh: %s", filepath.c_str())));
			return;
		}

		m_meshStorage.erase(filepath);
	}

	void ResourceHandler::UnloadShader(const std::string& filepath)
	{
		if (m_shaderStorage.find(filepath) == m_shaderStorage.end())
		{
			spdlog::error(std::string(fmt::sprintf("Failed to unload shader: %s", filepath.c_str())));
			return;
		}

		m_shaderStorage.erase(filepath);
	}

	void ResourceHandler::UnloadTexture(const std::string& filepath)
	{
		if (m_textureStorage.find(filepath) == m_textureStorage.end())
		{
			spdlog::error(std::string(fmt::sprintf("Failed to unload texture: %s", filepath.c_str())));
			return;
		}

		m_textureStorage.erase(filepath);
	}

	void ResourceHandler::UnloadMaterial(const std::string& filepath)
	{
		if (m_materialStorage.find(filepath) == m_materialStorage.end())
		{
			spdlog::error(std::string(fmt::sprintf("Failed to unload material: %s", filepath.c_str())));
			return;
		}

		m_materialStorage.erase(filepath);
	}

	void ResourceHandler::UnloadSampler(const std::string& filepath)
	{
		if (m_SamplerStorage.find(filepath) == m_SamplerStorage.end())
		{
			spdlog::error(std::string(fmt::sprintf("Failed to unload material: %s", filepath.c_str())));
			return;
		}

		m_SamplerStorage.erase(filepath);
	}

	void ResourceHandler::Clear()
	{
		m_meshStorage.clear();
		m_shaderStorage.clear();
		m_textureStorage.clear();
		m_materialStorage.clear();
	}
}