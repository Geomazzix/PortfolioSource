#pragma once
#include <SceneManagement/Scene.h>
#include <memory>
#include <string>
#include <vector>
#include <map>

#include "Graphics/Vertex.h"

namespace tinygltf
{
	class Model;
	class Node;
	struct Scene;
}

namespace mrpoc
{
	class ResourceHandler;
	class Scene;
	class Node;
	class Texture;

	enum EExtensionValues
	{
		KHR_LIGHT = 7
	};

	enum class DrawModes {
		POINTS,
		LINES,
		TRIANGLES
	};

	struct MeshLoadStruct
	{
	public:
		MeshLoadStruct() = default;
		~MeshLoadStruct() = default;

		DrawModes m_drawMode;
		std::map<Attributes, int> m_attributes;
		std::vector<Attributes> m_attributesToLoad;
		int indiceDataIndex;
		int materialDataIndex;
	};

	class GLTFLoader {
	public:

		GLTFLoader();
		~GLTFLoader();

		bool LoadGLTFIntoScene(ResourceHandler& resourceHandler, std::vector<mrpoc::Scene>& aSceneVectorToLoadScenesInto, std::string aPathToGLTFFile, bool flipNormals = false);
		std::shared_ptr<mrpoc::Node> LoadNode(
			std::shared_ptr<mrpoc::Node> parent, 
			ResourceHandler& resourceHandler, 
			tinygltf::Model& tinyGLTFModel, 
			std::vector<tinygltf::Node>& nodes, 
			tinygltf::Scene& sceneToLoad, 
			mrpoc::Scene& sceneToCacheInto, 
			std::set<std::string>& extentions,
			int nodeIndex, bool flipNormals = false);

	private:
		void CalculateNormal(glm::vec3& normal, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2);
		void CalculateTangent(glm::vec3& tangent, 
			const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2,
			const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2);

		std::weak_ptr<Texture> LoadTexture(tinygltf::Model& tinyGLTFModel, ResourceHandler& resourceHandler, int index, bool isSRGB = false);
	};
}