#pragma once
#include "TileManager.h"
#include "Core/ISystem.h"
#include "PlayerEntityCreator.h"
class GameDirector;
class CursorSystem;

class PlayerManagerSystem : public Frac::ISystem
{
public:

	PlayerManagerSystem(TileManager& tileManager, GameDirector& gameDirector, CursorSystem& cursorSystem, PlayerEntityCreator& playerEntityCreator);

	~PlayerManagerSystem() = default;

	void Update(float DeltaTime) override;

	int GetResources();
	void SetResources(int value);
	void AddResources(int toAdd);
	void RemoveResources(int toRemove);
	void SellAction();
	void BuildAction();
	void SetSelectedTower(int value);

private:
	void ShowTurretRange(float DeltaTime);
	int m_resources;
	int m_selectedTower;
	float m_showRangeDelay{2.f};
	float m_showRangeTimer{ 0.f };
	entt::entity m_selectedTile{entt::null};
	int m_oldSelectedTower = 0;
	TileManager& m_tileManager;
	GameDirector& m_gameDirector;
	CursorSystem& m_cursorSystem;
	Frac::InputSystem& m_inputSystem;
	PlayerEntityCreator& m_playerEntityCreator;
};

