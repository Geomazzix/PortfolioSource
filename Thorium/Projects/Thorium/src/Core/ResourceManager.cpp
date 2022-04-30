#include <Thpch.h>
#include <Core/ResourceManager.h>
#include <Core/MessageLogger.h>
#include <Core/WorldManager.h>
#include <Graphics/ModelLoading/ObjFactory.h>
#include <Graphics/ModelLoading/GLTFFactory.h>
#include <Components/ModelComponent.h>
#include <Components/TransformComponent.h>
#include <Graphics/ResourceLoader.h>

namespace Th
{
	ResourceManager& Th::ResourceManager::GetInstance()
	{
		static ResourceManager manager{};
		return manager;
	}

	void ResourceManager::Initialize()
	{
#if PLATFORM_PS4
		//Emitted PS4 code
#endif
	}

	void ResourceManager::Destroy()
	{
#if PLATFORM_PS4
		//Emitted PS4 code
#endif
	}

	std::string ResourceManager::LoadModel(ResourceLoader& resourceLoader, const std::string& filePath, EModelFileType modelType)
	{
		std::string evaluatedFilePath = EvaluateWildCard(filePath);
		if(m_ModelMap.find(evaluatedFilePath) != m_ModelMap.end())
		{
			return evaluatedFilePath;
		}

#if PLATFORM_WIN64
		m_ModelMap.emplace(evaluatedFilePath, resourceLoader.LoadModel(evaluatedFilePath, modelType));
#elif PLATFORM_PS4
		//Emitted PS4 code
#endif
		return evaluatedFilePath;
	}

	void ResourceManager::UnloadModel(const std::string& key)
	{
		std::string evaluatedFilePath = EvaluateWildCard(key);
		if(m_ModelMap.find(evaluatedFilePath) != m_ModelMap.end())
		{
			m_ModelMap.erase(evaluatedFilePath);
		}
		else
		{
			LOGERROR("The model you are trying to remove doesn't exist");
		}
	}

#if PLATFORM_WIN64
	std::string ResourceManager::LoadShader(ResourceLoader& resourceLoader, const std::string& vertShader, const std::string& fragShader)
	{
		//TODO: Find a way to store the shader filepath appropriately.
		std::string vertFilePath = EvaluateWildCard(vertShader);
		std::string fragFilePath = EvaluateWildCard(fragShader);

		if (m_ModelMap.find(vertFilePath) != m_ModelMap.end())
		{
			return vertFilePath;
		}

		m_ShaderMap.emplace(vertFilePath, resourceLoader.LoadShaderPipeline(vertFilePath, fragFilePath));
		return vertShader;
	}

#elif PLATFORM_PS4
	//Emitted PS4 code
#endif

	void ResourceManager::UnloadShader(const std::string& key)
	{
		std::string evaluatedFilePath = EvaluateWildCard(key);
		if (m_ShaderMap.find(evaluatedFilePath) != m_ShaderMap.end())
		{
			m_ShaderMap.erase(evaluatedFilePath);
		}
		else
		{
			LOGERROR("The shader you are trying to remove doesn't exist");
		}
	}

	Components::ModelComponent* Th::ResourceManager::GetModel(const std::string& key)
	{
		std::string evaluatedFilePath = EvaluateWildCard(key);
		if (m_ModelMap.find(evaluatedFilePath) != m_ModelMap.end())
		{
			return m_ModelMap[evaluatedFilePath];
		}
		else
		{
			LOGERROR("The model you are trying to get doesn't exist");
		}

		return nullptr;
	}

	Shader* ResourceManager::GetShader(const std::string& key)
	{
		std::string evaluatedFilePath = EvaluateWildCard(key);
		if (m_ShaderMap.find(evaluatedFilePath) != m_ShaderMap.end())
		{
			return m_ShaderMap[evaluatedFilePath];
		}
		else
		{
			LOGERROR("The shader:%s, you are trying to get doesn't exist", evaluatedFilePath.c_str());
			return nullptr;
		}
	}

	void ResourceManager::AddWildCard(const std::string& wildcard, const std::string& value)
	{
		if(m_wildcardMap.find(wildcard) != m_wildcardMap.end())
		{
			m_wildcardMap[wildcard] = value;
		}else
		{
			m_wildcardMap.insert(std::make_pair(wildcard, value));
		}
	}

	void ResourceManager::RemoveWildCard(const std::string& wildcard)
	{
		if(m_wildcardMap.find(wildcard) != m_wildcardMap.end())
		{
			m_wildcardMap.erase(wildcard);
		}else
		{
			printf("Wild card does not exist!\n");
		}
	}

	std::string ResourceManager::EvaluateWildCard(const std::string& filepath)
	{
		std::string parsedString{filepath};
	
		auto it = m_wildcardMap.begin();
		while (it != m_wildcardMap.end())
		{
			int found = parsedString.find(it->first);
			if(found != std::string::npos)
			{
				parsedString.replace(parsedString.begin() + found,parsedString.begin() + it->first.size(), it->second);
			}
			it++;
		}
		return parsedString;
	}
}
