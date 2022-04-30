#pragma once
#include <stdint.h>

#include "EngineCore.h"

namespace Frac
{
	class SystemDirector;
	enum class eStage
	{
		PreFrame,
		OnLoad,
		PostLoad,
		PreUpdate,
		OnUpdate,
		PostUpdate,
		OnStore,
		PostStore,
		PostFrame,
		STAGE_MAX
	};
	struct SystemInfo
	{
		uint64_t ID = UINT64_MAX;
		bool enabled = true;
		eStage stage = eStage::OnUpdate;
	};
	class ISystem
	{
	public:
		ISystem(eStage stage);
		virtual ~ISystem();
		/**
		 * \brief systems should be moved and not copied
		 */
		ISystem(const ISystem& sys) = delete;
		ISystem(ISystem&& sys) = default;
		virtual void Update(float DeltaTime) = 0;
		[[nodiscard]] const SystemInfo& GetSystemInfo() const;
		void SetSystemInfo(const SystemInfo& info);
	protected:
		SystemInfo m_info;
	};
}
