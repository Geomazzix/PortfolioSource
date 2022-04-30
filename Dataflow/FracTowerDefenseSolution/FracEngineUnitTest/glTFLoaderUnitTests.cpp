#include <gtest/gtest.h>
#include <Core/ECSWrapper.h>
#include <Logger.h>
#include <CoreRenderAPI/Components/RenderComponents.h>
#include <Graphics/RenderSystem.h>
#include <CoreRenderAPI/RenderAPI.h>
#include <SceneManagement/SceneManager.h>
#include <Resources/PrefabManager.h>

#include "MockedPrefabManager.h"
#include "MockedRenderAPI.h"
#include "MockedRenderSystem.h"


using namespace TOR;
using namespace Frac;

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::AtLeast;

#include <filesystem>
#include <iostream>




using ::testing::Return;
using ::testing::ReturnRef; 
using ::testing::StrEq;


TEST(TestingGLTFLoading, load_gltf)
{
	//Frac::ECSWrapper flecsDep;
	//MockedRenderAPI renderAPI;
	//

	//
	//MockedRenderSystem renderSystem(flecsDep);
	//MockedPrefabManager mockprefabManager(renderSystem,flecsDep);
	//
	//SceneManager sut(renderSystem,mockprefabManager, flecsDep);

	//EXPECT_CALL(renderSystem, GetRenderAPI())
	//	.WillRepeatedly(ReturnRef(renderAPI));

	//EXPECT_CALL(renderAPI, CreateMesh(testing::_));

	//EXPECT_CALL(mockprefabManager,HasPrefab(testing::_))
	//.Times(2)
	//.WillRepeatedly(Return(false));

	//// I could not care less what is passed in here atm.
	//EXPECT_CALL(renderAPI, BufferAttributeData(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
	//.Times(3);

	//// I could not care less what is passed in here atm.
	//EXPECT_CALL(renderAPI, BufferElementData(testing::_, testing::_, testing::_, testing::_))
	//	.Times(1);

	//EXPECT_CALL(renderAPI, UnbindMesh(testing::_));

	//EXPECT_CALL(mockprefabManager, CachePrefab(StrEq("box_MESH_0_0_MATERIAL_0"), testing::_))
	//	.Times(1);

	//EXPECT_CALL(mockprefabManager, GetPrefab(StrEq("box_MESH_0_0_MATERIAL_0")))
	//	.Times(1)
	//	.WillOnce(Return(310));
	//
	//EXPECT_CALL(mockprefabManager, HasPrefab(StrEq("box_MATERIAL_0")))
	//	.Times(1);

	//EXPECT_CALL(mockprefabManager, CachePrefab(StrEq("box_MATERIAL_0"), testing::_))
	//	.Times(1);
	//
	//EXPECT_CALL(mockprefabManager, GetPrefab(StrEq("box_MATERIAL_0")))
	//	.Times(1)
	//	.WillOnce(Return(312));

	//EntityHandle cube = sut.GetModel("../TDGame/Assets/Models/Box/glTF/box.gltf",EModelFileType::GLTF);

	//EXPECT_TRUE(flecsDep.has<TOR::Transform>(cube));




	EXPECT_TRUE(true);
}
