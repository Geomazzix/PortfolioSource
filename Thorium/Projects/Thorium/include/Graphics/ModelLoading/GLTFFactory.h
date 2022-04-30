#pragma once
#ifdef PLATFORM_WIN64
#include <Graphics/ModelLoading/GLTFFactoryWin.h>

#elif PLATFORM_PS4
//Emitted PS4 code
#endif

namespace Th
{
#ifdef PLATFORM_WIN64
	typedef GLTFFactoryWin GLTFFactory;
#elif PLATFORM_PS4
	//Emitted PS4 code
#endif

}
