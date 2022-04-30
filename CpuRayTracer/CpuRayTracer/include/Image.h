#pragma once
#include <cstdint>
#include <vec3.hpp>

namespace CRT
{
	/// <summary>
	/// An image contains all the texture data and the properties of the image.
	/// </summary>
	struct Image
	{
		Image() = default;
		~Image();

		int Width;
		int Height;
		int ChannelCount;
		uint8_t* Data;

		glm::vec3 LookUp(float u, float v);
	};
}