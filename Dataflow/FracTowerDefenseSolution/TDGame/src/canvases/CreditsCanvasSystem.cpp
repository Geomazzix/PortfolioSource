#include "TDpch.h"

#include "Canvases/CreditsCanvasSystem.h"

#include "SceneManagement/SceneManager.h"

#include "CameraSystem.h"

#include "InputSystem/InputSystem.h"

#include "Audio/AudioComponent.h"
#include "Audio/AudioManager.h"
#include "Core/EngineCore.h"
#include "Graphics/ParticleRenderSystem/ParticleSystem.h"

CreditsCanvasSystem::CreditsCanvasSystem(CameraSystem* camSystem, Frac::EngineCore& engine, GameDirector* gameDirector, state gameState) :
	CanvasSystem(engine, gameDirector, gameState)
	, m_camSystem(camSystem)
	, m_crawlerAnchorEnt(nullptr)
	, m_skipAnchorEnt(nullptr)
	, m_skipGraphicEnt(nullptr)
	, m_timer(0.f)
	, m_inputActivateTarget(0.5f)
	, m_crawlerSpeed(8.f)
	, m_timeUntilSkip(145.f)
	, m_timeUntilSkipGraphic(5.f)
	, m_skipGraphicIsVisible(false)
	, m_startPos(glm::vec3(0.f, -70.f, 65.f))
{
}

void CreditsCanvasSystem::Initialise()
{
	m_roles = std::vector<std::pair<std::string, char>>({
		{"Engine Programmer",		11}
		, {"Graphics Programmer",	12}
		, {"Tools Programmer",		13}
		, {"Gameplay Programmer",	14}
		, {"AI Programmer",			17}
		, {"Procedural Artist",		15}
		, {"UI Programmer",			16}
		, {"Special Thanks",		22}
		, {"Scrum Master",			1}
		, {"Product Owner",			0}
		});

	m_names = std::vector<std::pair<std::string, std::vector<char>>>({
		// Current team
		  { "Niels Voskens",		{15}}
		, {"Levi de Koning",		{11, 14}}
		, {"Danny Kruiswijk",		{11, 13}}
		, {"Niels van der Marel",	{14}}
		, {"Tom Lemmers",			{1, 11, 12}}
		, {"Antreas Christodoulou", {14, 17}}
		, {"Olivier Candel",		{0, 11, 12}}
		, {"Dion van der Wateren",	{16, 11}}
		// Special Thanks
		, {"Bojan Endrovski",		{22}}
		, {"Vlad Cenusa",			{22}}
		, {"Fabian Akker",			{22}}
		});

	m_canvasBaseScale = 1.f;

	CanvasSystem::Initialise();
}

void CreditsCanvasSystem::CreateCanvas()
{
	std::string canvasName = "CreditsCanvas";

	Frac::SceneManager& sceneManager = m_engine.GetSceneManager();

	Frac::Entity& canvasEnt = *(m_canvasEnt = new Frac::Entity(m_fracReg, canvasName));
	m_fracReg.AddComponent<TOR::Transform>(canvasEnt, TOR::Transform({ glm::vec3(0.f) }));
	m_fracReg.AddComponent<UICanvas>(canvasEnt, UICanvas({ canvasName }));
	m_fracReg.AddComponent<Frac::TextComponent>(canvasEnt, Frac::TextComponent());

	canvasEnt.SetParent(sceneManager.GetActiveScene());

	UICanvas& theCanvas = m_fracReg.GetComponent<UICanvas>(canvasEnt);

	Frac::Entity& skipAnchor = *new Frac::Entity(m_fracReg, "skipAnchor");
	m_fracReg.AddComponent<TOR::Transform>(skipAnchor, TOR::Transform({ glm::vec3(-190.f, -125.f, 200.f)}));
	m_fracReg.AddComponent<UIElement>(skipAnchor, UIElement({}));

	skipAnchor.SetParent(canvasEnt);

	Frac::Entity& skipText = *new Frac::Entity(m_fracReg, "skipText");
	m_fracReg.AddComponent<TOR::Transform>(skipText, TOR::Transform({ glm::vec3(0.f, 0.f, 0.f)}));
	m_fracReg.AddComponent<UIElement>(skipText, UIElement({}));
	m_fracReg.AddComponent<Frac::TextComponent>(skipText, Frac::TextComponent(
		{
			"to skip"
			,0.15f
		, glm::vec3(1.f, 1.f, 0.f)
		}));

	skipText.SetParent(skipAnchor);

	Frac::Entity& skipIcon = *new Frac::Entity(m_fracReg, "SkipIcon");
	m_fracReg.AddComponent<TOR::Transform>(skipIcon, TOR::Transform({ glm::vec3(20.f, 0.f, 0.f)}));
	m_fracReg.AddComponent<UIElement>(skipIcon, UIElement({}));
	m_fracReg.AddComponent<Frac::TextComponent>(skipIcon, Frac::TextComponent(
		{
			"-"
			,0.015f
			, glm::vec3(1.f, 1.f, 1.f)
			, Frac::TextComponent::FontFamily::Icons
		}));

	skipIcon.SetParent(skipAnchor);

	m_skipAnchorEnt = &skipAnchor;
	m_skipGraphicEnt = &skipIcon;
	
	{
		Frac::Entity& crawlerAnchor = *new Frac::Entity(m_fracReg, "CrawlerAnchor");

		m_fracReg.AddComponent<TOR::Transform>(crawlerAnchor, TOR::Transform({ m_startPos, glm::vec3(0.3f) }));
		m_fracReg.AddComponent<UIElement>(crawlerAnchor, UIElement({}));

		m_crawlerAnchorEnt = &crawlerAnchor;
		TOR::Transform& transform = m_fracReg.GetComponent<TOR::Transform>(crawlerAnchor);
		transform.EulerOrientation += glm::vec3(
			glm::radians<float>(45.f),
			0.f,
			0.f);
		transform.Orientation = glm::quat(glm::vec3(transform.EulerOrientation));

		crawlerAnchor.SetParent(canvasEnt);

		{
			Frac::Entity& infoBox = *new Frac::Entity(m_fracReg, "Header");
			m_fracReg.AddComponent<TOR::Transform>(infoBox, TOR::Transform({ glm::vec3(0.f, 50.f, m_buttonDepth), glm::vec3(1.f) }));
			m_fracReg.AddComponent<UIElement>(infoBox, UIElement({}));
			m_fracReg.AddComponent<Frac::TextComponent>(infoBox, Frac::TextComponent(
				{
					"DataFlow | Credits"
					,0.3f
				, glm::vec3(1.f, 1.f, 0.f)
				}));

			infoBox.SetParent(crawlerAnchor);
		}

		float anchorHeightOffset = 0.f;
		for (auto& role : m_roles)
		{
			// get names with a specific role
			std::vector<std::string> m_namesWithRole;
			for (auto& name : m_names)
			{
				auto it = std::find(name.second.begin(), name.second.end(), role.second);
				if (it != name.second.end())
				{
					m_namesWithRole.emplace_back(name.first);
				}
			}

			if (m_namesWithRole.empty())
				continue;

			// set-up entities
			Frac::Entity& roleAnchor = *new Frac::Entity(m_fracReg, "roleAnchor_" + role.first);
			m_fracReg.AddComponent<TOR::Transform>(roleAnchor, TOR::Transform({ glm::vec3(0.f, anchorHeightOffset, m_buttonDepth) }));
			m_fracReg.AddComponent<UIElement>(roleAnchor, UIElement({}));

			roleAnchor.SetParent(crawlerAnchor);

			Frac::TextComponent nameTextComp({
					""
					,0.2f
				});

			nameTextComp.text = role.first;
			nameTextComp.color = m_roleColor;
			nameTextComp.alignment = Frac::TextComponent::Alignment::Right;

			Frac::Entity& leftRoleElement = *new Frac::Entity(m_fracReg, "leftRoleElement_" + role.first);
			m_fracReg.AddComponent<TOR::Transform>(leftRoleElement, TOR::Transform({ glm::vec3(m_roleNameVerOffset, 0.f, 0.f) }));
			m_fracReg.AddComponent<UIElement>(leftRoleElement, UIElement({}));
			m_fracReg.AddComponent<Frac::TextComponent>(leftRoleElement, nameTextComp);

			leftRoleElement.SetParent(roleAnchor);

			nameTextComp.color = m_nameColor;
			nameTextComp.alignment = Frac::TextComponent::Alignment::Left;

			float heightProgress = 0.f;
			for (size_t i = 0; i < m_namesWithRole.size(); i++)
			{
				nameTextComp.text = m_namesWithRole[i];

				Frac::Entity& rightNameElement = *new Frac::Entity(m_fracReg, "rightNameElement_" + nameTextComp.text);
				m_fracReg.AddComponent<TOR::Transform>(rightNameElement, TOR::Transform({ glm::vec3(-m_roleNameVerOffset, -heightProgress, 0.f) }));
				m_fracReg.AddComponent<UIElement>(rightNameElement, UIElement({}));
				m_fracReg.AddComponent<Frac::TextComponent>(rightNameElement, nameTextComp);

				rightNameElement.SetParent(roleAnchor);

				heightProgress += m_nameHeightOffset;
			}

			anchorHeightOffset -= heightProgress + m_roleHeightOffset;
		}
	}
}

void CreditsCanvasSystem::Update(float dt)
{
	CanvasSystem::Update(dt);

	if (m_firstRun)
	{
		m_camSystem->SetMainCameraEntity(m_camSystem->GetDefaultCameraEnt()->GetHandle());

		m_fracReg.GetEnTTRegistry().view<TOR::Transform, Frac::AudioSourceComponent>().each([this, dt](const auto entity, TOR::Transform& trans, Frac::AudioSourceComponent& audio)
		{
			if (m_engine.GetAudioManager().HasEvent(audio, "event:/BackgroundMusic")) {
				m_engine.GetAudioManager().StopEvent(audio, "event:/BackgroundMusic");
				m_engine.GetAudioManager().PlayEvent(audio, "event:/CreditsMusic");
			}
		});
		
		SetTextChildRenderState(*m_skipAnchorEnt, false);

		m_firstRun = false;

		Frac::FountainParticleEmitterSettings settings;

		settings.EndColor = glm::vec4(1, 1, 0.6, 1);
		settings.StartColor = glm::vec4(1, 1, 0.3, 1);
		settings.HasGravity = false;
		settings.InitialAngleOffset = 93;
		settings.ParticleLifetime = 40.f;
		//settings.PrimitiveShapeToEmit = TOR::PrimitiveShape::CUBE;
		settings.IsEmissive = true;
		settings.FountainHeight = 12;
		settings.TimePerEmit = 0.02f;
		settings.TimeToEmit = 40.f;
		settings.StartSize = glm::vec3(0.02, 0.02, 0.02);
		settings.Endsize = glm::vec3(static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 0.1)) + 0.04, static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 0.1)) + 0.04, static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 0.1)) + 0.04);
		//settings.

		int outOfBounds = 100;
		int extraAngle = 12;

		Frac::EngineCore::GetInstance().GetParticleSystem().SpawnFountainEmitter(settings, glm::vec3(-70 - outOfBounds, 45, -90));
		//settings.HasGravity = true;

		settings.InitialAngleOffset = 73 + extraAngle;
		Frac::EngineCore::GetInstance().GetParticleSystem().SpawnFountainEmitter(settings, glm::vec3(-35 - outOfBounds, 45, -60));
		settings.InitialAngleOffset = 76 + extraAngle;
		Frac::EngineCore::GetInstance().GetParticleSystem().SpawnFountainEmitter(settings, glm::vec3(35 + outOfBounds, 45, -80));
		settings.InitialAngleOffset = 81 + extraAngle;
		Frac::EngineCore::GetInstance().GetParticleSystem().SpawnFountainEmitter(settings, glm::vec3(70 + outOfBounds, 45, -60));
		settings.InitialAngleOffset = 75;
		Frac::EngineCore::GetInstance().GetParticleSystem().SpawnFountainEmitter(settings, glm::vec3(-70 - outOfBounds, -45, -70));
		settings.InitialAngleOffset = 90;

		Frac::EngineCore::GetInstance().GetParticleSystem().SpawnFountainEmitter(settings, glm::vec3(0, 45, -40));

		//settings.HasGravity = true;

		settings.InitialAngleOffset = 82;
		Frac::EngineCore::GetInstance().GetParticleSystem().SpawnFountainEmitter(settings, glm::vec3(-35 - outOfBounds, -45, -40));
		settings.InitialAngleOffset = 74;
		Frac::EngineCore::GetInstance().GetParticleSystem().SpawnFountainEmitter(settings, glm::vec3(35 + outOfBounds, -45, -80));
		settings.InitialAngleOffset = 80;
		Frac::EngineCore::GetInstance().GetParticleSystem().SpawnFountainEmitter(settings, glm::vec3(70 + outOfBounds, -45, -60));
		settings.InitialAngleOffset = 90;
		Frac::EngineCore::GetInstance().GetParticleSystem().SpawnFountainEmitter(settings, glm::vec3(0, -45, -40));
	}

	UpdateButtonLayout();
	
	m_timer += dt;
	
	if (m_timer > m_timeUntilSkipGraphic && !m_skipGraphicIsVisible)
	{
		m_skipGraphicIsVisible = true;
		SetTextChildRenderState(*m_skipAnchorEnt, true);
	}
	
	if (m_timer < m_inputActivateTarget)
	{
		return;
	}
	
	TOR::Transform& trans = m_fracReg.GetComponent<TOR::Transform>(*m_crawlerAnchorEnt);
	const glm::mat4 inverted = glm::inverse(trans.ModelTransformData);
	const glm::vec3 up = normalize(glm::vec3(inverted[2]));

	trans.Position += -up * (m_crawlerSpeed * 0.5f * dt);

	if (m_inputSystem.GetKey(Frac::KEY_ENTER) || m_inputSystem.GetKey(Frac::KEY_SPACE) || m_select || m_timer > m_timeUntilSkip) // skip credits
	{
		if (m_gameDirector->GetPreviousState() == GameState::NextLevelState)
		{
			LOGINFO("[CreditsCanvasSystem] Entering WinState");
			m_gameDirector->SwitchToGroup(GameState::WinState);
		}
		else
		{
			LOGINFO("[CreditsCanvasSystem] Entering Main Menu Phase");
			m_gameDirector->SwitchToGroup(GameState::MainMenuState);
		}
		Frac::EngineCore::GetInstance().GetParticleSystem().Reset();
		HideCanvas();

		Reset();

		m_fracReg.GetEnTTRegistry().view<TOR::Transform, Frac::AudioSourceComponent>().each([this, dt](const auto entity, TOR::Transform& trans, Frac::AudioSourceComponent& audio)
			{
				if (m_engine.GetAudioManager().HasEvent(audio, "event:/BackgroundMusic")) {
					m_engine.GetAudioManager().StopEvent(audio, "event:/CreditsMusic");
					m_engine.GetAudioManager().PlayEvent(audio, "event:/BackgroundMusic");
				}
			});
	}
}

void CreditsCanvasSystem::UpdateButtonLayout() const
{
#if defined(PLATFORM_WINDOWS)
	auto& skipGraphicText = m_fracReg.GetComponent<Frac::TextComponent>(*m_skipGraphicEnt);
	
	switch (m_inputSystem.GetLastUsedInputType())
	{
		case Frac::JoystickType::JOYSTICK_TYPE_XBOX:
		case Frac::JoystickType::JOYSTICK_TYPE_XBOX_360:
			{
				skipGraphicText.text = ",";
			}
		break;
		case Frac::JoystickType::JOYSTICK_TYPE_PLAYSTATION:
		case Frac::JoystickType::JOYSTICK_TYPE_STEAM:
			{
				skipGraphicText.text = ")";
			}
			break;
		case Frac::JoystickType::JOYSTICK_TYPE_NINTENDO_LEFT_JOYCON:
		case Frac::JoystickType::JOYSTICK_TYPE_NINTENDO_RIGHT_JOYCON:
		case Frac::JoystickType::JOYSTICK_TYPE_NINTENDO_DUAL_JOYCONS:
		case Frac::JoystickType::JOYSTICK_TYPE_NINTENDO_FULLKEY:
		case Frac::JoystickType::JOYSTICK_TYPE_NINTENDO_HANDHELD:
			{
				skipGraphicText.text = "-";
			}
			break;
		case Frac::JoystickType::JOYSTICK_TYPE_VIRTUAL:
		case Frac::JoystickType::JOYSTICK_TYPE_INVALID:
			{
				skipGraphicText.text = "W";
			}
		break;
		default: ;
	}
#endif
}

void CreditsCanvasSystem::Reset()
{
	m_timer = 0.f;

	TOR::Transform& trans = m_fracReg.GetComponent<TOR::Transform>(*m_crawlerAnchorEnt);
	trans.Position = m_startPos;

	m_camSystem->SetMainCameraEntity(m_camSystem->GetPreviousActiveCamEnt()->GetHandle());

	SetTextChildRenderState(*m_skipAnchorEnt, false);
	m_skipGraphicIsVisible = false;
	m_firstRun = true;
}
