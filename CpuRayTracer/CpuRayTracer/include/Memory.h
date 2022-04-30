#pragma once

namespace CRT
{
#define L1_CACHE_LINE_SIZE 64

	extern void* AllocAligned(size_t sizeInBytes);
	extern void FreeAligned(void* address);

	template<typename T>
	T* AllocAligned(size_t count)
	{
		return (T*)AllocAligned(count * sizeof(T));
	}
}