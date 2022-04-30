#pragma once

#if defined PLATFORM_WIN64
#include <Graphics/D3D12/D3D12Device.h>
#elif defined PLATFORM_PS4
//Emitted PS4 code
#endif

namespace Th
{
#if defined PLATFORM_WIN64
	typedef D3D12Device RenderDevice;
#elif defined PLATFORM_PS4
	//Emitted PS4 code
#endif
}