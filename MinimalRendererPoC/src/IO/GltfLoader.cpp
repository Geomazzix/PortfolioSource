#include "MrpocPch.h"

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "IO/tiny_gltf.h"

#include <IO/GltfLoader.h>

#include <SceneManagement/Scene.h>
#include "SceneManagement/Node.h"

#include "SceneManagement/Nodes/Camera.h"
#include "SceneManagement/Nodes/Light.h"
#include "SceneManagement/Nodes/Model.h"

#include <Resources/ResourceHandler.h>
#include <Resources/Mesh.h>
#include <Resources/Texture.h>

#include "Graphics/Vertex.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/bundled/printf.h"

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include <glm/gtx/hash.hpp>

#include <memory>

namespace mrpoc
{
	GLTFLoader::GLTFLoader()
	{
	}

	GLTFLoader::~GLTFLoader()
	{
	}

	bool GLTFLoader::LoadGLTFIntoScene(ResourceHandler& resourceHandler, std::vector<mrpoc::Scene>& aSceneVectorToLoadScenesInto, std::string aPathToGLTFFile, bool flipNormals)
	{
		tinygltf::Model tinyGLTFModel;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool ret = loader.LoadASCIIFromFile(&tinyGLTFModel, &err, &warn, aPathToGLTFFile);
		//bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

		if (!warn.empty()) {
			printf("Warn: %s\n", warn.c_str());
		}

		if (!err.empty()) {
			printf("Err: %s\n", err.c_str());
		}

		if (!ret) {
			printf("Failed to parse glTF\n");
			return false;
		}

		std::vector<tinygltf::Node, std::allocator<tinygltf::Node>>& nodes = tinyGLTFModel.nodes;

		// gltf file may contain multiple scenes!
		for (int i = 0; i < tinyGLTFModel.scenes.size(); ++i)
		{
			tinygltf::Scene& sceneToLoad = tinyGLTFModel.scenes[i];
			aSceneVectorToLoadScenesInto.push_back(Scene());

			std::set<std::string> extensionBuffer;

			for (auto& extension : tinyGLTFModel.extensionsUsed)
			{
				extensionBuffer.insert(extension);
			}

			for (auto& extension : tinyGLTFModel.extensionsRequired)
			{
				extensionBuffer.insert(extension);
			}

			
			// load nodes of scene i
			for (int nodeIndex = 0; nodeIndex < sceneToLoad.nodes.size(); ++nodeIndex)
			{
				std::shared_ptr<Node> newLoadedNode = LoadNode(nullptr, resourceHandler, tinyGLTFModel, nodes, sceneToLoad, aSceneVectorToLoadScenesInto[i], extensionBuffer, sceneToLoad.nodes[nodeIndex]);
				// #TODO: think of a way to set parents and children
				aSceneVectorToLoadScenesInto[i].GetRoot()->AddChild(newLoadedNode);
			}

			if (aSceneVectorToLoadScenesInto[i].GetCameraDataSize() == 0)
			{
				// load default camera.

				std::shared_ptr<mrpoc::Camera> defaultCam = std::make_shared<mrpoc::Camera>();

				//#TODO: Camera width and height is not being loaded from somewhere. Agreed on having it 800 600 for now.
				//#TODO: Camera from gltf gives you yfov, current camera expects XFoV.
				defaultCam->Initialize(800, 600, 75, 0.1f, 100);
				std::shared_ptr<Node> defaultCamNode = std::make_shared<Node>();
				defaultCamNode->Translate(glm::vec3(-2, 0, 5));
				defaultCamNode->InitializeNode(CAMERA, aSceneVectorToLoadScenesInto[i].GetRoot());
				defaultCamNode->SetDataIndex(aSceneVectorToLoadScenesInto[i].AddCamera(defaultCam));
				defaultCamNode->LookAt(glm::vec3(0, 0, 0));
				aSceneVectorToLoadScenesInto[i].GetRoot()->AddChild(defaultCamNode);
			}
		}

		return true;
	}

	std::shared_ptr<mrpoc::Node> GLTFLoader::LoadNode(std::shared_ptr<mrpoc::Node> parent,
		ResourceHandler& resourceHandler, tinygltf::Model& tinyGLTFModel, std::vector<tinygltf::Node>& nodes,
		tinygltf::Scene& sceneToLoad, mrpoc::Scene& sceneToCacheInto, std::set<std::string>& extensions, int nodeIndex, bool flipNormals)
	{
		tinygltf::Node& nodeToLoad = nodes[nodeIndex];
		std::string nodeName = nodeToLoad.name;

		std::shared_ptr<Node> loadedNode = std::make_shared<Node>();
		loadedNode->SetName(nodeName);

		ENodeType loadedNodeType = ENodeType::EMPTY;
		// nodes their translation, rotation (is a quaternion) and scale are optional values. same for the matrix. be sure to supply default values when needed.

		glm::vec3 positionOfNode(0);
		glm::quat rotationOfNode = glm::quat_identity<float, glm::defaultp>();
		glm::vec3 scaleOfNode{ 1,1,1 };

		glm::mat4x4 localMatrixOfNode;

		if (nodeToLoad.matrix.size() != 0)
		{
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					localMatrixOfNode[i][j] = (float)nodeToLoad.matrix[i + (j * 4)];
				}
			}
			loadedNode->SetLocalTransform(localMatrixOfNode);
		}
		else
		{
			if (nodeToLoad.translation.size() != 0)
			{
				loadedNode->Translate(glm::vec3{ nodeToLoad.translation[0], nodeToLoad.translation[1], nodeToLoad.translation[2] });
			}
			if (nodeToLoad.rotation.size() != 0)
			{
				loadedNode->Rotate(glm::quat{ (float)nodeToLoad.rotation[3], (float)nodeToLoad.rotation[0], (float)nodeToLoad.rotation[1], (float)nodeToLoad.rotation[2] });
			}
			if (nodeToLoad.scale.size() != 0)
			{
				loadedNode->Scale(glm::vec3{ nodeToLoad.scale[0], nodeToLoad.scale[1], nodeToLoad.scale[2] });
			}
		}

		// check type of node
		// node is a camera
		if (nodeToLoad.camera != -1)
		{
			loadedNodeType = CAMERA;
			int cameraArrayIndex = nodeToLoad.camera;
			tinygltf::Camera& cameraOfNodeThatIsBeingLoaded = tinyGLTFModel.cameras[cameraArrayIndex];
			std::shared_ptr<mrpoc::Camera> newLoadedCamera = std::make_shared<mrpoc::Camera>();

			newLoadedCamera->Initialize(800, 600, 35.f,
				cameraOfNodeThatIsBeingLoaded.perspective.znear, cameraOfNodeThatIsBeingLoaded.perspective.zfar);
			loadedNode->SetDataIndex(sceneToCacheInto.AddCamera(newLoadedCamera));
		}
		// node is a mesh
		else if (nodeToLoad.mesh != -1)
		{
			loadedNodeType = MODEL;
			int meshArrayIndex = nodeToLoad.mesh;
			tinygltf::Mesh& tinyGLTFMesh = tinyGLTFModel.meshes[meshArrayIndex];
			std::shared_ptr<mrpoc::Model> newLoadedModel = std::make_shared<mrpoc::Model>();
			std::vector<MeshLoadStruct> meshPrimitivesToLoad;

			for (int i = 0; i < tinyGLTFMesh.primitives.size(); i++)
			{
				MeshLoadStruct newPrimitivetoLoad;

				switch (tinyGLTFMesh.primitives[i].mode)
				{
				case 0:
					newPrimitivetoLoad.m_drawMode = DrawModes::POINTS;
					spdlog::error("This draw mode is not implemented");
					break;

				case 1:
					newPrimitivetoLoad.m_drawMode = DrawModes::LINES;
					spdlog::error("This draw mode is not implemented");
					break;

				case 2:
					spdlog::error("This draw mode is not implemented");
					break;

				case 3:
					spdlog::error("This draw mode is not implemented");
					break;

				case 4:
					newPrimitivetoLoad.m_drawMode = DrawModes::TRIANGLES;
					break;

				case 5:
					spdlog::error("This draw mode is not implemented");
					break;

				case 6:
					spdlog::error("This draw mode is not implemented");
					break;

				default:
					newPrimitivetoLoad.m_drawMode = DrawModes::TRIANGLES;
					break;
				}

				newPrimitivetoLoad.indiceDataIndex = tinyGLTFMesh.primitives[i].indices;
				newPrimitivetoLoad.materialDataIndex = tinyGLTFMesh.primitives[i].material;

				if (tinyGLTFMesh.primitives[i].attributes.find("POSITION") != tinyGLTFMesh.primitives[i].attributes.end())
				{
					newPrimitivetoLoad.m_attributes[Attributes::POSITION] = tinyGLTFMesh.primitives[i].attributes.at("POSITION");
					newPrimitivetoLoad.m_attributesToLoad.push_back(Attributes::POSITION);
				}

				if (tinyGLTFMesh.primitives[i].attributes.find("NORMAL") != tinyGLTFMesh.primitives[i].attributes.end())
				{
					newPrimitivetoLoad.m_attributes[Attributes::NORMAL] = tinyGLTFMesh.primitives[i].attributes.at("NORMAL");
					newPrimitivetoLoad.m_attributesToLoad.push_back(Attributes::NORMAL);
				}
				
				if (tinyGLTFMesh.primitives[i].attributes.find("TEXCOORD_0") != tinyGLTFMesh.primitives[i].attributes.end())
				{
					newPrimitivetoLoad.m_attributes[Attributes::TEXCOORD_0] = tinyGLTFMesh.primitives[i].attributes.at("TEXCOORD_0");
					newPrimitivetoLoad.m_attributesToLoad.push_back(Attributes::TEXCOORD_0);
				}
				
				if (tinyGLTFMesh.primitives[i].attributes.find("TEXCOORD_1") != tinyGLTFMesh.primitives[i].attributes.end())
				{
					newPrimitivetoLoad.m_attributes[Attributes::TEXCOORD_1] = tinyGLTFMesh.primitives[i].attributes.at("TEXCOORD_1");
					newPrimitivetoLoad.m_attributesToLoad.push_back(Attributes::TEXCOORD_1);
				}

				if (tinyGLTFMesh.primitives[i].attributes.find("TANGENT") != tinyGLTFMesh.primitives[i].attributes.end())
				{
					newPrimitivetoLoad.m_attributes[Attributes::TANGENT] = tinyGLTFMesh.primitives[i].attributes.at("TANGENT");
					newPrimitivetoLoad.m_attributesToLoad.push_back(Attributes::TANGENT);
				}

				if (tinyGLTFMesh.primitives[i].attributes.find("COLOR_0") != tinyGLTFMesh.primitives[i].attributes.end())
				{
					newPrimitivetoLoad.m_attributes[Attributes::COLOR_0] = tinyGLTFMesh.primitives[i].attributes.at("COLOR_0");
					newPrimitivetoLoad.m_attributesToLoad.push_back(Attributes::COLOR_0);
				}
				if (tinyGLTFMesh.primitives[i].attributes.find("JOINTS_0") != tinyGLTFMesh.primitives[i].attributes.end())
				{
					newPrimitivetoLoad.m_attributes[Attributes::JOINTS_0] = tinyGLTFMesh.primitives[i].attributes.at("JOINTS_0");
					spdlog::critical("not implemented"); assert(false);
					newPrimitivetoLoad.m_attributesToLoad.push_back(Attributes::JOINTS_0);
				}
				if (tinyGLTFMesh.primitives[i].attributes.find("WEIGHTS_0") != tinyGLTFMesh.primitives[i].attributes.end())
				{
					newPrimitivetoLoad.m_attributes[Attributes::WEIGHTS_0] = tinyGLTFMesh.primitives[i].attributes.at("WEIGHTS_0");
					spdlog::critical("not implemented"); assert(false);
					newPrimitivetoLoad.m_attributesToLoad.push_back(Attributes::WEIGHTS_0);
				}

				std::shared_ptr<mrpoc::Mesh> newlyLoadedMesh = std::make_shared<mrpoc::Mesh>();

				newlyLoadedMesh->Bind();

				auto& accessors = tinyGLTFModel.accessors;

				// Load attributes
				for (Attributes& attr : newPrimitivetoLoad.m_attributesToLoad)
				{
					auto& accessorToLoadDataWith = accessors[newPrimitivetoLoad.m_attributes[attr]];
					auto& bufferView = tinyGLTFModel.bufferViews[accessorToLoadDataWith.bufferView];
					auto& bufferToLoadDataFrom = tinyGLTFModel.buffers[bufferView.buffer];

					uint32_t amountOfBytesToLoad = accessorToLoadDataWith.count;

					int numberOfComponents;
					switch (accessorToLoadDataWith.type)
					{
					case TINYGLTF_TYPE_SCALAR:
						numberOfComponents = 1;
						break;
					case TINYGLTF_TYPE_VEC2:
						numberOfComponents = 2;
						amountOfBytesToLoad *= 8;
						break;
					case TINYGLTF_TYPE_VEC3:
						numberOfComponents = 3;
						amountOfBytesToLoad *= (numberOfComponents * 4);
						break;
					case TINYGLTF_TYPE_VEC4:
						numberOfComponents = 4;
						amountOfBytesToLoad *= (numberOfComponents * 4);
						break;
					case TINYGLTF_TYPE_MAT2:
						numberOfComponents = 4;
						amountOfBytesToLoad *= (numberOfComponents * 4);
						break;
					case TINYGLTF_TYPE_MAT3:
						numberOfComponents = 9;
						amountOfBytesToLoad *= (numberOfComponents * 4);
						break;
					case TINYGLTF_TYPE_MAT4:
						numberOfComponents = 16;
						amountOfBytesToLoad *= (numberOfComponents * 4);
						break;
					}

					if (attr == Attributes::POSITION)
					{
						newlyLoadedMesh->SetVertexCount(bufferView.byteLength / sizeof(glm::vec3));
					}

					newlyLoadedMesh->BufferAttributeData(attr, amountOfBytesToLoad
						, bufferView.byteStride, numberOfComponents, &bufferToLoadDataFrom.data.at(bufferView.byteOffset + accessorToLoadDataWith.byteOffset)
						, accessorToLoadDataWith.componentType /* Need to make sure that this is the correct number! might only work for OpenGL */
						, GL_STATIC_DRAW);
				}		

				// load indices
				auto& accessorToLoadDataWith = accessors[newPrimitivetoLoad.indiceDataIndex];
				auto& bufferView = tinyGLTFModel.bufferViews[accessorToLoadDataWith.bufferView];
				auto& bufferToLoadDataFrom = tinyGLTFModel.buffers[bufferView.buffer];
				int elementSize = 0;
				switch (accessorToLoadDataWith.componentType)
				{
				case TINYGLTF_COMPONENT_TYPE_BYTE:
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
					newlyLoadedMesh->SetElementDataType(GL_BYTE);
					elementSize = 1;
					break;
				case TINYGLTF_COMPONENT_TYPE_SHORT:
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
					newlyLoadedMesh->SetElementDataType(GL_UNSIGNED_SHORT);
					elementSize = 2;
					break;
				case TINYGLTF_COMPONENT_TYPE_FLOAT:
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
					newlyLoadedMesh->SetElementDataType(GL_FLOAT);
					elementSize = 4;
					break;
				default:
					assert(false);
				}

				newlyLoadedMesh->SetIndiceCount(accessorToLoadDataWith.count);
				newlyLoadedMesh->BufferElementData(accessorToLoadDataWith.count * elementSize, &bufferToLoadDataFrom.data.at(bufferView.byteOffset + accessorToLoadDataWith.byteOffset), GL_STATIC_DRAW);

				newlyLoadedMesh->Unbind();

				// Load material; cache if new otherwise retrieve.
				int MaterialNr = tinyGLTFMesh.primitives[i].material;
				bool isPBRMaterial = false;
				std::shared_ptr<Material> meshMaterial;
				if (!resourceHandler.ContainsMaterial("Material_" + std::to_string(MaterialNr)))
				{
					meshMaterial = std::make_shared<Material>();
					tinygltf::Material MaterialToLoad = tinyGLTFModel.materials[MaterialNr];
					// load Material values

					meshMaterial->AlphaCutoff = MaterialToLoad.alphaCutoff;
					meshMaterial->AlphaMode = MaterialToLoad.alphaMode;
					meshMaterial->DoubleSided = MaterialToLoad.doubleSided;

					// Albedo
					if (MaterialToLoad.pbrMetallicRoughness.baseColorTexture.index != -1)
					{
						meshMaterial->AlbedoMap = LoadTexture(tinyGLTFModel,resourceHandler,MaterialToLoad.pbrMetallicRoughness.baseColorTexture.index,true);
						meshMaterial->TexCoords[static_cast<int>(ETextureType::Albedo)] = MaterialToLoad.pbrMetallicRoughness.baseColorTexture.texCoord;

						
					}
					else
					{
						meshMaterial->AlbedoFactor = glm::vec4(static_cast<float>(MaterialToLoad.pbrMetallicRoughness.baseColorFactor[0]),
							static_cast<float>(MaterialToLoad.pbrMetallicRoughness.baseColorFactor[1]),
							static_cast<float>(MaterialToLoad.pbrMetallicRoughness.baseColorFactor[2]),
							static_cast<float>(MaterialToLoad.pbrMetallicRoughness.baseColorFactor[3])
						);
					}
					// Metallic Roughness
					if (MaterialToLoad.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
					{
						meshMaterial->MetalnessRoughnessMap = LoadTexture(tinyGLTFModel, resourceHandler, MaterialToLoad.pbrMetallicRoughness.metallicRoughnessTexture.index);
						meshMaterial->TexCoords[static_cast<int>(ETextureType::MetalnessRoughness)] = MaterialToLoad.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;

						
					}
					else
					{
						meshMaterial->MetallicFactor = MaterialToLoad.pbrMetallicRoughness.metallicFactor;
						meshMaterial->RoughnessFactor = MaterialToLoad.pbrMetallicRoughness.roughnessFactor;
					}
					// Normal
					if (MaterialToLoad.normalTexture.index != -1) {
						meshMaterial->NormalMap = LoadTexture(tinyGLTFModel, resourceHandler, MaterialToLoad.normalTexture.index);
						meshMaterial->TexCoords[static_cast<int>(ETextureType::Normal)] = MaterialToLoad.normalTexture.texCoord;

						// scaledNormal = normalize((<sampled normal texture value>* 2.0 - 1.0) * vec3(<normal scale>, <normal scale>, 1.0))
						meshMaterial->NormalScale = static_cast<float>(MaterialToLoad.normalTexture.scale);
					}
					else
					{
						// normal map not supported for this material.
					}
					// Occlusion
					if (MaterialToLoad.occlusionTexture.index != -1)
					{
						meshMaterial->HasOcclusionMap = true;
						//Reason for commenting this out -> refer to the material.h.
						//meshMaterial->m_albedoMap =LoadTexture(tinyGLTFModel,resourceHandler,MaterialToLoad.occlusionTexture.index);
						//meshMaterial->m_texCoords[static_cast<int>(ETextureType::Occlusion)] = MaterialToLoad.occlusionTexture.texCoord;

						// occludedColor = lerp(color, color * <sampled occlusion texture value>, <occlusion strength>)
						//meshMaterial->m_occlusionStrength = static_cast<float>(MaterialToLoad.occlusionTexture.strength);
					}
					else
					{
						// occlusion map not supported for this material
					}

					// Emission
					if (MaterialToLoad.emissiveTexture.index != -1)
					{
						meshMaterial->EmissionMap = LoadTexture(tinyGLTFModel, resourceHandler, MaterialToLoad.emissiveTexture.index);
						meshMaterial->EmissionFactor = glm::vec3(
							MaterialToLoad.emissiveFactor[0], 
							MaterialToLoad.emissiveFactor[1], 
							MaterialToLoad.emissiveFactor[2]);
						meshMaterial->TexCoords[static_cast<int>(ETextureType::Emission)] = MaterialToLoad.emissiveTexture.texCoord;
					}
					else
					{
						// emissive map not supported for this material
					}
					resourceHandler.CacheMaterial("Material_" + std::to_string(MaterialNr), meshMaterial);
				}
				else
				{
					meshMaterial = resourceHandler.GetMaterial("Material_" + std::to_string(MaterialNr)).lock();
				}

				meshMaterial->FlipNormals = flipNormals;
				newlyLoadedMesh->SetMaterial(meshMaterial);
				isPBRMaterial = meshMaterial->MetalnessRoughnessMap.lock() != nullptr;
				newlyLoadedMesh->SetShader(resourceHandler.GetShader("resources/Shaders/vs/diffusePBR.vert").lock());
				/*newlyLoadedMesh->SetShader(isPBRMaterial
					? resourceHandler.GetShader("resources/Shaders/vs/diffusePBR.vert").lock()
					: resourceHandler.GetShader("resources/Shaders/vs/diffuseWithNormalMap.vert").lock());*/

				// add loaded mesh to model class.
				newLoadedModel->AddMesh(newlyLoadedMesh);
			}

			loadedNode->SetDataIndex(sceneToCacheInto.AddModel(newLoadedModel));
		}
		// node is a skin
		else if (nodeToLoad.skin != -1)
		{
			int skinArrayIndex = nodeToLoad.skin;
			tinygltf::Skin& skinOfNodeThatIsBeingLoaded = tinyGLTFModel.skins[skinArrayIndex];
			std::shared_ptr<mrpoc::Light> newLoadedLight;

			spdlog::info("Ignoring skin, as this is not implemented.");
		}
		else
		{
			for (auto& extension : extensions)
			{
				if (nodeToLoad.extensions.find(extension) == nodeToLoad.extensions.end())
					continue;

				tinygltf::Value extensionValue = nodeToLoad.extensions[extension];
				if (extensionValue.Type() != 0)
				{
					switch (extensionValue.Type())
					{
					case EExtensionValues::KHR_LIGHT:
					{
						loadedNodeType = ENodeType::LIGHT;
						tinygltf::Light tinyGltfLight = tinyGLTFModel.lights[extensionValue.Get("light").GetNumberAsInt()];

						//Babylon exporter fix - when the color is white the exporter ignores it.
						glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);
						if(tinyGltfLight.color.capacity() > 0)
						{
							color = glm::vec3(
								static_cast<float>(tinyGltfLight.color[0]),
								static_cast<float>(tinyGltfLight.color[1]),
								static_cast<float>(tinyGltfLight.color[2]));
						}

						if (tinyGltfLight.type == "point")
						{
							float range = tinyGltfLight.range;
							if (range == 0.0f) range = 1.0f;

							std::shared_ptr<PointLight> pointLight = std::make_shared<PointLight>(PointLight{
								color,
								static_cast<float>(tinyGltfLight.intensity),
								ELightType::POINT,
								static_cast<float>(range)
								});

							loadedNode->SetDataIndex(sceneToCacheInto.AddLight(pointLight));
						}
						else if (tinyGltfLight.type == "directional")
						{
							std::shared_ptr<Light> directionalLight = std::make_shared<Light>(Light{
								color,
								static_cast<float>(tinyGltfLight.intensity),
								ELightType::DIRECTIONAL
								});

							loadedNode->SetDataIndex(sceneToCacheInto.AddLight(directionalLight));
						}
						else if (tinyGltfLight.type == "spot")
						{
							std::shared_ptr<SpotLight> spotLight = std::make_shared<SpotLight>(SpotLight{
								color,
								static_cast<float>(tinyGltfLight.intensity),
								ELightType::SPOT,
								static_cast<float>(tinyGltfLight.spot.innerConeAngle),
								static_cast<float>(tinyGltfLight.spot.outerConeAngle)
								});

							loadedNode->SetDataIndex(sceneToCacheInto.AddLight(spotLight));
						}
						break;
					}
					}
				}
			}
		}

		if (parent)
		{
			loadedNode->InitializeNode(loadedNodeType, parent);
		}
		else
		{
			loadedNode->InitializeNode(loadedNodeType, sceneToCacheInto.GetRoot());
		}

		for (int childToLoadIndex = 0; childToLoadIndex < nodeToLoad.children.size(); ++childToLoadIndex)
		{
			loadedNode->AddChild(LoadNode(loadedNode, resourceHandler, tinyGLTFModel, nodes, sceneToLoad, sceneToCacheInto, extensions, nodeToLoad.children[childToLoadIndex]));
		}

		return loadedNode;
	}

	void GLTFLoader::CalculateNormal(glm::vec3& normal, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2)
	{
		glm::vec3 v10;
		v10[0] = p1[0] - p0[0];
		v10[1] = p1[1] - p0[1];
		v10[2] = p1[2] - p0[2];

		glm::vec3 v20;
		v20[0] = p2[0] - p0[0];
		v20[1] = p2[1] - p0[1];
		v20[2] = p2[2] - p0[2];

		normal[0] = v20[1] * v10[2] - v20[2] * v10[1];
		normal[1] = v20[2] * v10[0] - v20[0] * v10[2];
		normal[2] = v20[0] * v10[1] - v20[1] * v10[0];

		const float len2 = normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2];
		if (len2 > 0.0f) {
			const float len = sqrtf(len2);
			normal[0] /= len;
			normal[1] /= len;
		}
	}

	void GLTFLoader::CalculateTangent(glm::vec3& tangent, 
		const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, 
		const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2)
	{
		glm::vec3 edge1 = p1 - p0;
		glm::vec3 edge2 = p2 - p0;
		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	}

	std::weak_ptr<Texture> GLTFLoader::LoadTexture(tinygltf::Model& tinyGLTFModel, ResourceHandler& resourceHandler, int index, bool isSRGB)
	{
		if (!resourceHandler.ContainsTexture("Texture_" + std::to_string(index)))
		{	// Load texture 
			tinygltf::Texture tinyGltfTexture = tinyGLTFModel.textures[index];
			tinygltf::Image tinyGltfImage = tinyGLTFModel.images[tinyGltfTexture.source];

			Image image =
			{
				tinyGltfImage.uri,
				tinyGltfImage.width,
				tinyGltfImage.height,
				tinyGltfImage.component,
				tinyGltfImage.bits
			};

			Sampler sampler =
			{
				GL_LINEAR,
				GL_LINEAR,
				GL_REPEAT,
				GL_REPEAT,
				GL_REPEAT,
			};

			if (tinyGltfTexture.sampler != -1)
			{
				tinygltf::Sampler tinyGltfSampler = tinyGLTFModel.samplers[tinyGltfTexture.sampler];
				sampler =
				{
					tinyGltfSampler.minFilter > -1 ? tinyGltfSampler.minFilter : GL_LINEAR_MIPMAP_LINEAR,
					tinyGltfSampler.magFilter > -1 ? tinyGltfSampler.magFilter : GL_LINEAR,
					tinyGltfSampler.wrapS > -1 ? tinyGltfSampler.wrapS : GL_REPEAT,
					tinyGltfSampler.wrapT > -1 ? tinyGltfSampler.wrapT : GL_REPEAT,
					tinyGltfSampler.wrapR > -1 ? tinyGltfSampler.wrapR : GL_REPEAT,
				};
			}

			std::shared_ptr<Texture> texture = std::make_shared<Texture>();
			std::string textureName = "Texture_" + std::to_string(index);
			std::string samplerName = "Sampler_" + std::to_string(tinyGltfTexture.sampler);
			if(tinyGltfTexture.sampler != -1)
			{
				resourceHandler.CacheSampler(samplerName, std::make_shared<Sampler>(sampler));
			}
			int format = isSRGB? GL_SRGB8_ALPHA8 : GL_RGBA;
			texture->Initialize(image,(tinyGltfTexture.sampler != -1 ? resourceHandler.GetSampler(samplerName) : std::weak_ptr<mrpoc::Sampler>()), std::move(tinyGltfImage.image), textureName, format);
			resourceHandler.CacheTexture(textureName, texture);
		}
		
		return resourceHandler.GetTexture("Texture_" + std::to_string(index));
	}

}