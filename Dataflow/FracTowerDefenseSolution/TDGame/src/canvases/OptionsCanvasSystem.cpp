#include "TDpch.h"

#include "Canvases/OptionsCanvasSystem.h"

#include "SceneManagement/SceneManager.h"

#include "InputSystem/InputSystem.h"

#include "Audio/AudioManager.h"

OptionsCanvasSystem::OptionsCanvasSystem(Frac::EngineCore& engine, GameDirector* gameDirector, state gameState) :
	CanvasSystem(engine, gameDirector, gameState)
	, m_volumeValue(1.f)
	, m_volumeStep(0.05f)
	, m_volumeTimer(0.f)
	, m_volumeStepWait(0.1f)
	, m_volumeTextEnt(nullptr)
{
}

void OptionsCanvasSystem::Initialise()
{
	CanvasSystem::Initialise();

	m_volumeValue = Frac::EngineCore::GetInstance().GetAudioManager().GetMasterValue();
}

void OptionsCanvasSystem::CreateCanvas()
{
	std::string canvasName = "OptionsCanvas";

	Frac::SceneManager& sceneManager = m_engine.GetSceneManager();

	Frac::Entity& canvasEnt = *(m_canvasEnt = new Frac::Entity(m_fracReg, canvasName));
	m_fracReg.AddComponent<TOR::Transform>(canvasEnt, TOR::Transform({glm::vec3(0.f), glm::vec3(0.f)}));
	m_fracReg.AddComponent<UICanvas>(canvasEnt, UICanvas({canvasName}));
	m_fracReg.AddComponent<Frac::TextComponent>(canvasEnt, Frac::TextComponent());

	canvasEnt.SetParent(sceneManager.GetActiveScene());

	UICanvas& theCanvas = m_fracReg.GetComponent<UICanvas>(canvasEnt);
	{
		Frac::Entity& mmLogoEnt = *new Frac::Entity(m_fracReg, "Logo");
		m_fracReg.AddComponent<TOR::Transform>(mmLogoEnt, TOR::Transform({m_logoPos, m_logoSize}));
		m_fracReg.AddComponent<UIElement>(mmLogoEnt, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(mmLogoEnt, Frac::TextComponent(
			{"Options" // paint icon A
			, 2.f
			, m_primaryButtonColor
			}));

		mmLogoEnt.SetParent(canvasEnt);

		Frac::Entity& optionsAnchor = *new Frac::Entity(m_fracReg, "ButtonAnchor");
		m_fracReg.AddComponent<TOR::Transform>(optionsAnchor, TOR::Transform(
			{
				glm::vec3(0.f, m_buttonVerStartOffset - 20.f, 0.f)
			}));
		m_fracReg.AddComponent<UIElement>(optionsAnchor, UIElement({}));

		optionsAnchor.SetParent(canvasEnt);
		
		// volume toggle
		{
			Frac::Entity& volumeRoot = *new Frac::Entity(m_fracReg, "VolumeToggleText");
			m_fracReg.AddComponent<TOR::Transform>(volumeRoot, TOR::Transform({glm::vec3(0.f, 5.f,0.f)}));
			m_fracReg.AddComponent<UIElement>(volumeRoot, UIElement({"Volume"}));
			m_fracReg.AddComponent<Frac::TextComponent>(volumeRoot, Frac::TextComponent(
				{
					"Volume"
					,m_buttonFontSize
					, theCanvas.IdleColor
				}
			));
			
			volumeRoot.SetParent(optionsAnchor);
			
			Frac::Entity& volumeLeftIndicator = *new Frac::Entity(m_fracReg, "volumeLeftIndicator");
			m_fracReg.AddComponent<TOR::Transform>(volumeLeftIndicator, TOR::Transform(
				{
					glm::vec3(25.f, m_volumeSliderHeightOffset, 0.f)
				}
			));
			m_fracReg.AddComponent<UIElement>(volumeLeftIndicator, UIElement({}));
			m_fracReg.AddComponent<Frac::TextComponent>(volumeLeftIndicator, Frac::TextComponent(
				{"T" // square
				, m_volumeIndicatorSize
				, m_waveIdleCol
				, Frac::TextComponent::FontFamily::Icons
				, false
				}));
			
			volumeLeftIndicator.SetParent(volumeRoot);

			Frac::Entity& volumeRightIndicator = *new Frac::Entity(m_fracReg, "volumeRightIndicator");
			m_fracReg.AddComponent<TOR::Transform>(volumeRightIndicator, TOR::Transform(
				{
					glm::vec3(-25.f, m_volumeSliderHeightOffset, 0.f)
				}
			));
			m_fracReg.AddComponent<UIElement>(volumeRightIndicator, UIElement({}));
			m_fracReg.AddComponent<Frac::TextComponent>(volumeRightIndicator, Frac::TextComponent(
				{"U" // square
				, m_volumeIndicatorSize
				, m_waveIdleCol
				, Frac::TextComponent::FontFamily::Icons
				, false
				}));
			
			volumeRightIndicator.SetParent(volumeRoot);

			Frac::Entity& volumeText = *new Frac::Entity(m_fracReg, "volumeText");
			m_fracReg.AddComponent<TOR::Transform>(volumeText, TOR::Transform(
				{
					glm::vec3(0.f, m_volumeSliderHeightOffset, 0.f)
				}
			));
			m_fracReg.AddComponent<UIElement>(volumeText, UIElement({}));
			m_fracReg.AddComponent<Frac::TextComponent>(volumeText, Frac::TextComponent(
				{"100" // square
				, 0.4f
				, m_waveHighlightCol
				}));
			
			volumeText.SetParent(volumeRoot);

			m_volumeTextEnt = &volumeText;
			
			theCanvas.SelectableElements.push_back(volumeRoot.GetHandle());
		}
		
		// back button
		{
			Frac::Entity& buttonElement = *new Frac::Entity(m_fracReg, "BuildInfo");
			m_fracReg.AddComponent<TOR::Transform>(buttonElement, TOR::Transform(
				{glm::vec3(
					0.f, 
					-90.f, 
					0.f)}
			));

			m_fracReg.AddComponent<UIElement>(buttonElement, UIElement(
				{
					"Back"
				}
			));

			m_fracReg.AddComponent<Frac::TextComponent>(buttonElement, Frac::TextComponent(
				{
					"Back"
					,m_buttonFontSize
				, theCanvas.IdleColor
				}
			));

			buttonElement.SetParent(optionsAnchor);
			
			theCanvas.SelectableElements.push_back(buttonElement.GetHandle());
		}

		Frac::Entity& mmBackdropEnt = *new Frac::Entity(m_fracReg, "Backdrop");
		m_fracReg.AddComponent<TOR::Transform>(mmBackdropEnt, TOR::Transform({m_backdropPos, m_backdropSize}));
		m_fracReg.AddComponent<UIElement>(mmBackdropEnt, UIElement({}));
		m_fracReg.AddComponent<Frac::TextComponent>(mmBackdropEnt, Frac::TextComponent(
			{"S" // Circle
			, m_winLoseBackdropSize
			, m_winBackDropCol
			, Frac::TextComponent::FontFamily::Icons }));

		mmBackdropEnt.SetParent(canvasEnt);

		theCanvas.CurSelectedElement = static_cast<int>(theCanvas.SelectableElements.size()) - 1;
	}
}

void OptionsCanvasSystem::Update(float dt)
{
	CanvasSystem::Update(dt);
	
	UICanvas& canvas = m_fracReg.GetComponent<UICanvas>(*m_canvasEnt);

	UpdateCanvasSelector(dt, canvas);
	
	const auto uiEventName = m_fracReg.GetComponent<UIElement>(m_fracReg.TranslateENTTEntity(canvas.SelectableElements[canvas.CurSelectedElement])).EventName;

	if (uiEventName == "Volume")
	{
		m_volumeTimer += dt;
		
		if (m_volumeTimer > m_volumeStepWait)
		{
			m_volumeTimer = 0.f;

			auto& audioManager = Frac::EngineCore::GetInstance().GetAudioManager();
			if (m_inputSystem.GetKey(Frac::KEY_LEFT) || m_inputSystem.GetKey(Frac::KEY_A)
				|| m_inputSystem.GetAnyJoystickButton(Frac::JOYSTICK_BUTTON_DPAD_LEFT) || m_inputSystem.GetAnyLeft()
				/*|| m_inputSystem.GetAnyJoystickAxis(Frac::JoystickAxes::JOYSTICK_AXIS_LEFT_THUMB_HORIZONTAL) < -0.1f*/)
			{
				m_volumeValue -= m_volumeStep;
			}
			else if (m_inputSystem.GetKey(Frac::KEY_RIGHT) || m_inputSystem.GetKey(Frac::KEY_D)
				|| m_inputSystem.GetAnyJoystickButton(Frac::JOYSTICK_BUTTON_DPAD_RIGHT) || m_inputSystem.GetAnyRight()
				/*|| m_inputSystem.GetAnyJoystickAxis(Frac::JoystickAxes::JOYSTICK_AXIS_LEFT_THUMB_HORIZONTAL) > 0.1f*/)
			{
				m_volumeValue += m_volumeStep;
			}

			m_volumeValue = glm::clamp(m_volumeValue, 0.f, 1.f);

			auto& textComp = m_fracReg.GetComponent<Frac::TextComponent>(*m_volumeTextEnt);
			textComp.text = std::to_string(static_cast<int>(glm::round(m_volumeValue * 100.f)));

			audioManager.SetMasterVolume(m_volumeValue);
		}
	}
	
	if (m_select)
	{

		if (!uiEventName.empty())
		{
			if (uiEventName == "Back")
			{
				LOGINFO("[OptionsCanvasSystem] Restarting Level");
				m_gameDirector->SwitchToGroup(m_gameDirector->GetPreviousState()); //TODO: Reload level. Restart.

				HideCanvas();
			}
		}
		else
		{
			LOGWARNING("CanvasSystem | OptionsCanvas select, uiEventName is empty");
		}
	}
}
