#include "pch.h"

#include "IO/GLTFModelBuilder.h"
#include "Core/EngineCore.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include <tinygltf/tiny_gltf.h>

#include "CoreRenderAPI/Components/RenderComponents.h"
#include "CoreRenderAPI/RenderAPI.h"

namespace Frac
{
	GLTF_ModelBuilder::GLTF_ModelBuilder(Frac::Renderer& renderSystem, ResourceManager& resourceManager, const std::string& filePath, bool loadUsingBinary, bool flipNormals) :
		ModelBuilder(renderSystem, resourceManager, filePath),
		m_LoadUsingBinary(loadUsingBinary),
		m_filePath(filePath),
		m_flipNormals(flipNormals)
	{
	}

	bool GLTF_ModelBuilder::LoadModelFromAPI()
	{
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;
		bool ret = m_LoadUsingBinary
			? loader.LoadBinaryFromFile(&m_tinyGLTFModel, &err, &warn, std::string(m_ModelDirectory + m_ModelName + ".glb"))
			: loader.LoadASCIIFromFile(&m_tinyGLTFModel, &err, &warn, std::string(m_ModelDirectory + m_ModelName + ".gltf"));

		if (!warn.empty()) {
			LOGWARNING("%s\n", warn.c_str());
		}

		if (!err.empty()) {
			LOGERROR("%s\n", err.c_str());
		}

		if (!ret) {
			LOGINFO("Failed to parse glTF\n");
			return false;
		}

		return true;
	}

	bool GLTF_ModelBuilder::ParseModelData()
	{
		std::vector<tinygltf::Node, std::allocator<tinygltf::Node>>& nodes = m_tinyGLTFModel.nodes;
		GLTFBuildNode& rootNode = m_resourceManager.LoadGltfModelNode(m_filePath);
		rootNode.Name = m_filePath;

		for (int i = 0; i < m_tinyGLTFModel.scenes.size(); ++i)
		{
			std::set<std::string> extensionBuffer;
			for (auto& extension : m_tinyGLTFModel.extensionsUsed)
			{
				extensionBuffer.insert(extension);
			}

			for (auto& extension : m_tinyGLTFModel.extensionsRequired)
			{
				extensionBuffer.insert(extension);
			}

			GLTFBuildNode& sceneNode = m_resourceManager.LoadGltfModelNode(m_tinyGLTFModel.scenes[i].name + "_" + std::to_string(i) + "_" + m_ModelName);
			sceneNode.Name = m_tinyGLTFModel.scenes[i].name + "_" + std::to_string(i) + "_" + m_ModelName;

			tinygltf::Scene& modelToLoad = m_tinyGLTFModel.scenes[i];
			for (int nodeIndex = 0; nodeIndex < modelToLoad.nodes.size(); ++nodeIndex)
			{
				LoadNode(sceneNode, nodes, extensionBuffer, modelToLoad.nodes[nodeIndex]);
			}
			rootNode.Children.push_back(sceneNode);
		}
		return true;
	}

	void GLTF_ModelBuilder::LoadNode(GLTFBuildNode& parent, std::vector<tinygltf::Node>& nodes, std::set<std::string>& extensions, int nodeIndex)
	{
		tinygltf::Node& nodeToLoad = nodes[nodeIndex];
		GLTFBuildNode node;
		node.Name = nodes[nodeIndex].name;
		node.Transform = LoadTransform(nodeToLoad);

		if (nodeToLoad.camera != -1)
		{
			LOGWARNING("Trying to load in a camera from the model file");
			//loadedEntity = LoadCamera(nodeToLoad,transformOfNode);
		}
		else if (nodeToLoad.mesh != -1)
		{
			LoadMesh(node, nodeToLoad);
		}
		else if (nodeToLoad.skin != -1)
		{
			LoadSkin(nodeToLoad);
		}
		else
		{
			if (nodeToLoad.extensions.size() > 0)
			{
				for (auto& extension : extensions)
				{
					if (nodeToLoad.extensions.find(extension) == nodeToLoad.extensions.end())
					{
						continue;
					}

					tinygltf::Value extensionValue = nodeToLoad.extensions[extension];
					if (extensionValue.Type() != 0)
					{
						switch (extensionValue.Type())
						{
						case EExtensionValues::KHR_LIGHT:
						{
							LoadLight(node, nodeToLoad, extensionValue);
							break;
						}
						}
					}
				}
			}
		}

		for (int childToLoadIndex = 0; childToLoadIndex < nodeToLoad.children.size(); ++childToLoadIndex)
		{
			LoadNode(node, nodes, extensions, nodeToLoad.children[childToLoadIndex]);
		}
		parent.Children.push_back(node);
	}

	void GLTF_ModelBuilder::AddDefaultCamera(GLTFBuildNode& sceneRoot)
	{
		//#TODO: Camera width and height is not being loaded from somewhere. Agreed on having it 1820 768 for now.
		TOR::Transform defaultCameraTransform;
		defaultCameraTransform.Position = glm::vec3(-2, 0, 5);
		defaultCameraTransform.ModelTransformData = glm::lookAtRH(
			defaultCameraTransform.Position,
			glm::vec3(0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);

		std::string entityName = std::string("DEFAULT_CAMERA");
		if (m_resourceManager.IsCameraLoaded(entityName))
		{
			GLTFBuildNode node;
			node.Name = entityName;
			node.Transform = defaultCameraTransform;
			node.Type = EGLTFModelNode::CAMERA;
			node.Data = &m_resourceManager.GetCamera(entityName);
			sceneRoot.Children.push_back(node);

			LOGWARNING("Loaded the default camera with a duplicated name!");
		}
		else
		{
			//#TODO: The camera component creation is a duplicate from the GLTF_ModelBuilder::LoadCamera method. Add a factory method somewhere along the line.
			float fovInDeg = 70.0f;
			float zNear = 0.01f;
			float zFar = 1000.0f;
			float screenWidth = 1280.0f;
			float screenHeight = 768.0f;
			float aspectRatio = screenHeight / screenHeight;

			TOR::Camera& defaultCam = m_resourceManager.LoadCamera(entityName);

			defaultCam.ProjectionMat = glm::perspectiveRH(glm::radians(fovInDeg), aspectRatio, zNear, zFar);
			defaultCam.FovInDeg = fovInDeg;
			defaultCam.ZNear = zNear;
			defaultCam.ZFar = zFar;
			defaultCam.AspectRatio = aspectRatio;
			defaultCam.ScreenWidth = static_cast<int>(screenWidth);
			defaultCam.ScreenHeight = static_cast<int>(screenHeight);
			defaultCam.CameraType = TOR::ECameraType::Perspective;
			defaultCam.IsDirty = false;

			//#TODO: Camera from glTF gives you yfov, current camera expects XFoV.
			GLTFBuildNode node;
			node.Name = entityName;
			node.Type = EGLTFModelNode::CAMERA;
			node.Transform = defaultCameraTransform;
			node.Data = &defaultCam;
			sceneRoot.Children.push_back(node);
		}
	}

	TOR::Transform GLTF_ModelBuilder::LoadTransform(tinygltf::Node& nodeToLoad)
	{
		TOR::Transform transform;

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
					localMatrixOfNode[i][j] = static_cast<float>(nodeToLoad.matrix[i + (j * 4)]);
				}
			}

			//#TODO: Unit test the decompose, this was implemented in the PoC, but did not work as intended.
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(localMatrixOfNode, transform.Scale, transform.Orientation, transform.Position, skew, perspective);
			transform.ModelTransformData = glm::identity<glm::mat4x4>();
		}
		else
		{
			if (nodeToLoad.translation.size() != 0)
			{
				transform.Position =
				{
					static_cast<float>(nodeToLoad.translation[0]),
					static_cast<float>(nodeToLoad.translation[1]),
					static_cast<float>(nodeToLoad.translation[2])
				};
			}
			else
			{
				transform.Position =
				{
					0,0,0
				};
			}
			if (nodeToLoad.rotation.size() != 0)
			{
				transform.Orientation =
				{
					static_cast<float>(nodeToLoad.rotation[3]),
					static_cast<float>(nodeToLoad.rotation[0]),
					static_cast<float>(nodeToLoad.rotation[1]),
					static_cast<float>(nodeToLoad.rotation[2])
				};
			}
			else
			{
				transform.Orientation =
				{
					1,0,0,0
				};
			}
			if (nodeToLoad.scale.size() != 0)
			{
				transform.Scale =
				{
					static_cast<float>(nodeToLoad.scale[0]),
					static_cast<float>(nodeToLoad.scale[1]),
					static_cast<float>(nodeToLoad.scale[2])
				};
			}
			else
			{
				transform.Scale =
				{
					1,1,1
				};
			}
		}
		return transform;
	}

	void GLTF_ModelBuilder::LoadCamera(tinygltf::Node& nodeToLoad)
	{
		std::string entityName = std::string(m_ModelName + "_CAMERA_" + std::to_string(nodeToLoad.camera));
		if (m_resourceManager.IsCameraLoaded(entityName))
		{
			GLTFBuildNode node;
			node.Name = entityName;
			node.Type = EGLTFModelNode::CAMERA;
			node.Data = &m_resourceManager.GetCamera(entityName);

			LOGWARNING("Loaded in a camera with a duplicated name!");
		}
		else
		{
			tinygltf::Camera& loadedCamera = m_tinyGLTFModel.cameras[nodeToLoad.camera];
			TOR::Camera& camera = m_resourceManager.LoadCamera(entityName);

			if (loadedCamera.type == "PERSPECTIVE")
			{
				float screenWidth = 1280.0f;
				float screenHeight = 768.0f;

				camera.FovInDeg = 35.0f;
				camera.ZNear = loadedCamera.perspective.znear;
				camera.ZFar = loadedCamera.perspective.zfar;
				camera.AspectRatio = screenWidth / screenHeight;

				camera.ScreenWidth = static_cast<int>(screenWidth);
				camera.ScreenHeight = static_cast<int>(screenHeight);

				camera.CameraType = TOR::ECameraType::Perspective;
				camera.ProjectionMat = glm::perspectiveRH(glm::radians(camera.FovInDeg), camera.AspectRatio, camera.ZNear, camera.ZFar);
			}
			else //Orthographic
			{
				//#TODO: Implement orthographic camera input.
			}

			GLTFBuildNode cameraNode;
			cameraNode.Name = entityName;
			cameraNode.Type = EGLTFModelNode::CAMERA;
			cameraNode.Data = &camera;
		}
	}

	void GLTF_ModelBuilder::LoadMesh(GLTFBuildNode& modelNode, tinygltf::Node& nodeToLoad)
	{
		TOR::RenderAPI& renderAPI = m_renderSystem.GetRenderAPI();
		tinygltf::Mesh& tinyGLTFMesh = m_tinyGLTFModel.meshes[nodeToLoad.mesh];
		bool hasVertexColors = false;

		for (int i = 0; i < tinyGLTFMesh.primitives.size(); i++)
		{
			hasVertexColors = false;
			GLTFBuildNode meshNode;
			std::string meshName = std::string(m_ModelName + "_MESH_" + std::to_string(nodeToLoad.mesh) + "_" + std::to_string(i));

			if (!m_resourceManager.IsMeshLoaded(meshName))
			{
				MeshLoadStruct newPrimitivetoLoad;
				switch (tinyGLTFMesh.primitives[i].mode)
				{
				case 0:
					newPrimitivetoLoad.DrawMode = EDrawModes::POINTS;
					spdlog::error("This draw mode is not implemented");
					break;

				case 1:
					newPrimitivetoLoad.DrawMode = EDrawModes::LINES;
					spdlog::error("This draw mode is not implemented");
					break;

				case 2:
					spdlog::error("This draw mode is not implemented");
					break;

				case 3:
					spdlog::error("This draw mode is not implemented");
					break;

				case 4:
					newPrimitivetoLoad.DrawMode = EDrawModes::TRIANGLES;
					break;

				case 5:
					spdlog::error("This draw mode is not implemented");
					break;

				case 6:
					spdlog::error("This draw mode is not implemented");
					break;

				default:
					newPrimitivetoLoad.DrawMode = GLTF_ModelBuilder::EDrawModes::TRIANGLES;
					break;
				}

				newPrimitivetoLoad.IndiceDataIndex = tinyGLTFMesh.primitives[i].indices;
				newPrimitivetoLoad.MaterialDataIndex = tinyGLTFMesh.primitives[i].material;

				if (tinyGLTFMesh.primitives[i].attributes.find("POSITION") != tinyGLTFMesh.primitives[i].attributes.end())
				{
					newPrimitivetoLoad.Attributes[TOR::EVertexAttributes::POSITION] = tinyGLTFMesh.primitives[i].attributes.at("POSITION");
					newPrimitivetoLoad.AttributesToLoad.push_back(TOR::EVertexAttributes::POSITION);
				}

				if (tinyGLTFMesh.primitives[i].attributes.find("NORMAL") != tinyGLTFMesh.primitives[i].attributes.end())
				{
					newPrimitivetoLoad.Attributes[TOR::EVertexAttributes::NORMAL] = tinyGLTFMesh.primitives[i].attributes.at("NORMAL");
					newPrimitivetoLoad.AttributesToLoad.push_back(TOR::EVertexAttributes::NORMAL);
				}

				if (tinyGLTFMesh.primitives[i].attributes.find("TANGENT") != tinyGLTFMesh.primitives[i].attributes.end())
				{
					newPrimitivetoLoad.Attributes[TOR::EVertexAttributes::TANGENT] = tinyGLTFMesh.primitives[i].attributes.at("TANGENT");
					newPrimitivetoLoad.AttributesToLoad.push_back(TOR::EVertexAttributes::TANGENT);
				}

				if (tinyGLTFMesh.primitives[i].attributes.find("TEXCOORD_0") != tinyGLTFMesh.primitives[i].attributes.end())
				{
					newPrimitivetoLoad.Attributes[TOR::EVertexAttributes::TEXCOORD_0] = tinyGLTFMesh.primitives[i].attributes.at("TEXCOORD_0");
					newPrimitivetoLoad.AttributesToLoad.push_back(TOR::EVertexAttributes::TEXCOORD_0);
				}

				if (tinyGLTFMesh.primitives[i].attributes.find("COLOR_0") != tinyGLTFMesh.primitives[i].attributes.end())
				{
					newPrimitivetoLoad.Attributes[TOR::EVertexAttributes::COLOR_0] = tinyGLTFMesh.primitives[i].attributes.at("COLOR_0");
					newPrimitivetoLoad.AttributesToLoad.push_back(TOR::EVertexAttributes::COLOR_0);
					hasVertexColors = true;
				}

				if (tinyGLTFMesh.primitives[i].attributes.find("TEXCOORD_1") != tinyGLTFMesh.primitives[i].attributes.end())
				{
					newPrimitivetoLoad.Attributes[TOR::EVertexAttributes::TEXCOORD_1] = tinyGLTFMesh.primitives[i].attributes.at("TEXCOORD_1");
					newPrimitivetoLoad.AttributesToLoad.push_back(TOR::EVertexAttributes::TEXCOORD_1);
				}

				if (tinyGLTFMesh.primitives[i].attributes.find("JOINTS_0") != tinyGLTFMesh.primitives[i].attributes.end())
				{
					newPrimitivetoLoad.Attributes[TOR::EVertexAttributes::JOINTS_0] = tinyGLTFMesh.primitives[i].attributes.at("JOINTS_0");
					spdlog::critical("not implemented"); assert(false);
					newPrimitivetoLoad.AttributesToLoad.push_back(TOR::EVertexAttributes::JOINTS_0);
				}

				if (tinyGLTFMesh.primitives[i].attributes.find("WEIGHTS_0") != tinyGLTFMesh.primitives[i].attributes.end())
				{
					newPrimitivetoLoad.Attributes[TOR::EVertexAttributes::WEIGHTS_0] = tinyGLTFMesh.primitives[i].attributes.at("WEIGHTS_0");
					spdlog::critical("not implemented"); assert(false);
					newPrimitivetoLoad.AttributesToLoad.push_back(TOR::EVertexAttributes::WEIGHTS_0);
				}

				//Create the mesh.
				TOR::Mesh& meshComponent = m_resourceManager.LoadMesh(meshName);
				renderAPI.CreateMesh(meshComponent);

				// Load attributes
				auto& accessors = m_tinyGLTFModel.accessors;
				for (TOR::EVertexAttributes& attr : newPrimitivetoLoad.AttributesToLoad)
				{
					auto& accessorToLoadDataWith = accessors[newPrimitivetoLoad.Attributes[attr]];
					auto& bufferView = m_tinyGLTFModel.bufferViews[accessorToLoadDataWith.bufferView];
					auto& bufferToLoadDataFrom = m_tinyGLTFModel.buffers[bufferView.buffer];

					int typeByteLength;
					switch (static_cast<TOR::EValueTypes>(accessorToLoadDataWith.componentType))
					{
					case TOR::EValueTypes::BYTE:
					case TOR::EValueTypes::UNSIGNED_BYTE:
						typeByteLength = 1;
						break;
					case TOR::EValueTypes::SHORT:
					case TOR::EValueTypes::UNSIGNED_SHORT:
						typeByteLength = 2;
						break;
					case TOR::EValueTypes::INT:
					case TOR::EValueTypes::UNSIGNED_INT:
					case TOR::EValueTypes::FLOAT:
						typeByteLength = 4;
						break;
					default:
						FRAC_ASSERT(false, "Unexpected valuetype for a buffer in the GLTF file found!");
					}

					int numberOfComponents;
					switch (accessorToLoadDataWith.type)
					{
					case TINYGLTF_TYPE_SCALAR:
						numberOfComponents = 1;
						break;
					case TINYGLTF_TYPE_VEC2:
						numberOfComponents = 2;
						break;
					case TINYGLTF_TYPE_VEC3:
						numberOfComponents = 3;
						break;
					case TINYGLTF_TYPE_VEC4:
						numberOfComponents = 4;
						break;
					case TINYGLTF_TYPE_MAT2:
						numberOfComponents = 4;
						break;
					case TINYGLTF_TYPE_MAT3:
						numberOfComponents = 9;
						break;
					case TINYGLTF_TYPE_MAT4:
						numberOfComponents = 16;
						break;
					default:
						FRAC_ASSERT(false, "Unexpected  accessorToLoadDataWith type!");
					}

					if (attr == TOR::EVertexAttributes::POSITION)
					{
						meshComponent.VertexCount = bufferView.byteLength / sizeof(glm::vec3);
					}

					uint32_t amountOfBytesToLoad = accessorToLoadDataWith.count * (numberOfComponents * typeByteLength);
					renderAPI.BufferAttributeData(
						meshComponent.Id,
						attr,
						amountOfBytesToLoad,
						bufferView.byteStride,
						accessorToLoadDataWith.normalized,
						numberOfComponents,
						&bufferToLoadDataFrom.data.at(bufferView.byteOffset + accessorToLoadDataWith.byteOffset),
						static_cast<TOR::EValueTypes>(accessorToLoadDataWith.componentType), //Need to make sure that this is the correct number! might only work for OpenGL
						TOR::EDrawTypes::STATIC_DRAW
					);
				}

				// load indices
				auto& accessorToLoadDataWith = accessors[newPrimitivetoLoad.IndiceDataIndex];
				auto& bufferView = m_tinyGLTFModel.bufferViews[accessorToLoadDataWith.bufferView];
				auto& bufferToLoadDataFrom = m_tinyGLTFModel.buffers[bufferView.buffer];

				int elementSize = 0;
				switch (accessorToLoadDataWith.componentType)
				{
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
					meshComponent.ElementDataType = TOR::ETorDataType::UBYTE;
					elementSize = 1;
					break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
					meshComponent.ElementDataType = TOR::ETorDataType::USHORT;
					elementSize = 2;
					break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
					meshComponent.ElementDataType = TOR::ETorDataType::UINT;
					elementSize = 4;
					break;
				default:
					assert(false);
				}

				meshComponent.IndiceCount = accessorToLoadDataWith.count;
				renderAPI.BufferElementData(
					meshComponent.Id,
					accessorToLoadDataWith.count * elementSize,
					&bufferToLoadDataFrom.data.at(bufferView.byteOffset + accessorToLoadDataWith.byteOffset),
					elementSize,
					TOR::EDrawTypes::DYNAMIC_DRAW
				);

				renderAPI.UnbindMesh(meshComponent.Id);
				meshNode.Name = meshName;
				meshNode.Data = &meshComponent;
				meshNode.Type = EGLTFModelNode::MESH;
				modelNode.Children.push_back(meshNode);
			}
			else
			{
				GLTFBuildNode mesh;
				mesh.Name = meshName;
				mesh.Type = EGLTFModelNode::MESH;
				mesh.Data = &m_resourceManager.GetMesh(meshName);
				modelNode.Children.push_back(mesh);
			}

			//Load material; cache if new otherwise retrieve.
			GLTFBuildNode materialNode;
			int MaterialNr = tinyGLTFMesh.primitives[i].material;
			std::string materialName = m_ModelName + "_MATERIAL_" + std::to_string(MaterialNr);

			if (!m_resourceManager.IsMaterialLoaded(materialName))
			{
				std::string textureName;
				TOR::Material& materialComponent = m_resourceManager.LoadMaterial(materialName);
				tinygltf::Material MaterialToLoad = m_tinyGLTFModel.materials[MaterialNr];

				//load Material values
				if (MaterialToLoad.alphaMode == "OPAQUE")
				{
					materialComponent.AlphaMode = TOR::EAlphaMode::OPAQUE_MODE;
				}
				else if (MaterialToLoad.alphaMode == "BLEND")
				{
					materialComponent.AlphaMode = TOR::EAlphaMode::BLEND_MODE;
				}
				else if (MaterialToLoad.alphaMode == "MASK")
				{
					//Defaulted to 0.5, only needs to be be set when the mask is applied.
					materialComponent.AlphaMode = TOR::EAlphaMode::MASK_MODE;
					materialComponent.AlphaCutoff = MaterialToLoad.alphaCutoff;
				}
				else
				{
					LOGWARNING("'%s' is an invalid alpha mode! OPAQUE will be used.");
					materialComponent.AlphaMode = TOR::EAlphaMode::OPAQUE_MODE;
				}

				//Double sided planes.
				materialComponent.DoubleSided = MaterialToLoad.doubleSided;

				//Albedo
				if (MaterialToLoad.pbrMetallicRoughness.baseColorTexture.index != -1)
				{
					GLTFBuildNode textureNode = LoadTexture(MaterialToLoad.pbrMetallicRoughness.baseColorTexture.index, true);
					TOR::Texture* albedoTexture = reinterpret_cast<TOR::Texture*>(textureNode.Data);
					albedoTexture->TextureType = TOR::ETextureType::ALBEDO;
					materialComponent.TexCoords[TOR::ETextureType::ALBEDO] = MaterialToLoad.pbrMetallicRoughness.baseColorTexture.texCoord;
					materialComponent.TexturesMaps[TOR::ETextureType::ALBEDO] = albedoTexture;
				}
				else
				{
					materialComponent.AlbedoFactor = glm::vec4(
						static_cast<float>(MaterialToLoad.pbrMetallicRoughness.baseColorFactor[0]),
						static_cast<float>(MaterialToLoad.pbrMetallicRoughness.baseColorFactor[1]),
						static_cast<float>(MaterialToLoad.pbrMetallicRoughness.baseColorFactor[2]),
						static_cast<float>(MaterialToLoad.pbrMetallicRoughness.baseColorFactor[3])
					);
				}

				//Metallic Roughness
				if (MaterialToLoad.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
				{
					GLTFBuildNode textureNode = LoadTexture(MaterialToLoad.pbrMetallicRoughness.metallicRoughnessTexture.index);
					TOR::Texture* metallicRoughnessTexture = reinterpret_cast<TOR::Texture*>(textureNode.Data);
					metallicRoughnessTexture->TextureType = TOR::ETextureType::METALROUGHNESS;
					materialComponent.TexCoords[static_cast<int>(TOR::ETextureType::METALROUGHNESS)] = MaterialToLoad.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;
					materialComponent.TexturesMaps[TOR::ETextureType::METALROUGHNESS] = metallicRoughnessTexture;
				}
				else
				{
					//LOGINFO("No PBR texture found, using the predefined Metallic and roughness factors.");
					materialComponent.MetallicFactor = MaterialToLoad.pbrMetallicRoughness.metallicFactor;
					materialComponent.RoughnessFactor = MaterialToLoad.pbrMetallicRoughness.roughnessFactor;
				}

				//Normal
				if (MaterialToLoad.normalTexture.index != -1)
				{
					GLTFBuildNode textureNode = LoadTexture(MaterialToLoad.normalTexture.index);
					TOR::Texture* normalTexture = reinterpret_cast<TOR::Texture*>(textureNode.Data);
					normalTexture->TextureType = TOR::ETextureType::NORMAL;
					materialComponent.TexCoords[static_cast<int>(TOR::ETextureType::NORMAL)] = MaterialToLoad.normalTexture.texCoord;

					//scaledNormal = normalize((<sampled normal texture value>* 2.0 - 1.0) * vec3(<normal scale>, <normal scale>, 1.0))
					materialComponent.NormalScale = static_cast<float>(MaterialToLoad.normalTexture.scale);
					materialComponent.TexturesMaps[TOR::ETextureType::NORMAL] = normalTexture;
					materialComponent.HasNormalMap = true;
				}
				else
				{
					materialComponent.HasNormalMap = false;
					LOGWARNING("No normal map found on model " + m_filePath);
				}

				//Occlusion
				if (MaterialToLoad.occlusionTexture.index != -1)
				{
					materialComponent.HasOcclusionMap = true;
				}
				else
				{
					LOGWARNING("Occlusion map is not provided by material and will therefor not be used.");
				}

				//Emission
				if (MaterialToLoad.emissiveTexture.index != -1)
				{
					GLTFBuildNode textureNode = LoadTexture(MaterialToLoad.emissiveTexture.index);
					TOR::Texture* emissionTexture = reinterpret_cast<TOR::Texture*>(textureNode.Data);
					emissionTexture->TextureType = TOR::ETextureType::EMISSION;
					materialComponent.EmissionFactor = glm::vec3(MaterialToLoad.emissiveFactor[0], MaterialToLoad.emissiveFactor[1], MaterialToLoad.emissiveFactor[2]);
					materialComponent.TexCoords[static_cast<int>(TOR::ETextureType::EMISSION)] = MaterialToLoad.emissiveTexture.texCoord;
					materialComponent.TexturesMaps[TOR::ETextureType::EMISSION] = emissionTexture;
				}

				materialComponent.FlipNormals = m_flipNormals;
				materialNode.Name = materialName;
				materialNode.Data = &materialComponent;
				materialNode.Type = EGLTFModelNode::MATERIAL;
				materialComponent.HasVertexColors = hasVertexColors;
				modelNode.Children.push_back(materialNode);
			}
			else
			{
				GLTFBuildNode material;
				material.Name = materialName;
				material.Type = EGLTFModelNode::MATERIAL;
				material.Data = &m_resourceManager.GetMaterial(materialName);
				modelNode.Children.push_back(material);
			}

			m_renderSystem.GetRenderAPI().AssignShaderToMesh(reinterpret_cast<TOR::Mesh*>(meshNode.Data)->Id, TOR::EShaderType::DiffusePBR);
		}
	}

	void GLTF_ModelBuilder::LoadLight(GLTFBuildNode& lightNode, tinygltf::Node& nodeToLoad, tinygltf::Value& extensionValue)
	{
		tinygltf::Light tinyGltfLight = m_tinyGLTFModel.lights[extensionValue.Get("light").GetNumberAsInt()];
		std::string lightName = m_ModelName + "_LIGHT_" + std::to_string(extensionValue.Get("light").GetNumberAsInt());

		//Babylon exporter fix - when the color is white the exporter ignores it.
		glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);
		if (tinyGltfLight.color.capacity() > 0)
		{
			color = glm::vec3(
				static_cast<float>(tinyGltfLight.color[0]),
				static_cast<float>(tinyGltfLight.color[1]),
				static_cast<float>(tinyGltfLight.color[2]));
		}

		if (tinyGltfLight.type == "point")
		{
			lightNode.Name = lightName;
			lightNode.Type = EGLTFModelNode::POINTLIGHT;

			if (m_resourceManager.IsPointLightLoaded(lightName))
			{
				TOR::PointLight& pointLight = m_resourceManager.GetPointLight(lightName);
				lightNode.Data = &pointLight;
			}
			else
			{
				TOR::PointLight& pointLight = m_resourceManager.LoadPointLight(lightName);
				pointLight.Color = color;
				pointLight.Intensity = static_cast<float>(tinyGltfLight.intensity);
				pointLight.Radius = tinyGltfLight.range > 0.0f ? static_cast<float>(tinyGltfLight.range) : 1.0f;

				lightNode.Data = &pointLight;
			}
		}
		else if (tinyGltfLight.type == "directional")
		{
			lightNode.Name = lightName;
			lightNode.Type = EGLTFModelNode::DIRECTIONALLIGHT;

			if (m_resourceManager.IsDirectionalLightLoaded(lightName))
			{
				TOR::DirectionalLight& directionalLight = m_resourceManager.GetDirectionalLight(lightName);
				lightNode.Data = &directionalLight;
			}
			else
			{
				TOR::DirectionalLight& directionalLight = m_resourceManager.LoadDirectionalLight(lightName);
				directionalLight.Color = color;
				directionalLight.Intensity = static_cast<float>(tinyGltfLight.intensity);

				lightNode.Name = lightName;
				lightNode.Data = &directionalLight;
			}
		}
		else if (tinyGltfLight.type == "spot")
		{
			lightNode.Name = lightName;
			lightNode.Type = EGLTFModelNode::SPOTLIGHT;

			if (m_resourceManager.IsSpotLightLoaded(lightName))
			{
				TOR::SpotLight& spotLight = m_resourceManager.GetSpotLight(lightName);
				lightNode.Data = &spotLight;
			}
			else
			{
				TOR::SpotLight& spotLight = m_resourceManager.LoadSpotLight(lightName);
				spotLight.Color = color;
				spotLight.Intensity = static_cast<float>(tinyGltfLight.intensity);
				spotLight.InnerCone = static_cast<float>(tinyGltfLight.spot.innerConeAngle);
				spotLight.OuterCone = static_cast<float>(tinyGltfLight.spot.outerConeAngle);

				lightNode.Data = &spotLight;
			}
		}
		else
		{
			FRAC_ASSERT(false, "WARNING: Unsupported light type encountered\n");
		}
	}

	void GLTF_ModelBuilder::LoadSkin(tinygltf::Node& nodeToLoad)
	{
		FRAC_ASSERT(false, "Ignoring glTF skin, implementation missing.");
	}

	GLTFBuildNode GLTF_ModelBuilder::LoadTexture(int index, bool isSRGB)
	{
		GLTFBuildNode textureNode;
		TOR::RenderAPI& renderAPI = m_renderSystem.GetRenderAPI();
		tinygltf::Texture tinyGltfTexture = m_tinyGLTFModel.textures[index];
		tinygltf::Image tinyGltfImage = m_tinyGLTFModel.images[tinyGltfTexture.source];

		std::string imageName = m_ModelName + "_" + std::to_string(index) + "_REUSED_IMAGE_" + tinyGltfImage.uri;
		std::string samplerName = tinyGltfTexture.sampler != -1
			? m_ModelName + "REUSED_SAMPLER_" + std::to_string(tinyGltfTexture.sampler)
			: "REUSED_SAMPLER_DEFAULT";

		std::string textureName = m_ModelName + "_REUSED_TEXTURE_" + tinyGltfTexture.name + "_" + std::to_string(index);
		if (!m_resourceManager.IsTextureLoaded(textureName))
		{
			if (!m_resourceManager.IsImageLoaded(imageName))
			{
				TOR::Image& imageOfTexture = m_resourceManager.LoadImage(imageName);

				imageOfTexture.FilePath = tinyGltfImage.uri.c_str();
				imageOfTexture.Width = tinyGltfImage.width;
				imageOfTexture.Height = tinyGltfImage.height;
				imageOfTexture.Format = tinyGltfImage.component;
				imageOfTexture.BitSize = tinyGltfImage.bits;
			}

			if (!m_resourceManager.IsSamplerLoaded(samplerName))
			{
				TOR::Sampler& sampler = m_resourceManager.LoadSampler(samplerName);
				if (tinyGltfTexture.sampler != -1)
				{
					tinygltf::Sampler tinyGltfSampler = m_tinyGLTFModel.samplers[tinyGltfTexture.sampler];
					sampler.MinFilter = tinyGltfSampler.minFilter > -1 ? TOR::Sampler::ConvertglTFFilterMethodToTORFilterMethod(tinyGltfSampler.minFilter) : TOR::ESampleFilterMethod::LINEAR_MIPMAP_LINEAR;
					sampler.MagFilter = tinyGltfSampler.magFilter > -1 ? TOR::Sampler::ConvertglTFFilterMethodToTORFilterMethod(tinyGltfSampler.magFilter) : TOR::ESampleFilterMethod::LINEAR;
					sampler.WrapFilterS = tinyGltfSampler.wrapS > -1 ? TOR::Sampler::ConvertglTFWrapMethodToTORWrapMethod(tinyGltfSampler.wrapS) : TOR::ETextureWrapMethod::REPEAT;
					sampler.WrapFilterT = tinyGltfSampler.wrapT > -1 ? TOR::Sampler::ConvertglTFWrapMethodToTORWrapMethod(tinyGltfSampler.wrapT) : TOR::ETextureWrapMethod::REPEAT;
					sampler.WrapFilterR = tinyGltfSampler.wrapR > -1 ? TOR::Sampler::ConvertglTFWrapMethodToTORWrapMethod(tinyGltfSampler.wrapR) : TOR::ETextureWrapMethod::REPEAT;
				}
				else
				{
					sampler.MinFilter = TOR::ESampleFilterMethod::LINEAR_MIPMAP_LINEAR;
					sampler.MagFilter = TOR::ESampleFilterMethod::LINEAR;
					sampler.WrapFilterS = TOR::ETextureWrapMethod::REPEAT;
					sampler.WrapFilterT = TOR::ETextureWrapMethod::REPEAT;
					sampler.WrapFilterR = TOR::ETextureWrapMethod::REPEAT;
				}
			}

			TOR::Texture& texture = m_resourceManager.LoadTexture(textureName);
			TOR::EImageChannelType format = isSRGB
				? TOR::EImageChannelType::SRGB8_ALPHA8
				: TOR::EImageChannelType::RGBA;

			renderAPI.CreateTexture(texture);
			renderAPI.LoadTexture(
				texture.Id,
				m_resourceManager.GetImage(imageName),
				tinyGltfImage.image.data(),
				tinyGltfImage.image.size(),
				m_resourceManager.GetSampler(samplerName),
				static_cast<int>(format) // should we cast format? you will be casting it in render api aswell this way @ Tom.
			);

			textureNode.Data = &texture;
		}
		else
		{
			textureNode.Type = EGLTFModelNode::TEXTURE;
			textureNode.Data = &m_resourceManager.GetTexture(textureName);
		}

		return textureNode;
	}
}