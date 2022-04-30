#include "TDpch.h"
#include "Game.h"
#include "Tools/Version.h"
#include "Graphics/TextRenderSystem/TextRenderSystem.h"
#include "SceneManagement/SceneManager.h"
#include "Graphics/Components/TextComponent.h"
#include "InputSystem/InputSystem.h"
#include "Audio/AudioManager.h"
#include "Audio/AudioComponent.h"

#include "GameDirector.h"
#include "HealthSystem.h"
#include "PlayerManagerSystem.h"
#include "TileManager.h"
#include "EnemyDirectorSystem.h"
#include "CameraSystem.h"
#include "Graphics/DebugRenderer.h"
#include "EnemyEntityCreator.h"
#include "LevelEntityCreator.h"
#include "PlayerEntityCreator.h"
#include "LevelManager.h"

#include "Canvases/MainMenuCanvasSystem.h"
#include "Canvases/GameplayCanvasSystem.h"
#include "Canvases/PauseCanvasSystem.h"
#include "Canvases/DefeatCanvasSystem.h"
#include "Canvases/VictoryCanvasSystem.h"
#include "Canvases/LoadCanvasSystem.h"
#include "Canvases/DeveloperCanvasSystem.h"
#include "Canvases/OptionsCanvasSystem.h"
#include "Canvases/CreditsCanvasSystem.h"
#include "DeathAnimationSystem.h"
#include "SpawnAnimationSystem.h"
#include "BulletSystem.h"
#include "TurretSystem.h"
#include "TutorialSystem.h"

#include "CursorSystem.h"
#include "Canvases/NextLevelCanvasSystem.h"

namespace TDGame
{
	Game::Game() :
		m_engineCore(Frac::EngineCore::GetInstance())
	{
		m_engineCore.Initialize();
		
		LOGINFO("Current build version: %s", Version::GetVersion());
#if defined (PLATFORM_WINDOWS)
		Version::StoreVersion();
#endif

		Frac::SceneManager& sceneManager = m_engineCore.GetSceneManager();
		Frac::Entity& level1 = sceneManager.CreateScene("Level_1");

		SetupModelPreLoadList();
		
		m_engineCore.GetAudioManager().LoadBank("[Audio]Events.bank");
		m_engineCore.GetAudioManager().LoadBank("[Audio]Events.strings.bank");
		m_engineCore.GetAudioManager().LoadBank("[Audio]Music.bank");

		//Initialize Managers, Systems and Directors
		m_gameDirector = std::make_unique<GameDirector>();
		m_tileManager = std::make_unique<TileManager>();

		//Creating the entity creators
		m_enemyEntityCreator = std::make_unique<EnemyEntityCreator>(*m_tileManager, m_engineCore.GetFracRegistry(), m_engineCore.GetSceneManager());
		m_playerEntityCreator = std::make_unique<PlayerEntityCreator>(*m_tileManager, m_engineCore.GetFracRegistry(), m_engineCore.GetSceneManager());
		m_levelEntityCreator = std::make_unique<LevelEntityCreator>(*m_tileManager, m_engineCore.GetFracRegistry(), m_engineCore.GetSceneManager());
		
		m_cameraSystem = std::make_unique<CameraSystem>(*m_gameDirector);

#if defined (CONFIG_DEBUG) || defined (CONFIG_DEVELOP)
		m_renderSystem = std::make_unique<RenderSystem>(*m_gameDirector, *m_cameraSystem);	
#endif		
		m_bulletSystem = std::make_unique<BulletSystem>(*m_tileManager);
		m_turretSystem = std::make_unique<TurretSystem>(*m_tileManager, *m_playerEntityCreator);

		m_cursorSystem = std::make_unique<CursorSystem>(m_tileManager.get(), m_cameraSystem.get(), m_gameDirector.get());
		m_playerManagerSystem = std::make_unique<PlayerManagerSystem>(*m_tileManager, *m_gameDirector, *m_cursorSystem, *m_playerEntityCreator);
		m_healthSystem = std::make_unique<HealthSystem>(*m_gameDirector, *m_playerManagerSystem, *m_tileManager);

		m_enemyDirectorSystem = std::make_unique<EnemyDirectorSystem>(*m_gameDirector, *m_tileManager, *m_enemyEntityCreator);

		m_deathAnimationSystem = std::make_unique<DeathAnimationSystem>();
		m_gameDirector->AssignSystemToStates(m_deathAnimationSystem.get(), GameState::AllState & ~GameState::PauseState);

		m_spawnAnimationSystem = std::make_unique<SpawnAnimationSystem>();
		
		m_gameDirector->AssignSystemToStates(m_spawnAnimationSystem.get(), GameState::WavePhase);
		m_LevelManager = std::make_unique<LevelManager>(m_engineCore.GetFracRegistry(), *m_tileManager, *m_enemyEntityCreator, *m_levelEntityCreator, *m_playerEntityCreator, *m_enemyDirectorSystem, *m_playerManagerSystem,*m_deathAnimationSystem);
		m_LevelManager->Init();
		m_enemyEntityCreator.get()->SetLevelManager(*m_LevelManager);

		m_gameDirector->AssignSystemToStates(m_bulletSystem.get(), GameState::WavePhase | GameState::PreperationPhase);
		m_gameDirector->AssignSystemToStates(m_turretSystem.get(), GameState::WavePhase | GameState::PreperationPhase | GameState::TutorialState);
		
		//Audio
		Frac::Entity backgroundMusic(m_engineCore.GetFracRegistry(), "BackgroundMusic");
		backgroundMusic.SetParent(m_engineCore.GetSceneManager().GetActiveScene());
		m_engineCore.GetFracRegistry().AddComponent<TOR::Transform>(backgroundMusic, TOR::Transform());
		m_engineCore.GetFracRegistry().AddComponent<Frac::AudioSourceComponent>(backgroundMusic, Frac::AudioSourceComponent());
		m_engineCore.GetAudioManager().AddEvent(m_engineCore.GetFracRegistry().GetComponent<Frac::AudioSourceComponent>(backgroundMusic), "event:/BackgroundMusic");
		m_engineCore.GetAudioManager().AddEvent(m_engineCore.GetFracRegistry().GetComponent<Frac::AudioSourceComponent>(backgroundMusic), "event:/CreditsMusic");
		m_engineCore.GetAudioManager().PlayEvent(m_engineCore.GetFracRegistry().GetComponent<Frac::AudioSourceComponent>(backgroundMusic), "event:/BackgroundMusic");

		//Init Game Functionality
		entt::registry& m_enttReg = m_engineCore.GetEnttRegistry();
		Frac::EntityRegistry& m_fracReg = m_engineCore.GetFracRegistry();

		m_enemyDirectorSystem->SetMaxWaves(5);
		m_enemyDirectorSystem->SetPrepPhaseTimer(20.0f);
		m_playerManagerSystem->SetResources(50);

		const unsigned viewportWidth = m_engineCore.GetDevice().GetWindowWidth();
		const unsigned viewportHeight = m_engineCore.GetDevice().GetWindowHeight();

		//Create Cameras
		TOR::Camera camera;
		camera.AspectRatio = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
		camera.ScreenWidth = viewportWidth;
		camera.ScreenHeight = viewportHeight;
		camera.ZNear = 0.01f;
		camera.ZFar = 1000.0f;
		camera.ProjectionMat = glm::orthoRH(-80.0f, 80.0f, -45.0f, 45.0f, 0.01f, 1000.0f);
		camera.CameraType = TOR::ECameraType::Orthographic;
		camera.IsActive = true;

		const glm::vec3 camOffset = glm::vec3(10.0f, 12.5f, 10.0f);
		m_cameraSystem->CreateCamera(
			CameraSystem::ECameraBehaviour::DefenseGrid, 
			true,
			camOffset,
			glm::vec3(0.0f), 
			camera
		);
		m_cameraSystem->CreateCamera(CameraSystem::ECameraBehaviour::Noclip, false);

		m_tutorialSystem = std::make_unique<TutorialSystem>(m_tileManager.get(), m_gameDirector.get(),m_cursorSystem.get());
		
		CreateCanvases();

		m_gameDirector->AssignSystemToStates(m_tutorialSystem.get(), GameState::TutorialState | GameState::WavePhase | GameState::PreperationPhase | GameState::CreditsState);
		
		m_gameDirector->SwitchToGroup(GameState::LoadGameState);
		LOGINFO("[Game] Entered Load Game State State");

		LOGINFO("[Game] Game Initialized");

		m_engineCore.Run();
	}

	Game::~Game()
	{
		LOGINFO("[Game] Shutting down");
		m_engineCore.Shutdown();
	}

	void Game::CreateCanvases()
	{
		m_canvasSystems.emplace_back(
			std::make_unique<MainMenuCanvasSystem>(
				m_engineCore
				, *m_LevelManager
				, m_gameDirector.get()
				, GameState::MainMenuState)
		);

		m_canvasSystems.emplace_back(
			std::make_unique<DefeatCanvasSystem>(
				m_engineCore
				, *m_LevelManager
				, m_gameDirector.get()
				, GameState::LoseState)
		);

		m_canvasSystems.emplace_back(
			std::make_unique<VictoryCanvasSystem>(
				m_engineCore
				, *m_LevelManager
				, m_gameDirector.get()
				, GameState::WinState)
		);

		m_canvasSystems.emplace_back(
			std::make_unique<PauseCanvasSystem>(
				m_engineCore
				, *m_LevelManager
				, m_gameDirector.get()
				, GameState::PauseState)
		);

		auto& gameplayCanvas = m_canvasSystems.emplace_back(
			std::make_unique<GameplayCanvasSystem>(
				m_tileManager.get()
				, m_cursorSystem.get()
				, m_enemyDirectorSystem.get()
				, m_playerEntityCreator.get()
				,*m_LevelManager
				, m_playerManagerSystem.get()
				, m_engineCore
				, m_gameDirector.get()
				, GameState::PreperationPhase | GameState::WavePhase | GameState::TutorialState)
		);

		m_canvasSystems.emplace_back(
			std::make_unique<LoadCanvasSystem>(
				m_engineCore
				, m_gameDirector.get()
				, GameState::LoadGameState)
		);

		m_canvasSystems.emplace_back(
			std::make_unique<DeveloperCanvasSystem>(
				gameplayCanvas.get()
				, m_engineCore
				, m_gameDirector.get()
				, GameState::AllState ^ GameState::CreditsState )
		);
		
		m_canvasSystems.emplace_back(
			std::make_unique<OptionsCanvasSystem>(
				m_engineCore
				, m_gameDirector.get()
				, GameState::OptionsState)
		);
		
		m_canvasSystems.emplace_back(
			std::make_unique<CreditsCanvasSystem>(
				m_cameraSystem.get()
				, m_engineCore
				, m_gameDirector.get()
				, GameState::CreditsState)
		);

		m_canvasSystems.emplace_back(
			std::make_unique<NextLevelCanvasSystem>(
				m_LevelManager.get()
				, m_engineCore
				, m_gameDirector.get()
				, GameState::NextLevelState)
		);

		for (auto& canvas : m_canvasSystems)
		{
			canvas->Initialise();
		}
	}

	void Game::SetupModelPreLoadList()
	{
		Frac::SceneManager& sceneManager = Frac::EngineCore::GetInstance().GetSceneManager();
		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/Projectiles/Bullet.gltf");
		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/Enemies/Enemy_Flamer.gltf");
		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/Enemies/Enemy_MG.gltf");
		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/Enemies/Enemy_TeslaGun.gltf");
		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/Turrets/12.gltf");
		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/Core/core.gltf");

		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/NewLevels/Leve01.gltf");
		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/NewLevels/Level1.gltf");
		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/NewLevels/Level2.gltf");
		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/NewLevels/Level3.gltf");
		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/NewLevels/Level4.gltf");
		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/NewLevels/Level5.gltf");

		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/Turrets/10.gltf");
		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/Turrets/12.gltf");
		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/Turrets2.0/Basic_turret.gltf");
		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/Turrets2.0/Laser_Turret.gltf");
		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/Turrets2.0/Launcher_turret.gltf");
		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/Projectiles/Rocket.gltf");
		sceneManager.AddPreloadModel(Frac::EModelFileType::GLTF, "Models/Core/core.gltf");
	}

#if defined(CONFIG_DEBUG) || defined(CONFIG_DEVELOP)

	RenderSystem::RenderSystem(GameDirector& gameDirector, CameraSystem& cameraSystem) : ISystem { Frac::eStage::PostStore }
	, m_cameraSystem(cameraSystem), m_drawBoxes(true)
	{
		Frac::EngineCore::GetInstance().GetDebugRenderer().Initialize();
		gameDirector.AssignSystemToStates(this, GameState::PreperationPhase | GameState::WavePhase | GameState::PauseState);
	}

	void RenderSystem::Update(float DeltaTime)
	{	
		Frac::DebugRenderer& debugRenderer = Frac::EngineCore::GetInstance().GetDebugRenderer();
		entt::registry& m_enttReg = Frac::EngineCore::GetInstance().GetEnttRegistry();
		Frac::EntityRegistry& m_fracReg = Frac::EngineCore::GetInstance().GetFracRegistry();

		//Clear Screen
		debugRenderer.Reset();
		
		if (Frac::EngineCore::GetInstance().GetInputSystem().GetKeyOnce(Frac::KEY_P))
		{
			m_drawBoxes = m_drawBoxes ? false : true;
		}
		
		if (!m_drawBoxes)
		{
			return;
		}
		
		//Draw Boxes
		m_enttReg.view<BoxComponent, GridComponent, TOR::Transform>().each([this, &debugRenderer](BoxComponent& box, GridComponent& grid, TOR::Transform& trans)
			{
				if (box.isEnabled)
				{
					if (trans.Scale.y == 0.0f)
					{
						const glm::vec3 LeftLow = trans.Position - (glm::vec3(-0.5f, 0.0f, -0.5f) * trans.Scale);
						const glm::vec3 LeftHigh = trans.Position - (glm::vec3(-0.5f, 0.0f, 0.5f) * trans.Scale);
						const glm::vec3 RightLow = trans.Position - (glm::vec3(0.5f, 0.0f, -0.5f) * trans.Scale);
						const glm::vec3 RightHigh = trans.Position - (glm::vec3(0.5f, 0.0f, 0.5f) * trans.Scale);

						debugRenderer.AddLine(LeftLow, RightLow, box.Color);
						debugRenderer.AddLine(RightLow, RightHigh, box.Color);
						debugRenderer.AddLine(RightHigh, LeftHigh, box.Color);
						debugRenderer.AddLine(LeftHigh, LeftLow, box.Color);
					}
					else
					{
						const glm::vec3 LeftForwardLow = trans.Position - (glm::vec3(-0.5, -0.5, -0.5) * trans.Scale);
						const glm::vec3 LeftBackLow = trans.Position - (glm::vec3(-0.5, -0.5, 0.5) * trans.Scale);
						const glm::vec3 LeftForwardHigh = trans.Position - (glm::vec3(-0.5, 0.5, -0.5) * trans.Scale);
						const glm::vec3 LeftBackHigh = trans.Position - (glm::vec3(-0.5, 0.5, 0.5) * trans.Scale);
						const glm::vec3 RightForwardLow = trans.Position - (glm::vec3(0.5, -0.5, -0.5) * trans.Scale);
						const glm::vec3 RightBackLow = trans.Position - (glm::vec3(0.5, -0.5, 0.5) * trans.Scale);
						const glm::vec3 RightForwardHigh = trans.Position - (glm::vec3(0.5, 0.5, -0.5f) * trans.Scale);
						const glm::vec3 RightBackHigh = trans.Position - (glm::vec3(0.5, 0.5, 0.5) * trans.Scale);


						debugRenderer.AddLine(LeftForwardLow, LeftBackLow, box.Color);
						debugRenderer.AddLine(LeftForwardLow, RightForwardLow, box.Color);
						debugRenderer.AddLine(LeftForwardLow, LeftForwardHigh, box.Color);
						debugRenderer.AddLine(LeftForwardHigh, RightForwardHigh, box.Color);
						debugRenderer.AddLine(LeftForwardHigh, LeftBackHigh, box.Color);
						debugRenderer.AddLine(LeftBackLow, RightBackLow, box.Color);
						debugRenderer.AddLine(LeftBackLow, LeftBackHigh, box.Color);
						debugRenderer.AddLine(LeftBackHigh, RightBackHigh, box.Color);
						debugRenderer.AddLine(RightBackHigh, RightBackLow, box.Color);
						debugRenderer.AddLine(RightBackHigh, RightForwardHigh, box.Color);
						debugRenderer.AddLine(RightBackLow, RightForwardLow, box.Color);
						debugRenderer.AddLine(RightForwardLow, RightForwardHigh, box.Color);
					}
				}
			});

		//Render
		glm::mat4 vp = m_cameraSystem.GetMainCameraEntityVP();
		debugRenderer.Render(vp);
	}

#endif
}
