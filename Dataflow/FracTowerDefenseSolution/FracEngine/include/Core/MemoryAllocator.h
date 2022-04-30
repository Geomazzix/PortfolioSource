#pragma once

namespace Frac 
{
	class MemoryAllocator 
	{
	public:
		MemoryAllocator() = default;
		~MemoryAllocator() = default;

#ifdef PLATFORM_SWITCH
		//Emitted switch code
#endif // PLATFORM_SWITCH
	};
} // namespace Frac