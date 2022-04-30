#pragma once

#ifdef PLATFORM_WIN64
#include <Graphics/D3D12/D3D12DebugRenderer.h>
#else
//Emitted PS4 code
#endif

namespace Th
{
#ifdef PLATFORM_WIN64
	typedef D3D12DebugRenderer DebugRenderer;
#else
	//Emitted PS4 code
#endif
}