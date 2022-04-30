#pragma once
#include "ModelBuilder.h"
#include <CoreRenderAPI/Components/Scene.h>
#include <CoreRenderAPI/Components/Model.h>
#include <CoreRenderAPI/Components/Mesh.h>
#include <tinygltf/tiny_gltf.h>

namespace Frac
{
	/// <summary>
	/// The glTFbuilder is a model builder implementation.
	/// </summary>
	class GLTF_ModelBuilder final : public ModelBuilder
	{
	public:
		GLTF_ModelBuilder(Frac::Renderer& renderSystem, ResourceManager& resourceBank, const std::string& filePath, bool loadUsingBinary, bool flipNormals = false);
		~GLTF_ModelBuilder() = default;

	protected:
		bool LoadModelFromAPI() override;
		bool ParseModelData() override;

	private:
		enum EExtensionValues
		{
			KHR_LIGHT = 7
		};

		enum class EDrawModes
		{
			POINTS,
			LINES,
			TRIANGLES
		};

		/// <summary>
		/// Mesh used to load 
		/// </summary>
		struct MeshLoadStruct
		{
			EDrawModes DrawMode;
			std::map<TOR::EVertexAttributes, int> Attributes;
			std::vector<TOR::EVertexAttributes> AttributesToLoad;
			int IndiceDataIndex;
			int MaterialDataIndex;
		};

		void LoadNode(GLTFBuildNode& parent, std::vector<tinygltf::Node>& nodes, std::set<std::string>& extentions, int nodeIndex);
		TOR::Transform LoadTransform(tinygltf::Node& nodeToLoad);
		void AddDefaultCamera(GLTFBuildNode& sceneRoot);
		void LoadCamera(tinygltf::Node& nodeToLoad);
		void LoadMesh(GLTFBuildNode& node, tinygltf::Node& nodeToLoad);
		void LoadSkin(tinygltf::Node& nodeToLoad);
		void LoadLight(GLTFBuildNode& lightNode, tinygltf::Node& nodeToLoad, tinygltf::Value& extentionValue);
		GLTFBuildNode LoadTexture(int index, bool isSRGB = false);

		const std::string& m_filePath;
		bool m_LoadUsingBinary;
		bool m_flipNormals;
		tinygltf::Model m_tinyGLTFModel;
	};
}