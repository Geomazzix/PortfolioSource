#pragma once

#pragma once

#ifdef PLATFORM_WIN64
#include <Graphics/RenderProfiling/D3D12RenderProfile.h>
#else
//Emitted PS4 code
#endif

namespace Th
{
#ifdef PLATFORM_WIN64
	typedef D3D12RenderProfile RenderProfile;
#else
	//Emitted PS4 code
#endif
}