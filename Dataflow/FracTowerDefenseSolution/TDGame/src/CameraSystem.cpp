#include "TDpch.h"
#include "CameraSystem.h"
#include "GameDirector.h"

#include "SceneManagement/SceneManager.h"
#include "InputSystem/InputSystem.h"
#include "Graphics/TextRenderSystem/TextRenderSystem.h"

CameraSystem::CameraSystem(GameDirector& gameDirector) :
	ISystem(Frac::eStage::OnUpdate),
	m_inputSystem(Frac::EngineCore::GetInstance().GetInputSystem()),
	m_registry(Frac::EngineCore::GetInstance().GetFracRegistry()),
	m_upVec(glm::vec3(0.f, 1.f, 0.f)),
	m_camCounter(0),
	m_mainCamera(nullptr),
	m_activeCamHasChanged(false),
	m_newActiveCamID(entt::null),
	m_gameDirector(gameDirector),
	m_defaultCamera(&CreateCamera(CameraSystem::ECameraBehaviour::Static, false)),
	m_previousActiveCam(nullptr),
	m_deadzoneSize(0.25f)
{
	m_gameDirector.AssignSystemToStates(this, GameState::AllState, true);
}

void CameraSystem::Update(float dt)
{
	m_registry.GetEnTTRegistry().view<TOR::Transform, TOR::Camera, CameraTrait>().each([this, dt](TOR::Transform& transform, TOR::Camera& camera, CameraTrait& camTrait)
		{
			const entt::entity ent = entt::to_entity(m_registry.GetEnTTRegistry(), transform);
			UpdateMainCameraEntity(dt, ent, transform, camera, camTrait);
		});

	Frac::EngineCore::GetInstance().GetTextRenderSystem().SetViewProjection(GetMainCameraEntityVP());
}

Frac::Entity& CameraSystem::CreateCamera(ECameraBehaviour camBehaviour, bool makeActiveCam, glm::vec3 position, glm::vec3 lookAt, TOR::Camera camValues)
{
	Frac::Entity* camEnt = new Frac::Entity(m_registry, "CameraEntity_" + std::to_string(m_camCounter++));

	camEnt->SetParent(Frac::EngineCore::GetInstance().GetSceneManager().GetActiveScene());

	CameraTrait camTrait;
	camTrait.CamBehaviour = camBehaviour;
	camTrait.LookAt = lookAt;
	camTrait.Offset = (lookAt - position);

	camValues.IsActive = makeActiveCam;

	m_registry.AddComponent<TOR::Transform>(*camEnt, TOR::Transform({ position }));
	m_registry.AddComponent<TOR::Camera>(*camEnt, TOR::Camera(camValues));
	m_registry.AddComponent<CameraTrait>(*camEnt, CameraTrait(camTrait));

	if (makeActiveCam)
	{
		m_mainCamera = camEnt;
		m_newActiveCamID = m_mainCamera->GetHandle();
	}

	return *camEnt;
}

entt::entity CameraSystem::GetMainCameraEntity() const
{
	return m_mainCamera->GetHandle();
}

glm::mat4 CameraSystem::GetMainCameraEntityVP() const
{
	if (!m_registry.HasComponent<TOR::Transform>(*m_mainCamera))
	{
		LOGWARNING("CameraSystem | mainCamera has no Transform!");
		return glm::mat4(0.f);
	}

	const TOR::Transform& camTrans = m_registry.GetComponent<TOR::Transform>(*m_mainCamera);
	const CameraTrait& camTrait = m_registry.GetComponent<CameraTrait>(*m_mainCamera);

	const glm::mat4 viewMatrix = glm::lookAtRH(
		camTrans.Position,
		camTrait.LookAt,
		m_upVec);

	return (m_registry.GetComponent<TOR::Camera>(*m_mainCamera).ProjectionMat * viewMatrix);
}

std::vector<entt::entity> CameraSystem::GetCameraEntities(bool dontGetMain)
{
	std::vector<entt::entity> camEnts;

	m_registry.GetEnTTRegistry().view<TOR::Camera, CameraTrait>().each([this, dontGetMain, &camEnts](TOR::Camera& camera, CameraTrait& camTrait) mutable
		{
			if ((dontGetMain && !camera.IsActive) || !dontGetMain)
			{
				camEnts.emplace_back(entt::to_entity(m_registry.GetEnTTRegistry(), camera));
			}
		});

	return camEnts;
}

void CameraSystem::SetMainCameraEntity(entt::entity newMain)
{
	if (!m_registry.HasComponent<CameraTrait>(m_registry.TranslateENTTEntity(newMain)))
	{
		LOGWARNING("CameraSystem | Cannot set entity without a camera component as the main camera!");
		return;
	}

	m_previousActiveCam = m_mainCamera;

	TOR::Camera& oldCam = m_registry.GetComponent<TOR::Camera>(*m_mainCamera);
	oldCam.IsActive = false;

	m_activeCamHasChanged = true;
	m_newActiveCamID = newMain;
	m_mainCamera = &m_registry.TranslateENTTEntity(newMain);
}

Frac::Entity* CameraSystem::GetDefaultCameraEnt() const
{
	return m_defaultCamera;
}

Frac::Entity* CameraSystem::GetPreviousActiveCamEnt() const
{
	return m_previousActiveCam;
}

void CameraSystem::UpdateMainCameraEntity(const float dt, entt::entity e, TOR::Transform& transform, TOR::Camera& camera, CameraTrait& camTrait)
{
	if (m_activeCamHasChanged && e == m_newActiveCamID)
	{
		m_activeCamHasChanged = false;

		m_newActiveCamID = entt::null;
		camera.IsActive = true;
	}
	if (m_mainCamera->GetHandle() != e)
	{
		return;
	}

	m_registry.GetEnTTRegistry().patch<TOR::Transform>(e);

	// I need to be able to patch it once, this is currently being called every frame.
	m_registry.GetEnTTRegistry().patch<TOR::Camera>(e);
	if ((m_gameDirector.GetCurrentState() == GameState::PreperationPhase) || (m_gameDirector.GetCurrentState() == GameState::WavePhase) || (m_gameDirector.GetCurrentState() == GameState::TutorialState))
	{
		switch (camTrait.CamBehaviour)
		{
		case ECameraBehaviour::Static: break;
		case ECameraBehaviour::DefenseGrid:

			UpdateDefenseGrid(dt, e, transform, camera, camTrait);

			break;
		case ECameraBehaviour::Noclip:

			UpdateNoclip(dt, e, transform, camera, camTrait);

			break;
		case ECameraBehaviour::Demo: break;
		default:;
		}
	}

	const float width = static_cast<float>(Frac::EngineCore::GetInstance().GetDevice().GetWindowWidth());
	const float height = static_cast<float>(Frac::EngineCore::GetInstance().GetDevice().GetWindowHeight());

	if (width != 0.f || height != 0.f)
	{
		switch (camera.CameraType)
		{
		case TOR::ECameraType::Perspective:
		{
			camera.ProjectionMat = glm::perspectiveFovRH(
				glm::radians<float>(camera.FovInDeg),
				width,
				height,
				camera.ZNear,
				camera.ZFar);//PROJECTION MATRIX
		}
		break;
		case TOR::ECameraType::Orthographic:
		{
			float aspectRatio = width / height;

			float newHeight = height > 1080 ? 1080 : height;

			float newWidth = width > 1920 ? 1080 * aspectRatio : width;
			const float scale = 0.01f; // better as a TOR::Camera member?

			camera.ProjectionMat = glm::orthoRH(
				-newWidth * scale
				, newWidth * scale
				, -newHeight * scale
				, newHeight * scale
				, -1.0f
				, camera.ZFar
			);
		}
		break;
		default:;
		}
	}

#if defined (CONFIG_DEBUG) || defined (CONFIG_DEVELOP)
	// quick and dirty debug code
	auto camEnts = GetCameraEntities();
	if ((m_inputSystem.GetKeyOnce(Frac::KEY_1) || m_inputSystem.GetJoystickComboOnce(m_inputSystem.GetDefaultJoystick(), Frac::JOYSTICK_BUTTON_LB, Frac::JOYSTICK_BUTTON_DPAD_LEFT)) && !camEnts.empty())
	{
		SetMainCameraEntity(camEnts[0]);
		m_registry.GetEnTTRegistry().patch<TOR::Camera>(camEnts[0]);
	}
	else if ((m_inputSystem.GetKeyOnce(Frac::KEY_2) || m_inputSystem.GetJoystickComboOnce(m_inputSystem.GetDefaultJoystick(), Frac::JOYSTICK_BUTTON_LB, Frac::JOYSTICK_BUTTON_DPAD_RIGHT)) && camEnts.size() > 1)
	{
		SetMainCameraEntity(camEnts[1]);
		m_registry.GetEnTTRegistry().patch<TOR::Camera>(camEnts[1]);
	}
#endif
}

void CameraSystem::UpdateDefenseGrid(const float dt, entt::entity e, TOR::Transform& transform, TOR::Camera& camera, CameraTrait& camTrait)
{
	float speedMultiplier = 1.0f;
	if (m_inputSystem.GetAnyJoystickButton(Frac::JOYSTICK_BUTTON_L3) || m_inputSystem.GetKey(Frac::KEY_LEFT_SHIFT))
	{
		speedMultiplier *= 1.5f;
	}

	float horPan = m_inputSystem.GetAnyJoystickAxis(Frac::JoystickAxes::JOYSTICK_AXIS_LEFT_THUMB_HORIZONTAL);
	float verPan = m_inputSystem.GetAnyJoystickAxis(Frac::JoystickAxes::JOYSTICK_AXIS_LEFT_THUMB_VERTICAL);

	if (horPan > -m_deadzoneSize && horPan < m_deadzoneSize && verPan > -m_deadzoneSize && verPan < m_deadzoneSize)
	{
		horPan = -static_cast<int>(m_inputSystem.GetKey(Frac::KEY_A) || m_inputSystem.GetKey(Frac::KEY_LEFT))
			+ static_cast<int>(m_inputSystem.GetKey(Frac::KEY_D) || m_inputSystem.GetKey(Frac::KEY_RIGHT));
		verPan = -static_cast<int>(m_inputSystem.GetKey(Frac::KEY_W) || m_inputSystem.GetKey(Frac::KEY_UP))
			+ static_cast<int>(m_inputSystem.GetKey(Frac::KEY_S) || m_inputSystem.GetKey(Frac::KEY_DOWN));
	}

	horPan *= speedMultiplier;
	verPan *= speedMultiplier;

	// deadzone check
	if (horPan < -m_deadzoneSize || horPan > m_deadzoneSize || verPan < -m_deadzoneSize || verPan > m_deadzoneSize)
	{
		const glm::mat4 lookAtMat = glm::lookAtRH(camTrait.LookAt, transform.Position, m_upVec);

		const glm::vec3 forward = glm::vec3(lookAtMat[0][2], lookAtMat[1][2], lookAtMat[2][2]);
		const glm::vec3 right = glm::vec3(lookAtMat[0][0], lookAtMat[1][0], lookAtMat[2][0]);

		const glm::vec3 flatForward = glm::normalize(glm::vec3(forward.x, 0.f, forward.z)); //normalizing makes sure the vector has a length of 1 instead of less than.
		const glm::vec3 flatMove = (right * -horPan) + (flatForward * -verPan);

		const float scaledMoveSpeed = camTrait.MoveSpeed * dt;

		const glm::vec3 futureLookAt = camTrait.LookAt + glm::vec3(flatMove.x * scaledMoveSpeed, 0.f, flatMove.z * scaledMoveSpeed);

		// clamp position to level, Bounds set in LoadLevel of LevelManager.cpp
		if ((futureLookAt.x > camTrait.MinBounds.x && futureLookAt.z < camTrait.MinBounds.z)
			&& (futureLookAt.x < camTrait.MaxBounds.x && futureLookAt.z > camTrait.MaxBounds.z))
		{
			camTrait.LookAt = futureLookAt;
			transform.Position = transform.Position + flatMove * scaledMoveSpeed;
			transform.Orientation = glm::conjugate(glm::toQuat(glm::lookAt(transform.Position, camTrait.LookAt, { 0.f,1.f,0.f })));
		}
	}
}

void CameraSystem::UpdateNoclip(const float dt, entt::entity e, TOR::Transform& transform, TOR::Camera& camera, CameraTrait& camTrait)
{
	const Frac::MouseState& mouse = m_inputSystem.GetMouse();

	const glm::vec2 mouseVel = ((mouse.m_position - mouse.m_lastPosition) * 1.5f) * dt;

	float lHor = m_inputSystem.GetAnyJoystickAxis(Frac::JoystickAxes::JOYSTICK_AXIS_LEFT_THUMB_HORIZONTAL);
	float lVer = m_inputSystem.GetAnyJoystickAxis(Frac::JoystickAxes::JOYSTICK_AXIS_LEFT_THUMB_VERTICAL);

	const float rHor = m_inputSystem.GetAnyJoystickAxis(Frac::JoystickAxes::JOYSTICK_AXIS_RIGHT_THUMB_HORIZONTAL) + mouseVel.x * static_cast<float>(m_inputSystem.GetMouse().m_down[2]);
	const float rVer = m_inputSystem.GetAnyJoystickAxis(Frac::JoystickAxes::JOYSTICK_AXIS_RIGHT_THUMB_VERTICAL) + mouseVel.y * static_cast<float>(m_inputSystem.GetMouse().m_down[2]);

	const bool multiplySpeed = !m_inputSystem.GetAnyJoystickButton(Frac::JOYSTICK_BUTTON_L3) ? m_inputSystem.GetAnyJoystickButton(Frac::JOYSTICK_BUTTON_Y) : false;

	if (lHor == 0.f || lVer == 0.f)
	{
		lHor = -static_cast<int>(m_inputSystem.GetKey(Frac::KEY_A)) + static_cast<int>(m_inputSystem.GetKey(Frac::KEY_D));
		lVer = -static_cast<int>(m_inputSystem.GetKey(Frac::KEY_W)) + static_cast<int>(m_inputSystem.GetKey(Frac::KEY_S));
	}

	float upDown = 0.f;
	if (m_inputSystem.GetAnyJoystickButton(Frac::JOYSTICK_BUTTON_A) || m_inputSystem.GetKey(Frac::KEY_K))
	{
		upDown = 1.f;
	}
	else if (m_inputSystem.GetAnyJoystickButton(Frac::JOYSTICK_BUTTON_B) || m_inputSystem.GetKey(Frac::KEY_L))
	{
		upDown = -1.f;
	}

	const glm::mat4 lookAtMat = glm::lookAtRH(camTrait.LookAt, transform.Position, m_upVec);

	const glm::vec3 forward = glm::vec3(lookAtMat[0][2], lookAtMat[1][2], lookAtMat[2][2]);
	const glm::vec3 right = glm::vec3(lookAtMat[0][0], lookAtMat[1][0], lookAtMat[2][0]);
	const glm::vec3 up = glm::vec3(lookAtMat[0][1], lookAtMat[1][1], lookAtMat[2][1]);

	const float speedMultiplier = multiplySpeed ? camTrait.NoclipSpeedMultiplier : 1.f;

	transform.Position += (((right * -lHor) + (forward * -lVer) + (up * upDown)) * (camTrait.NoclipSpeed * speedMultiplier)) * dt;

	camTrait.LookAt = transform.Position + glm::vec3(transform.Orientation * glm::vec3(0.f, 0.f, -1.f));

	transform.EulerOrientation += glm::vec3(
		glm::radians<float>(-rVer * camTrait.NoclipRotSpeed),
		glm::radians<float>(-rHor * camTrait.NoclipRotSpeed),
		0.f);

	transform.Orientation = glm::quat(glm::vec3(transform.EulerOrientation));
}