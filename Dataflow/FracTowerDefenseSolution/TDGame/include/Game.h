#pragma once
#include <Core/EngineCore.h>

#include "Core/ISystem.h"

class EnemyEntityCreator;
class PlayerEntityCreator;
class LevelEntityCreator;
class CameraSystem;
class GameDirector;
class HealthSystem;
class PlayerManagerSystem;
class TileManager;
class EnemyDirectorSystem;
class LevelManager;
class CursorSystem;
class CanvasSystem;
class TurretSystem;
class BulletSystem;
class DeathAnimationSystem;
class SpawnAnimationSystem;
class TutorialSystem;

namespace TDGame
{

#if defined(CONFIG_DEBUG) || defined(CONFIG_DEVELOP)
	class RenderSystem : public Frac::ISystem
	{
	public:

		RenderSystem(GameDirector& gameDirector, CameraSystem& cameraSystem);

		~RenderSystem() = default;

		void Update(float DeltaTime) override;

	private:
		CameraSystem& m_cameraSystem;

		bool m_drawBoxes;
	};
#endif

	class Game 
	{
	public:
		Game();
		~Game();

	private:
		void CreateCanvases();
		void SetupModelPreLoadList();
		Frac::EngineCore& m_engineCore;

#if defined (CONFIG_DEBUG) || defined (CONFIG_DEVELOP)
		std::unique_ptr<RenderSystem> m_renderSystem;
#endif
		std::unique_ptr<CameraSystem> m_cameraSystem;
		std::unique_ptr<GameDirector> m_gameDirector;
		std::unique_ptr<HealthSystem> m_healthSystem;
		std::unique_ptr<PlayerManagerSystem> m_playerManagerSystem;
		std::unique_ptr<EnemyDirectorSystem> m_enemyDirectorSystem;	
		std::unique_ptr<TileManager> m_tileManager;
		std::unique_ptr<EnemyEntityCreator> m_enemyEntityCreator;
		std::unique_ptr<PlayerEntityCreator> m_playerEntityCreator;
		std::unique_ptr<LevelEntityCreator> m_levelEntityCreator;
		std::unique_ptr<LevelManager> m_LevelManager;
		std::unique_ptr<CursorSystem> m_cursorSystem;
		std::unique_ptr<TurretSystem> m_turretSystem;
		std::unique_ptr<BulletSystem> m_bulletSystem;
		std::unique_ptr<DeathAnimationSystem> m_deathAnimationSystem;
		std::unique_ptr<SpawnAnimationSystem> m_spawnAnimationSystem;
		std::unique_ptr<TutorialSystem> m_tutorialSystem;
		// canvases
		std::vector<std::unique_ptr<CanvasSystem>> m_canvasSystems;

	};
}