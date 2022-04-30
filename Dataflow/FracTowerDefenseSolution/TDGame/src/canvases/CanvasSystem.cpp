#include "TDpch.h"

#include "Core\EngineCore.h"
#include "Core/Entity.h"

#include "InputSystem/InputSystem.h"

#include "Canvases/CanvasSystem.h"
#include "CameraSystem.h"

#include "SceneManagement/SceneManager.h"

#include "Graphics/TextRenderSystem/TextRenderSystem.h"

#include "Audio/AudioManager.h"
#include "Audio/AudioComponent.h"

CanvasSystem::CanvasSystem(Frac::EngineCore& engine, GameDirector* gameDirector, state gameState) :
	Frac::ISystem(Frac::eStage::OnUpdate)
	, m_engine(engine)
	, m_gameDirector(gameDirector)
	, m_fracReg(engine.GetFracRegistry())
	, m_inputSystem(engine.GetInputSystem())
	, m_activePopupEnt(entt::null)
	, m_activeCamHandle(entt::null)
	, m_firstRun(true)
	, m_inputFirstRun(true)
	, m_updateInput(true)
	, m_isCanvasHidden(true)
{
	m_gameDirector->AssignSystemToStates(
		this
		, gameState);
}

void CanvasSystem::Update(float a_deltaTime)
{
	if (m_fracReg.HasComponent<TOR::Transform>(*m_canvasEnt) && m_fracReg.HasComponent<Frac::TextComponent>(*m_canvasEnt))
	{
		TOR::Transform& transform = m_fracReg.GetComponent<TOR::Transform>(*m_canvasEnt);
		Frac::TextComponent& text = m_fracReg.GetComponent<Frac::TextComponent>(*m_canvasEnt);

		UpdateCanvasTransform(transform, text);

		if (m_updateInput)
			HandleInput();

		UpdateHeightScaling();

		if (m_isCanvasHidden)
			ShowCanvas();
	}

	// hide canvas on request, Release build only hides in the game play phases
#if defined (CONFIG_RELEASE)
	if (m_gameDirector->GetCurrentState() != GameState::PreperationPhase && m_gameDirector->GetCurrentState() != GameState::WavePhase)
		return;
#endif
	
	if (m_inputSystem.GetKeyOnce(Frac::KEY_O) || m_inputSystem.GetAnyJoystickButtonOnce(Frac::JOYSTICK_BUTTON_BACK))
	{
		m_isCanvasManuallyHidden = m_isCanvasManuallyHidden ? false : true;
	}
	if (m_isCanvasManuallyHidden)
		HideCanvas();
}

void CanvasSystem::UpdateCanvasSelector(float dt, UICanvas& canvas)
{
	auto& enttReg = m_fracReg.GetEnTTRegistry();

	entt::entity lastSelectedEnt = canvas.SelectableElements[canvas.LastSelectedElement];
	
	TOR::Transform& lastTrans		= enttReg.get<TOR::Transform>(lastSelectedEnt);
	Frac::TextComponent& lastTextComp = enttReg.get<Frac::TextComponent>(lastSelectedEnt);
	lastTextComp.color = canvas.IdleColor;
	//lastTrans.Scale = glm::vec3(1.f);
	canvas.SelectorAnimTime = 0.f;

	if (m_up)
	{
		canvas.CurSelectedElement--;
		if (m_fracReg.HasComponent<Frac::AudioSourceComponent>(*m_canvasEnt))
		{
			m_engine.GetAudioManager().PlayEvent(m_fracReg.GetComponent<Frac::AudioSourceComponent>(*m_canvasEnt), "event:/UI_Select");
		}
	}
	if (m_down)
	{
		canvas.CurSelectedElement++;
		if (m_fracReg.HasComponent<Frac::AudioSourceComponent>(*m_canvasEnt))
		{
			m_engine.GetAudioManager().PlayEvent(m_fracReg.GetComponent<Frac::AudioSourceComponent>(*m_canvasEnt), "event:/UI_Select");
		}
	}

	if (canvas.CurSelectedElement < 0)
	{
		canvas.CurSelectedElement = static_cast<int>(canvas.SelectableElements.size()) - 1;
	} else if (canvas.CurSelectedElement > canvas.SelectableElements.size() - 1)
	{
		canvas.CurSelectedElement = 0;
	}

	canvas.LastSelectedElement = canvas.CurSelectedElement;
	
	// animate selected element
	TOR::Transform& selectedElementTrans		= enttReg.get<TOR::Transform>(canvas.SelectableElements[canvas.CurSelectedElement]);
	Frac::TextComponent& selectedElementText	= enttReg.get<Frac::TextComponent>(canvas.SelectableElements[canvas.CurSelectedElement]);

	selectedElementText.color = canvas.HoverColor;

	canvas.SelectorAnimTime += dt;
	canvas.SelectorAnimCurrent = glm::cos(canvas.SelectorAnimTime * canvas.SelectorAnimSpeed) * 0.2f;

	//selectedElementTrans.Scale *= canvas.SelectorAnimCurrent + 0.5f;
}

void CanvasSystem::UpdateCanvasTransform(TOR::Transform& transform, Frac::TextComponent& text)
{
	auto& enttReg = m_fracReg.GetEnTTRegistry();

	m_activeCamHandle = GetActiveCamera();
	if (m_activeCamHandle == entt::null
		|| !enttReg.any_of<TOR::Transform>(m_activeCamHandle) 
		|| !enttReg.any_of<CameraSystem::CameraTrait>(m_activeCamHandle))
	{
		return;
	}

	text.faceActiveCamera = true;

	const TOR::Transform& camTrans				= enttReg.get<TOR::Transform>(m_activeCamHandle);
	const CameraSystem::CameraTrait& camTrait	= enttReg.get<CameraSystem::CameraTrait>(m_activeCamHandle);

	const glm::vec3 pointInfrontOfCam = camTrans.Position + 
		(glm::normalize(camTrait.LookAt - camTrans.Position) * m_canvasDepthOffset);

	transform.Position = pointInfrontOfCam;

	m_fracReg.GetEnTTRegistry().patch<TOR::Transform>(m_canvasEnt->GetHandle()); 
}

void CanvasSystem::HandleInput()
{
	if (m_inputFirstRun)
	{
		m_inputFirstRun = false;
		m_select = m_up = m_down = m_pause = false;
		return;
	}
	
	m_select	= m_inputSystem.GetAnyJoystickButtonOnce(Frac::JOYSTICK_BUTTON_A)
		|| m_inputSystem.GetKeyOnce(Frac::KEY_ENTER) || m_inputSystem.GetKeyOnce(Frac::KEY_SPACE);

	m_up		= m_inputSystem.GetAnyUp()
		 || m_inputSystem.GetKeyOnce(Frac::KEY_W) || m_inputSystem.GetKeyOnce(Frac::KEY_UP);
	m_down		= m_inputSystem.GetAnyDown()
		 || m_inputSystem.GetKeyOnce(Frac::KEY_S) || m_inputSystem.GetKeyOnce(Frac::KEY_DOWN);

	m_pause		= m_inputSystem.GetAnyJoystickButtonOnce(Frac::JOYSTICK_BUTTON_START)
		|| m_inputSystem.GetKeyOnce(Frac::KEY_ESCAPE);
}

entt::entity CanvasSystem::GetActiveCamera()
{
	entt::entity activeCam = entt::null;
	m_engine.GetEnttRegistry().view<TOR::Camera>().each(
		[this, &activeCam](TOR::Camera& camera) mutable
	{
		if(camera.IsActive)
		{
			activeCam = entt::to_entity(m_engine.GetEnttRegistry(), camera);
		}
	});

	return activeCam;
}

void CanvasSystem::UpdateHeightScaling() const
{
	if (m_fracReg.HasComponent<TOR::Transform>(*m_canvasEnt))
	{
		const int canvasHeightScale = 768; // TODO playtest to see if this works on all devices

		int heightScale = static_cast<int>(m_engine.GetDevice().GetWindowHeight());
		if (heightScale > 1080) heightScale = 1080;

		const float newHeightScale =  (m_canvasBaseScale + 
			(static_cast<int>(heightScale) - canvasHeightScale) * 0.000075f);

		TOR::Transform& transform = m_fracReg.GetComponent<TOR::Transform>(*m_canvasEnt);

		transform.Scale = glm::vec3(1.f) * newHeightScale;
	}
}

void CanvasSystem::Initialise()
{
	CreateCanvas();
	HideCanvas();

	if (m_fracReg.HasComponent<TOR::Transform>(*m_canvasEnt) && m_fracReg.HasComponent<Frac::TextComponent>(*m_canvasEnt))
	{
		TOR::Transform& transform = m_fracReg.GetComponent<TOR::Transform>(*m_canvasEnt);
		Frac::TextComponent& text = m_fracReg.GetComponent<Frac::TextComponent>(*m_canvasEnt);

		UpdateCanvasTransform(transform, text);

		if (m_fracReg.GetEnTTRegistry().valid(m_activeCamHandle))
			m_engine.GetTextRenderSystem().FaceActiveCamera(m_activeCamHandle, transform, m_canvasEnt->GetHandle());
	}
}

void CanvasSystem::HideCanvas()
{
	m_inputFirstRun = true;
	m_isCanvasHidden = true;
	SetTextChildRenderState(*m_canvasEnt, false);
}

void CanvasSystem::ShowCanvas()
{
	m_isCanvasHidden = false;
	SetTextChildRenderState(*m_canvasEnt, true);
}

void CanvasSystem::SetTextChildRenderState(Frac::Entity& entity, bool activeState) const
{
	if (m_fracReg.HasComponent<Frac::TextComponent>(entity))
	{
		auto& textComp = m_fracReg.GetComponent<Frac::TextComponent>(entity);
		textComp.isActive = activeState;
	}
	
	for(const auto& child : entity.GetReferenceToChildren())
	{
		SetTextChildRenderState(*child, activeState);
	}
}
