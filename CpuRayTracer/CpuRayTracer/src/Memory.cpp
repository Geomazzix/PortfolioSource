#include "Memory.h"
#include <malloc.h>

namespace CRT
{
	void* AllocAligned(size_t sizeInBytes)
	{
		return _aligned_malloc(sizeInBytes, L1_CACHE_LINE_SIZE);
	}

	void FreeAligned(void* address)
	{
		_aligned_free(address);
	}
}