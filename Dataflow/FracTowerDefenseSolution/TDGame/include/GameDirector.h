#pragma once
#include "Core/SystemGroup.h"
typedef uint16_t state;
namespace GameState
{
	constexpr state Default = 0;
	constexpr state PreperationPhase = 1 << 0;
	constexpr state WavePhase = 1 << 1;
	constexpr state PauseState = 1 << 2;
	constexpr state WinState = 1 << 3;
	constexpr state LoseState = 1 << 4;
	constexpr state MainMenuState = 1 << 5;
	constexpr state QuitGameState = 1 << 6;
	constexpr state LoadGameState = 1 << 7;
	constexpr state OptionsState = 1 << 8;
	constexpr state CreditsState = 1 << 9;
	constexpr state NextLevelState = 1 << 10;
	constexpr state TutorialState = 1 << 11;
	constexpr state AllState = std::numeric_limits<state>::max();
}

class GameDirector
{
public:
	GameDirector() = default;
	~GameDirector() = default;
	void AssignSystemToStates(Frac::ISystem* system, state states, bool isEnabledAtStart = false);
	void RemoveSystemFromStates(Frac::ISystem* system, state states);
	void SwitchToGroup(state stateToSwitchTo);
	state GetCurrentState();
	state GetPreviousState();
	
private:
	static constexpr uint64_t s_maxNumberOfStates{sizeof(state) * 8};
	std::array<Frac::SystemGroup, s_maxNumberOfStates + 1> m_states;
	state m_currentStateIndex = GameState::Default;
	state m_currentState = GameState::Default;
	state m_previousState = GameState::Default;
};

