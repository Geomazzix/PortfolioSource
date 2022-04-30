#pragma once
#include "Material.h"
#include <memory>
#include <unordered_map>
#include "Mesh.h"
#include "Model.h"
#include "Image.h"

namespace CRT
{
	/// <summary>
	/// The resource manager stores instances of data, which can be reused.
	/// </summary>
	class ResourceManager
	{
	public:
		ResourceManager() = default;
		~ResourceManager();

		void Initialize();

		static std::string GetDefaultMaterialName();

		void AddMaterial(const std::string& name, std::shared_ptr<Material> material);
		void AddModel(const std::string& name, std::shared_ptr<Model> model);
		void AddImage(const std::string& name, std::shared_ptr<Image> model);

		void DeleteMaterial(const std::string& name);
		void DeleteModel(const std::string& name);
		void DeleteImage(const std::string& name);

		std::weak_ptr<Material> GetMaterial(const std::string& name);
		std::weak_ptr<Model> GetModel(const std::string& name);
		std::weak_ptr<Image> GetImage(const std::string& name);

	private:
		std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;
		std::unordered_map<std::string, std::shared_ptr<Model>> m_Models;
		std::unordered_map<std::string, std::shared_ptr<Image>> m_Images;
	};
}