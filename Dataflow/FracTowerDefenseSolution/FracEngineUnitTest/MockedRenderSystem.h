#pragma once
#include <Graphics/RenderSystem.h>

#include "gmock/gmock.h"  // Brings in Google Mock.

using namespace Frac;

class MockedRenderSystem : public Renderer {
public:
	MockedRenderSystem(ECSWrapper& wrapper);
	MOCK_METHOD3(Initialize, void(const std::string& dllName, unsigned viewportWidth, unsigned viewportHeight));
	MOCK_METHOD0(Shutdown, void());
	MOCK_CONST_METHOD0(GetRenderAPI, TOR::RenderAPI&());
};