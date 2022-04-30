#pragma once
#include <CoreRenderAPI/Components/Model.h>
#include "Graphics/RenderSystem.h"
#include "Core/ResourceBank.h"

namespace Frac
{
	/// <summary>
	/// Used to identify which file type should be loaded.
	/// </summary>
	enum class EModelFileType
	{
		GLTF = 0,
		GLB
	};

	/// <summary>
	/// Interface for the model loading.
	/// </summary>
	class ModelBuilder
	{
	public:
		friend class ModelDirector;

		ModelBuilder(Frac::Renderer& renderSystem, ResourceManager& resourceManager, const std::string& filePath);
		virtual ~ModelBuilder() = default;
		
	protected:
		virtual bool LoadModelFromAPI() = 0;
		virtual bool ParseModelData() = 0;

		ResourceManager& m_resourceManager;
		Renderer& m_renderSystem;
		std::string m_ModelName, m_ModelDirectory;
	};
}