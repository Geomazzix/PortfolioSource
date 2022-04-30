#pragma once

#ifdef PLATFORM_WIN64
#include <Graphics/Window/Win32Window.h>
#else
//Emitted PS4 code
#endif

namespace Th
{
#ifdef PLATFORM_WIN64
	typedef Win32Window Window;
#else
	//Emitted PS4 code
#endif
}