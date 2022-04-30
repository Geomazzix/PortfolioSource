#pragma once
#include "CoreRenderAPI/Renderpasses/IRenderPass.h"
#include <glm/mat4x4.hpp>
#include "CoreRenderAPI/Instancing/MaterialBatch.h"
#include "CoreRenderAPI/Instancing/PrimitiveBatch.h"

namespace TOR
{
	struct DirectionalLight;

	struct OnStateEnterArgsShadowPass : OnStateEnterArgs
	{
		OnStateEnterArgsShadowPass(std::pair<const DirectionalLight*
			, glm::mat4x4>& sunLight, const std::forward_list<MaterialBatch>& materialBatches
			, const std::forward_list<PrimitiveBatch>& primitiveBatches, const PrimitiveCache& primitiveCache) :
			OnStateEnterArgs(),
			SunLight(sunLight),
			RenderBatches(materialBatches),
			PrimitiveBatches(primitiveBatches),
			PrimCache(primitiveCache)
		{}
		const PrimitiveCache& PrimCache;
		std::pair<const DirectionalLight*, glm::mat4x4>& SunLight;
		const std::forward_list<MaterialBatch>& RenderBatches;
		const std::forward_list<PrimitiveBatch>& PrimitiveBatches;
	};

	//struct OnStateExitArgsShadowPass : OnStateExitArgs {
	//};

	struct OnConstructionArgsShadowPass : OnPassConstructionArgs {
	};

	//struct OnDestructionArgsShadowPass : OnPassDestructionArgs {
	//};

	/// <summary>
	/// The shadow pass creates 1 depth buffer that displays the first loaded directional light of the active scene.
	/// </summary>
	class ShadowPass : public IRenderPass
	{
	public:
		ShadowPass(const std::string& name, ERenderPass renderPass);
		~ShadowPass();

		void Initialize(OnPassConstructionArgs* args) override;
		void Destroy(OnPassDestructionArgs* args) override;

		void OnStateEnter(OnStateEnterArgs* args) override;
		void OnStateExit(OnStateExitArgs* args) override;

	private:
		LowLevelAPI* m_lowLevelRenderingCore;
		glm::mat4 m_viewMatrix;
	};
}