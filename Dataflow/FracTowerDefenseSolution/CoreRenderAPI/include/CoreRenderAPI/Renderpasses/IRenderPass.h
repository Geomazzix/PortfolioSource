#pragma once
#include <string>
#include "CoreRenderAPI/LowLevelAPI.h"

namespace TOR
{
	/// <summary>
	/// Used to identified the available render passes throughout the system.
	/// </summary>
	enum class ERenderPass
	{
		SHADOW = 0,
		PBR,
		POSTPROCESSING,
		//TONEMAPPING,

		COUNT
	};

	//Used as base parameters for the state switches.
	struct OnPassConstructionArgs
	{
		LowLevelAPI* Renderer;
	};

	struct OnPassDestructionArgs {};
	struct OnStateEnterArgs {};
	struct OnStateExitArgs {};

	/// <summary>
	/// The base class for the render passes.
	/// </summary>
	class IRenderPass
	{
	public:
		IRenderPass(const std::string& name, ERenderPass renderPass);
		virtual ~IRenderPass() = default;

		virtual void Initialize(OnPassConstructionArgs* args) = 0;
		virtual void Destroy(OnPassDestructionArgs* args) = 0;

		virtual void OnStateEnter(OnStateEnterArgs* args) = 0;
		virtual void OnStateExit(OnStateExitArgs* args) = 0;

		void SetState(bool state);
		bool IsEnabled() const;

		unsigned int GetId() const;
		const std::string& GetName() const;

	private:
		std::string m_name;
		unsigned int m_id;
		static int s_id;
		bool m_state;
		ERenderPass m_renderPass;
	};
}