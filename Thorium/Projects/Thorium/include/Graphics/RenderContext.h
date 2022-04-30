#pragma once

#ifdef PLATFORM_WIN64
#include <Graphics/D3D12/D3D12RenderContext.h>
#elif defined (PLATFORM_PS4)
//Emitted PS4 code
#endif

namespace Th
{
#ifdef PLATFORM_WIN64
	typedef D3D12RenderContext RenderContext;
#elif defined PLATFORM_PS4
	//Emitted PS4 code
#endif
}