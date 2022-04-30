#pragma once
#include <Resources/PrefabManager.h>
#include "MockedRenderSystem.h"
#include "gmock/gmock.h"

using namespace Frac;

class Frac::Renderer;
class Frac::ECSWrapper;

class MockedPrefabManager : public PrefabManager
{
	public:
	MockedPrefabManager(MockedRenderSystem& renderSystem, Frac::ECSWrapper& ecs);

	MOCK_METHOD1(HasPrefab, bool(std::string a_prefabName));
	MOCK_METHOD1(GetPrefab, EntityHandle(std::string a_prefabName));
	MOCK_METHOD1(GetPrefabInstance, EntityHandle(std::string a_prefabName));
	MOCK_METHOD2(CachePrefab, void(std::string a_prefabName, EntityHandle a_handleToEntityToCache));
};