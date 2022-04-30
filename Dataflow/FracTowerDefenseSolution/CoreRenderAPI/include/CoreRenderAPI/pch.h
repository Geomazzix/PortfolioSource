#pragma once

#include <cstddef>			// size_t
#include <memory>
#include <algorithm>
#include <chrono>
#include <cassert>
#include <cstdint>			//uint_64

// IO
#include <sstream>
#include <ostream>
#include <iostream>
#include <cstdio>

// Containers
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <forward_list>

#if defined(PLATFORM_WINDOWS)

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#ifdef APIENTRY
#undef APIENTRY
#endif

#include <cassert>
#include <windows.h>
#define TOR_ASSERT assert

#endif
//Emitted switch platform code
#endif
