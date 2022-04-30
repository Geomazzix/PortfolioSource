#pragma once

#ifndef STDAFX_H
#define STDAFX_H
#endif

#include <cstddef>			// size_t
#include <memory>
#include <algorithm>
#include <chrono>
#include <cstdint>			//uint_64

// IO
#include <sstream>
#include <ostream>
#include <iostream>
#include <cstdio>

// Containers
#include <vector>
#include <array>
#include <unordered_map>
#include <string>
#include <queue>
#include <stack>
#include <list>
#include <set>
#include <forward_list>

#include <Core/EngineCore.h>

//#include "Components.h"
#include "GameSettings.h"

// Externals
#include <Glm/mat4x4.hpp>
#include <Glm/vec3.hpp>
#include <Glm/glm.hpp>
#include <Glm/ext/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <CoreRenderAPI/Components/RenderComponents.h>

// Utility
#include "Logger.h"
#include "Core/FileIO.h"