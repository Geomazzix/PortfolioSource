#include "ResourceManager.h"

namespace CRT
{
	ResourceManager::~ResourceManager()
	{
		m_Materials.clear();
	}

	void ResourceManager::Initialize()
	{
		std::string magentaMaterialName = std::string("No_Material_Loaded");
		std::shared_ptr<Material> material = std::make_shared<Material>();
		material->AlbedoCoefficient = glm::vec3(1.0f, 0.0f, 1.0f);
		material->SpecularCoefficient = glm::vec3(0.5f);
		material->PhongExponent = 1024;
		material->IsReflective = false;
		m_Materials.emplace(magentaMaterialName, material);
	}

	std::string ResourceManager::GetDefaultMaterialName()
	{
		return "No_Material_Loaded";
	}

	void ResourceManager::AddMaterial(const std::string& name, std::shared_ptr<Material> material)
	{
		if (m_Materials.find(name) != m_Materials.end())
		{
			printf("WARNING: The material '%s' has already been stored and will therefor not be stored again!\n", name.c_str());
			return;
		}
		m_Materials.emplace(name, material);
	}

	void ResourceManager::AddModel(const std::string& name, std::shared_ptr<Model> model)
	{
		if (m_Models.find(name) != m_Models.end())
		{
			printf("WARNING: The model '%s' has already been stored and will therefor not be stored again!\n", name.c_str());
			return;
		}
		m_Models.emplace(name, model);
	}

	void ResourceManager::AddImage(const std::string& name, std::shared_ptr<Image> image)
	{
		if (m_Images.find(name) != m_Images.end())
		{
			printf("WARNING: The material '%s' has already been stored and will therefor not be stored again!\n", name.c_str());
			return;
		}
		m_Images.emplace(name, image);
	}

	void ResourceManager::DeleteMaterial(const std::string& name)
	{
		if (m_Materials.find(name) == m_Materials.end())
		{
			printf("WARNING: The material '%s' has not yet been stored before and will therefor not be deleted!\n", name.c_str());
			return;
		}
		m_Materials.erase(name);
	}

	void ResourceManager::DeleteModel(const std::string& name)
	{
		if (m_Models.find(name) == m_Models.end())
		{
			printf("WARNING: The model '%s' has not yet been stored before and will therefor not be deleted!\n", name.c_str());
			return;
		}
		m_Models.erase(name);
	}

	void ResourceManager::DeleteImage(const std::string& name)
	{
		if (m_Images.find(name) == m_Images.end())
		{
			printf("WARNING: The image '%s' has not yet been stored before and will therefor not be deleted!\n", name.c_str());
			return;
		}
		m_Images.erase(name);
	}

	std::weak_ptr<CRT::Material> ResourceManager::GetMaterial(const std::string& name)
	{
		return m_Materials[name];
	}

	std::weak_ptr<CRT::Model> ResourceManager::GetModel(const std::string& name)
	{
		return m_Models[name];
	}

	std::weak_ptr<CRT::Image> ResourceManager::GetImage(const std::string& name)
	{
		return m_Images[name];
	}
}