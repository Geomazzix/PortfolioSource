#pragma once
#include <entt/entity/fwd.hpp>
#include "Core/ISystem.h"


class GameDirector;
class TileManager;
class CameraSystem;
class CursorSystem : public Frac::ISystem
{
public:
	CursorSystem(TileManager* tileManager,CameraSystem* CameraSystem,GameDirector* director);
	~CursorSystem();

	
	entt::entity GetSelectedTile();
	void Update(float dt) override;
private:
	TileManager* m_tileManager;
	CameraSystem* m_cameraSystem;
	GameDirector* m_gameDirector;
	Frac::Entity* m_cursor;
	float m_cursorRotation = 0.f;
	float m_rotationSpeed = 20.f;
	const glm::vec3 m_cursorSize{ 0.4f,0.4f,0.4f };
};

