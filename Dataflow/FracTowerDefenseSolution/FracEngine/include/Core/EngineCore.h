#pragma once
#include "Core/EntityRegistry.h"
#include <entt/entt.hpp>

namespace Frac
{
#if defined(PLATFORM_WINDOWS)
	class PCDevice;
#elif defined(PLATFORM_SWITCH)
//Emitted switch code
#endif
	class SceneManager;
	class InputSystem;
	class ITools;
	class DebugRenderer;
	class TextRenderSystem;
	class Renderer;
	class TransformSystemController;
	class MemoryAllocator;
	class EngineConnector;
	class AudioManager;
	class SystemDirector;
	class ResourceManager;
	class ParticleSystem;
	class EngineCore
	{
	public:
		void Initialize();
		void Run();
		void Shutdown();

		void SetRunState(bool runState);

		MemoryAllocator& GetMemoryAllocator();

#if defined(PLATFORM_WINDOWS)
		PCDevice& GetDevice();
#elif defined(PLATFORM_SWITCH)
		//Emitted switch code
#endif
		InputSystem& GetInputSystem();
		TextRenderSystem& GetTextRenderSystem();
		SceneManager& GetSceneManager();
		AudioManager& GetAudioManager();
		SystemDirector& GetSystemDirector();
		ParticleSystem& GetParticleSystem();

#if defined(CONFIG_DEBUG) || defined(CONFIG_DEVELOP)
		Frac::DebugRenderer& GetDebugRenderer();
#endif

		static EngineCore& GetInstance()
		{
			static EngineCore instance;
			return instance;
		}

		// empty user-defined constructor/destructor so we can forward declare classes for smart pointers
		// https://ortogonal.github.io/cpp/forward-declaration-and-smart-pointers/
		EngineCore();
		~EngineCore();

		const entt::registry& GetEnttRegistry() const { return m_entityRegistry->GetEnTTRegistry(); }
		entt::registry& GetEnttRegistry() { return m_entityRegistry->GetEnTTRegistry(); }
		EntityRegistry& GetFracRegistry() { return *m_entityRegistry; }
	private:
		bool m_runState;

		std::unique_ptr<EntityRegistry> m_entityRegistry;
		std::unique_ptr<MemoryAllocator> m_memoryAllocator;
#if defined(PLATFORM_WINDOWS)
		std::unique_ptr<PCDevice> m_device;
#elif defined(PLATFORM_SWITCH)
//Emitted switch code
#endif
		std::unique_ptr<InputSystem> m_inputSystem;
#if defined(CONFIG_DEBUG) || defined(CONFIG_DEVELOP)
		std::unique_ptr<DebugRenderer> m_DebugRenderer;
		std::unique_ptr<ITools> m_tools;
#endif
		std::unique_ptr<Renderer> m_renderSystem;
		std::unique_ptr<SceneManager> m_sceneManager;
		std::unique_ptr<TextRenderSystem> m_textRenderer;
		std::unique_ptr<TransformSystemController> m_transformSystemController;
		std::unique_ptr<AudioManager> m_audioManager;
		std::unique_ptr<SystemDirector> m_systemDirector;
		std::unique_ptr<ResourceManager> m_resourceManager;
		std::unique_ptr<ParticleSystem> m_particleSystem;
	};
} // namespace Frac