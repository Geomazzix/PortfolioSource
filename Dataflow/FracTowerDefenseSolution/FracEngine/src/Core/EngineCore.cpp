#include "pch.h"
#include "Core\EngineCore.h"
#include "Core/MemoryAllocator.h"
#include "SceneManagement/System/TransformSystemController.h"
#include "Graphics/RenderSystem.h"
#include "Graphics/ParticleRenderSystem/ParticleSystem.h"
#include "Graphics/TextRenderSystem/TextRenderSystem.h"
#include "SceneManagement/SceneManager.h"
#include "Audio/AudioManager.h"
#include <Core/SystemDirector.h>
#include <Utility/timer.h>
#include "Core/SystemDirector.h"
#include "Core/ResourceBank.h"

#ifdef PLATFORM_WINDOWS
#include "Platform/PC/PCDevice.h"
#include "Platform/PC/GLFWInput.h"
#elif PLATFORM_SWITCH
//Emitted switch code
#endif // PLATFORM_SWITCH

#if defined(CONFIG_DEBUG) || defined(CONFIG_DEVELOP)
#include "Graphics/DebugRenderer.h"
#ifdef PLATFORM_WINDOWS
#include "Tools/PC/PCTools.h"
#elif PLATFORM_SWITCH
//Emitted switch code
#endif // PLATFORM_SWITCH
#endif // CONFIG_DEBUG || CONFIG_DEVELOP

namespace Frac
{
	void EngineCore::Initialize()
	{
		if (m_runState)
			return;

		//Core programs.
		m_runState = true;
		m_entityRegistry = std::make_unique<EntityRegistry>();
		FileIO::Initialize();
		m_systemDirector = std::make_unique<SystemDirector>();
		m_memoryAllocator = std::make_unique<MemoryAllocator>();

		// Render system creation, always pick one - initialization happens later.
		m_renderSystem = std::make_unique<Renderer>(*m_entityRegistry.get());
		m_resourceManager = std::make_unique<ResourceManager>(m_renderSystem->GetRenderAPI(), *m_entityRegistry);
		m_sceneManager = std::make_unique<SceneManager>(*m_renderSystem.get(), *m_entityRegistry.get(), *m_resourceManager);
		m_textRenderer = std::make_unique<TextRenderSystem>(this, m_entityRegistry->GetEnTTRegistry());
		//Platform specific
#ifdef PLATFORM_WINDOWS
		Frac::Logger::GetInstance().Initialize("Log/FracLog.txt"); //Located in TDGame Files.
		m_device = std::make_unique<PCDevice>(*m_renderSystem.get(), *m_textRenderer.get());
		m_device->Initialize(1920, 1080);
		m_inputSystem = std::make_unique<GLFWInput>(m_device.get()->GetWindow());

#elif PLATFORM_SWITCH
//Emitted switch code
#endif // PLATFORM_SWITCH

		//Platform specific rendering.
#if defined(CONFIG_DEBUG) || defined(CONFIG_DEVELOP)
		m_DebugRenderer = std::make_unique<DebugRenderer>();
#ifdef PLATFORM_WINDOWS
		m_tools = std::make_unique<PCTools>();
#elif PLATFORM_SWITCH
		//Emitted switch code
#endif // PLATFORM_SWITCH
#endif // CONFIG_DEBUG || CONFIG_DEVELOP
		int windowWidth = m_device->GetWindowWidth();
		int windowHeight = m_device->GetWindowHeight();
#ifdef PLATFORM_WINDOWS
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		float xscale = 0, yscale = 0;
		glfwGetMonitorContentScale(monitor, &xscale, &yscale);
		windowWidth *= xscale;
		windowHeight *= yscale;
#endif
		m_renderSystem->Initialize("OpenGLRenderer", windowWidth, windowHeight);
		m_textRenderer->Initialize(windowWidth, windowHeight);
		
		m_particleSystem = std::make_unique<ParticleSystem>(*m_entityRegistry, *m_renderSystem);
		m_transformSystemController = std::make_unique<TransformSystemController>();

		m_audioManager = std::make_unique<AudioManager>();
	}

	void EngineCore::Run()
	{
		Frac::Timer timer;
		float deltaTime = 0.0f;
		while (!m_device->ShouldClose() && m_runState)
		{
			timer.Reset();
			m_device->PollEvents();
			m_device->SwapBuffers();
			m_inputSystem->Update(deltaTime);
			m_systemDirector->Update(deltaTime);
#if defined(CONFIG_DEBUG) || defined(CONFIG_DEVELOP)
			m_tools.get()->Update();
#endif // CONFIG_DEBUG || CONFIG_DEVELOP
			deltaTime = timer.GetTime();
		}
	}

	MemoryAllocator& EngineCore::GetMemoryAllocator()
	{
		return *m_memoryAllocator;
	}

#if defined(PLATFORM_WINDOWS)
	PCDevice& EngineCore::GetDevice()
#elif defined(PLATFORM_SWITCH)
	//Emitted switch code
#endif
	{
		return *m_device;
	}

	InputSystem& EngineCore::GetInputSystem()
	{
		return *m_inputSystem;
	}

	TextRenderSystem& EngineCore::GetTextRenderSystem()
	{
		return *m_textRenderer;
	}

	SceneManager& EngineCore::GetSceneManager() {
		return *m_sceneManager;
	}

	AudioManager& EngineCore::GetAudioManager()
	{
		return *m_audioManager;
	}

	SystemDirector& EngineCore::GetSystemDirector()
	{
		return *m_systemDirector;
	}

	Frac::ParticleSystem& EngineCore::GetParticleSystem()
	{
		return *m_particleSystem;
	}

	void EngineCore::Shutdown()
	{
		m_renderSystem->Shutdown();
		Frac::Logger::GetInstance().Shutdown();
		m_runState = false;
	}

	void EngineCore::SetRunState(bool runState)
	{
		m_runState = runState;
	}

	// empty user-defined constructor/destructor so we can forward declare classes for smart pointers
	// https://ortogonal.github.io/cpp/forward-declaration-and-smart-pointers/
	EngineCore::EngineCore() { }
	EngineCore::~EngineCore() { }

#if defined(CONFIG_DEBUG) || defined(CONFIG_DEVELOP)
	DebugRenderer& EngineCore::GetDebugRenderer()
	{
		return *m_DebugRenderer.get();
	}
#endif
} // namespace Frac