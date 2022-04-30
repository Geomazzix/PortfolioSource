#include "TDpch.h"
#include "CursorSystem.h"

#include "CameraSystem.h"
#include "TileManager.h"
#include "Graphics/Components/TextComponent.h"
#include "SceneManagement/SceneManager.h"
#include "GameDirector.h"

CursorSystem::CursorSystem(TileManager* tileManager, CameraSystem* cameraSystem, GameDirector* director) :Frac::ISystem{ Frac::eStage::OnUpdate }, m_tileManager{ tileManager }, m_cameraSystem{ cameraSystem }, m_gameDirector{ director }
{
	m_info.enabled = false;
	director->AssignSystemToStates(this,  GameState::AllState);
	Frac::EntityRegistry& registry = Frac::EngineCore::GetInstance().GetFracRegistry();
	m_cursor = new Frac::Entity{ registry,"Cursor" };
	registry.AddComponent<TOR::Transform>(*m_cursor,TOR::Transform{{},m_cursorSize,glm::quat{glm::radians(glm::vec3{90.f,0.f,0.f})}});
	m_cursor->SetParent(Frac::EngineCore::GetInstance().GetSceneManager().GetActiveScene());
	registry.AddComponent<Frac::TextComponent>(*m_cursor, Frac::TextComponent{"m",0.003f,{0.85f, 0.86f, 0.56f},Frac::TextComponent::FontFamily::Icons,false});
}

void CursorSystem::Update(float Dt)
{
	TOR::Transform& transform = Frac::EngineCore::GetInstance().GetFracRegistry().GetEnTTRegistry().get<TOR::Transform>(m_cursor->GetHandle());
	
	if(m_gameDirector->GetCurrentState() == GameState::PreperationPhase || m_gameDirector->GetCurrentState() == GameState::WavePhase || m_gameDirector->GetCurrentState() == GameState::TutorialState)
	{
		entt::entity mainCam = m_cameraSystem->GetMainCameraEntity();
		const CameraSystem::CameraTrait& camTrait = Frac::EngineCore::GetInstance().GetEnttRegistry().get<CameraSystem::CameraTrait>(mainCam);
		glm::vec3 lookat = camTrait.LookAt;

		m_cursorRotation = m_cursorRotation += m_rotationSpeed * Dt;
		if (m_cursorRotation > 360.f)
			m_cursorRotation -= 360.f;

		glm::vec2 gridPos = m_tileManager->WorldToGrid(lookat);

		transform.Scale = m_cursorSize;
		transform.Position = m_tileManager->GridToWorld(gridPos.x, gridPos.y) + glm::vec3{ -0.8f,0.f,-0.80f };
		transform.Orientation = glm::quat{ glm::radians(glm::vec3{90.f,m_cursorRotation,0.f}) };

#if defined(CONFIG_DEBUG) || defined(CONFIG_DEVELOP)
		if (camTrait.CamBehaviour != CameraSystem::ECameraBehaviour::DefenseGrid)
		{
			transform.Scale = glm::vec3(0.f);
		}
#endif
	}
	else
	{
		transform.Scale = { 0.f,0.f,0.f };
	}
	
	Frac::EngineCore::GetInstance().GetFracRegistry().GetEnTTRegistry().patch<TOR::Transform>(m_cursor->GetHandle());
}

CursorSystem::~CursorSystem()
{
	delete m_cursor;
	m_cursor = nullptr;
}

entt::entity CursorSystem::GetSelectedTile()
{
	entt::entity mainCam = m_cameraSystem->GetMainCameraEntity();
	const CameraSystem::CameraTrait& transform = Frac::EngineCore::GetInstance().GetEnttRegistry().get<CameraSystem::CameraTrait>(mainCam);
	glm::vec2 gridPos = m_tileManager->WorldToGrid(transform.LookAt);
	if (m_tileManager->IsInGrid(gridPos.x, gridPos.y))
	{
		return m_tileManager->GetTile((int)gridPos.x, (int)gridPos.y);
	}
	else
	{
		return entt::null;
	}
}
