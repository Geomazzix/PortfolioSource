#pragma once

#ifdef PLATFORM_WIN64
#include <Resources/D3D12/D3D12Mesh.h>
#elif PLATFORM_PS4
//Emitted PS4 code
#endif


namespace Th
{
#ifdef PLATFORM_WIN64
	typedef D3D12Mesh Mesh;
#elif defined PLATFORM_PS4
	//Emitted PS4 code
#endif
}