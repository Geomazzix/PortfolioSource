#include "IO.h"
#include <gtc/type_ptr.hpp>
#include <common.hpp>
#include <vector>
#include "Material.h"
#include "Utility/MathUtility.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace CRT
{
	std::string IO::GetNameFromFilePath(const std::string& filePath)
	{
		std::size_t extentionCaretPos = filePath.find_last_of('.');
		std::size_t meshNameCaretPosEnd = filePath.find_last_of('/');
		const std::size_t extentionStringLength = filePath.size() - extentionCaretPos;
		return filePath.substr(meshNameCaretPosEnd + 1, filePath.size() - meshNameCaretPosEnd - extentionStringLength - 1);
	}

	void IO::StorePNG(const char* filePath, unsigned int width, unsigned int height, unsigned int numChannels, glm::vec3* colorData, bool normalizedColorData)
	{
		//uint8_t is the channel size, since the renderer samples RGB this means that it has to be multiplied by the channel count.
		uint8_t* pixels = new uint8_t[width * height * numChannels];

		for (int pixelIndex = 0, j = 0; j < height; j++)
		{
			for (int i = 0; i < width; i++)
			{
				for(int channelIndex = 0; channelIndex < numChannels; channelIndex++)
				{
					if(normalizedColorData)
					{
						float channelValue = glm::clamp<float>(colorData[pixelIndex / 3][channelIndex], 0.0f, 1.0f);
						pixels[pixelIndex + channelIndex] = static_cast<uint8_t>(channelValue * 255.0f);
					}
					else
					{
						float channelValue = glm::clamp<float>(colorData[pixelIndex / 3][channelIndex], 0.0f, 255.0f);
						pixels[pixelIndex + channelIndex] = static_cast<uint8_t>(channelValue);
					}
				}

				pixelIndex += numChannels;
			}
		}

		stbi_write_png(filePath, width, height, numChannels, pixels, width * numChannels);
		delete[] pixels;
	}

	std::shared_ptr<CRT::Image> IO::LoadImage(const char* filePath)
	{
		std::shared_ptr<Image> image = std::make_shared<Image>();
		stbi_set_flip_vertically_on_load(true);
		stbi_hdr_to_ldr_gamma(1.0f);
		image->Data = stbi_load(filePath, &image->Width, &image->Height, &image->ChannelCount, STBI_rgb);
		//stbi_image_free(data); //Do not free the data, since there is on GPU in the process the data needs to be kept as RAM.
		return image;
	}

	std::shared_ptr<CRT::Mesh> IO::LoadWavefrontFile(ResourceManager& resourceManager, const std::string& filePath)
	{
		printf("Started loading %s\n", filePath.c_str());

		std::size_t extentionCaretPos = filePath.find_last_of('.');
		std::size_t meshNameCaretPosEnd = filePath.find_last_of('/');

		const std::size_t extentionStringLength = filePath.size() - extentionCaretPos;
		std::string fileName = IO::GetNameFromFilePath(filePath);
		std::string directory = filePath.substr(0, filePath.size() - fileName.size() - extentionStringLength);

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warning, error;

		const bool result = tinyobj::LoadObj(
			&attrib, 
			&shapes, 
			&materials, 
			&warning, 
			&error, 
			filePath.c_str(), 
			directory.c_str(), 
			true, 
			true
		);

		if (!warning.empty())
		{
			printf("TinyObjLoader WARNING: %s\n", warning.c_str());
		}
		
		if (!error.empty())
		{
			printf("TinyObjLoader ERROR: %s\n", error.c_str());
		}

		if (!result)
		{
			printf("Aborting model loading...\n");
			return nullptr;
		}

		std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
		glm::vec3 minBounds = glm::vec3(INFINITY);
		glm::vec3 maxBounds = glm::vec3(-INFINITY);
		std::vector<std::shared_ptr<Primitive>> triangles;
		std::map<int, std::shared_ptr<Material>> meshMaterialMap;

		//Load materials.
		for(int i = 0; i < materials.size(); i++)
		{
			auto& tinyObjMaterial = materials[i];
			std::shared_ptr<Material> submeshMaterial = std::make_shared<Material>();
			meshMaterialMap.emplace(i, submeshMaterial);
		
			//Color loading.
			meshMaterialMap[i]->EmissionCoefficient = {
				tinyObjMaterial.emission[0],
				tinyObjMaterial.emission[1],
				tinyObjMaterial.emission[2]
			};

			meshMaterialMap[i]->AmbientReflectivity = {
				tinyObjMaterial.ambient[0],
				tinyObjMaterial.ambient[1],
				tinyObjMaterial.ambient[2]
			};

			meshMaterialMap[i]->AlbedoCoefficient = {
				tinyObjMaterial.diffuse[0],
				tinyObjMaterial.diffuse[1],
				tinyObjMaterial.diffuse[2]
			};

			meshMaterialMap[i]->SpecularCoefficient = {
				tinyObjMaterial.specular[0],
				tinyObjMaterial.specular[1],
				tinyObjMaterial.specular[2]
			};
			meshMaterialMap[i]->PhongExponent = tinyObjMaterial.shininess;

			//Transparency.
			meshMaterialMap[i]->RefractiveIndex = tinyObjMaterial.ior;
			
			if(tinyObjMaterial.dissolve < 1.0f)
			{
				meshMaterialMap[i]->IsDialetic = true;
				meshMaterialMap[i]->Reflectivity = 1.0f - tinyObjMaterial.dissolve;
				meshMaterialMap[i]->RefractiveIndex = tinyObjMaterial.dissolve;
				meshMaterialMap[i]->AbsorbanceCoefficient = {
					tinyObjMaterial.transmittance[0],
					tinyObjMaterial.transmittance[1],
					tinyObjMaterial.transmittance[2]
				};
			}
			else
			{
				//#TODO Add reflectivity support? - mtl doesn't seem to specify anything regarding this.
			}

			//#TODO: Texture loading.

			resourceManager.AddMaterial(fileName + "_" + std::to_string(i), submeshMaterial);
		}

		//Load submeshes into the model.
		for (const auto& shape : shapes)
		{
			std::map<int, std::vector<std::shared_ptr<Primitive>>> meshData;

			for (int f = 0; f < (shape.mesh.indices.size() / 3); f++)
			{
				const tinyobj::index_t verticiesIndex[3] =
				{
					shape.mesh.indices[3 * f + 0],
					shape.mesh.indices[3 * f + 1],
					shape.mesh.indices[3 * f + 2]
				};

				//Triangle positions.
				glm::vec3 v0, v1, v2;
				for (int compIndex = 0; compIndex < 3; compIndex++)
				{
					v0[compIndex] = attrib.vertices[3 * verticiesIndex[0].vertex_index + compIndex];
					v1[compIndex] = attrib.vertices[3 * verticiesIndex[1].vertex_index + compIndex];
					v2[compIndex] = attrib.vertices[3 * verticiesIndex[2].vertex_index + compIndex];
				}

				//Update the AABB bounds.
				minBounds = CRT::min(minBounds, v0);
				minBounds = CRT::min(minBounds, v1);
				minBounds = CRT::min(minBounds, v2);

				maxBounds = CRT::max(maxBounds, v0);
				maxBounds = CRT::max(maxBounds, v1);
				maxBounds = CRT::max(maxBounds, v2);

				//Triangle face normal.
				glm::vec3 n;
				if (!attrib.normals.empty())
				{
					glm::vec3 n0, n1, n2;
					for (int compIndex = 0; compIndex < 3; compIndex++)
					{
						n0[compIndex] = attrib.normals[3 * verticiesIndex[0].normal_index + compIndex];
						n1[compIndex] = attrib.normals[3 * verticiesIndex[1].normal_index + compIndex];
						n2[compIndex] = attrib.normals[3 * verticiesIndex[2].normal_index + compIndex];
					}

					n0 = normalize(n0);
					n1 = normalize(n1);
					n2 = normalize(n2);
					float totalLength = length(n0) + length(n1) + length(n2);

					n = normalize((n0 + n1 + n2) / totalLength);
				}
				else
				{
					glm::vec3 n0, n1, n2;
					IO::CalculateNormal(
						glm::value_ptr<float>(n0),
						glm::value_ptr<float>(v0),
						glm::value_ptr<float>(v1),
						glm::value_ptr<float>(v2));

					n1[0] = n0[0];
					n1[1] = n0[1];
					n1[2] = n0[2];

					n2[0] = n0[0];
					n2[1] = n0[1];
					n2[2] = n0[2];

					n = normalize((n0 + n1 + n2) / 3.0f);
				}

				if(meshData.find(shape.mesh.material_ids[f]) == meshData.end())
				{
					meshData.emplace(shape.mesh.material_ids[f], std::vector<std::shared_ptr<Primitive>>());
				}

				for(int j = 0; j < 3; j++)
				{
					meshData[shape.mesh.material_ids[f]].push_back(std::make_shared<Triangle>(v0, v1, v2, n));
				}
			}

			//Adding the meshes to the model and adding those to the resource system.
			for(auto& data : meshData)
			{
				if (data.second.size() <= 0) continue;

				for(auto& triangle : data.second)
				{
					std::weak_ptr<Material> material = data.first == -1
						? resourceManager.GetMaterial(ResourceManager::GetDefaultMaterialName())
						: resourceManager.GetMaterial(fileName + "_" + std::to_string(data.first));

					triangles.push_back(triangle);
					triangles[triangles.size() - 1]->SetMaterial(material);
				}
			}
		}

		AABB meshBounds(minBounds, maxBounds);
		mesh->Initialize(resourceManager.GetMaterial(ResourceManager::GetDefaultMaterialName()), glm::vec3(0.0f, (maxBounds.y + minBounds.y) * 0.5f + abs(minBounds.y), 0.0f), glm::vec3(0.0f), glm::vec3(1.0f), meshBounds);
		mesh->SetTriangles(std::move(triangles));
		printf("Finished loading %s\n\n", filePath.c_str());
		return mesh;
	}

	void IO::CalculateNormal(float normals[3], float v0[3], float v1[3], float v2[3])
	{
		float v10[3];
		v10[0] = v1[0] - v0[0];
		v10[1] = v1[1] - v0[1];
		v10[2] = v1[2] - v0[2];

		float v20[3];
		v20[0] = v2[0] - v0[0];
		v20[1] = v2[1] - v0[1];
		v20[2] = v2[2] - v0[2];

		normals[0] = v20[1] * v10[2] - v20[2] * v10[1];
		normals[1] = v20[2] * v10[0] - v20[0] * v10[2];
		normals[2] = v20[0] * v10[1] - v20[1] * v10[0];

		const float len2 = normals[0] * normals[0] + normals[1] * normals[1] + normals[2] * normals[2];
		if (len2 > 0.0f) {
			const float len = sqrtf(len2);
			normals[0] /= len;
			normals[1] /= len;
		}
	}
}