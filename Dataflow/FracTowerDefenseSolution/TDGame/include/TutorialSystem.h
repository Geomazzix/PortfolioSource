#pragma once
#include "Core/ISystem.h"
class CursorSystem;
class GameDirector;
class TileManager;
class TutorialSystem : public Frac::ISystem
{
public:
	TutorialSystem(TileManager* tileManager,GameDirector* gameDirector, CursorSystem* cursorSystem);
	~TutorialSystem();
	void Update(float DeltaTime) override;
private:
	TileManager* m_tileManager;
	GameDirector* m_gameDirector;
	CursorSystem* m_cursorSystem;
	bool m_soldTurret = false;
	bool m_boughtTurret = false;
	float m_spawnDelay = 2.f;
	float m_spawnTimer = 2.1f;
	entt::entity m_tile = entt::null;
	Frac::Entity* m_Text = nullptr;
};

