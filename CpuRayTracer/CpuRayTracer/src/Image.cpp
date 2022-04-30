#include "Image.h"
#include <algorithm>

namespace CRT
{
	Image::~Image()
	{
		delete Data;
	}

	glm::vec3 Image::LookUp(float u, float v)
	{
		int x = std::clamp<int>(round(u * static_cast<float>(Width) - 0.5f), 0, Width - 1);
		int y = std::clamp<int>(round(v * static_cast<float>(Height) - 0.5f), 0, Height - 1);

		uint8_t* colorPtr = &Data[y * Width * ChannelCount + x * ChannelCount];
		glm::vec3 color = glm::vec3(
			(*(colorPtr) / 255.0f), 
			(*(colorPtr + 1) / 255.0f), 
			(*(colorPtr + 2) / 255.0f)
		);
		return color;
	}
}