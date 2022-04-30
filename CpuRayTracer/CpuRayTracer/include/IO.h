#pragma once
#include <cstdint>
#include <memory>
#include <vec3.hpp>
#include "Image.h"
#include "Model.h"
#include "ResourceManager.h"

namespace CRT
{
	/// <summary>
	/// The IO class handles low level Input-Output functionality for the renderer.
	/// </summary>
	class IO
	{
	public:
		static std::string GetNameFromFilePath(const std::string& filePath);
		static void StorePNG(const char* filePath, unsigned int width, unsigned int height, unsigned int numChannels, glm::vec3* colorData, bool normalizedColorData = false);

		static std::shared_ptr<Image> LoadImage(const char* filePath);
		static std::shared_ptr<Mesh> LoadWavefrontFile(ResourceManager& resourceManager, const std::string& filePath);

	private:
		static void CalculateNormal(float normals[3], float v0[3], float v1[3], float v2[3]);
	};
}