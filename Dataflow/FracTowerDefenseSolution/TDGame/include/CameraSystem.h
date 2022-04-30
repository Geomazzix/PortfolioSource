#pragma once
#include "Core/ISystem.h"
#ifdef PLATFORM_WINDOWS
#include "Platform/PC/PCDevice.h"
#elif PLATFORM_SWITCH
//Switch Code Emitted
#endif

class GameDirector;

class CameraSystem : public Frac::ISystem
{
public:
	CameraSystem(GameDirector& gameDirector);
	~CameraSystem() = default;

	enum struct ECameraBehaviour {
		Static,
		DefenseGrid,
		Noclip,
		Demo
	};
	
	struct CameraTrait{
		ECameraBehaviour	CamBehaviour			= ECameraBehaviour::DefenseGrid;
		glm::vec3			LookAt					= glm::vec3(0.f);
		float				NoclipSpeed				= 6.f;
		float				NoclipRotSpeed			= 4.5f;
		float				NoclipSpeedMultiplier	= 2.f;
		glm::vec3			Offset					= glm::vec3(0.f);
		glm::vec2			MinMaxZoom				= {5.0f,26.f};
		float				MoveSpeed				= 10.f;
		float				ZoomSpeed				= 4.f;
		float				DemoTime				= 0.f;
		glm::vec3			DemoSpeed				= glm::vec3(10.f, 8.f, 10.f);
		bool				Initialized				= false;
		glm::vec3			MinBounds				= glm::vec3(0.f);
		glm::vec3			MaxBounds				= glm::vec3(0.f);
	};

	void Update(float dt) override;
	
	Frac::Entity& CreateCamera(
		ECameraBehaviour camBehaviour = ECameraBehaviour::Noclip, 
		bool makeActiveCam = true, 
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 10.0f), 
		glm::vec3 lookAt = glm::vec3(0.f), 
		TOR::Camera camValues = TOR::Camera({
			glm::mat4(1.f),
			70.0f, //FOV
			0.01f, //NEAR
			1000.0f, //FAR
			static_cast<float>(Frac::EngineCore::GetInstance().GetDevice().GetWindowWidth() / Frac::EngineCore::GetInstance().GetDevice().GetWindowHeight()), //ASPECTRATIO
			static_cast<int>(Frac::EngineCore::GetInstance().GetDevice().GetWindowWidth()),//WIDTH
			static_cast<int>(Frac::EngineCore::GetInstance().GetDevice().GetWindowHeight()),//HEIGHT
			TOR::ECameraType::Perspective,//CAM TYPE 
			true,//IS ACTIVE
			true//IS DIRTY
			}));

	void SetMainCameraEntity(entt::entity newMain);

	Frac::Entity* GetDefaultCameraEnt() const;
	Frac::Entity* GetPreviousActiveCamEnt() const;
	
	entt::entity GetMainCameraEntity() const;
	glm::mat4 GetMainCameraEntityVP() const;

	std::vector<entt::entity> GetCameraEntities(bool dontGetMain = false);

private:
	void UpdateMainCameraEntity(const float dt, entt::entity, TOR::Transform&, TOR::Camera&, CameraTrait&);

	void UpdateDefenseGrid(const float dt, entt::entity, TOR::Transform&, TOR::Camera&, CameraTrait&);
	void UpdateNoclip(const float dt, entt::entity, TOR::Transform&, TOR::Camera&, CameraTrait&);
	
	Frac::InputSystem& m_inputSystem;
	GameDirector& m_gameDirector;
	Frac::EntityRegistry& m_registry;
	glm::vec3 m_upVec;
	int m_camCounter;
	Frac::Entity* m_mainCamera;
	bool m_activeCamHasChanged;
	entt::entity m_newActiveCamID;

	Frac::Entity* m_defaultCamera;
	Frac::Entity* m_previousActiveCam;

	float m_deadzoneSize;
};
