#pragma once

#ifdef PLATFORM_WIN64
#include <Resources/D3D12/D3D12Shader.h>
#else
//Emitted PS4 code
#endif

namespace Th
{
#ifdef PLATFORM_WIN64
	typedef D3D12Shader Shader;
#else
	//Emitted PS4 code
#endif
}