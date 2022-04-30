#pragma once

#include "CoreRenderAPI/Renderpasses/IRenderPass.h"
#include "CoreRenderAPI/Instancing/MaterialBatch.h"
#include "CoreRenderAPI/Instancing/PrimitiveBatch.h"

namespace TOR
{
	class PrimitiveCache;

	struct OnStateEnterArgsPBRPass : OnStateEnterArgs {
		OnStateEnterArgsPBRPass(
			const std::vector<std::pair<const DirectionalLight*, glm::mat4x4>>& directionalLights,
			const std::vector<std::pair<const PointLight*, glm::mat4x4>>& pointLights,
			const std::vector<std::pair<const SpotLight*, glm::mat4x4>>& spotLights,
			const Camera& activeCamera,
			const glm::mat4x4& activeCameraPosition,
			const std::forward_list<MaterialBatch>& materialBatches,
			const std::forward_list<PrimitiveBatch>& primitiveBatches,
			const PrimitiveCache& primCache
		) : OnStateEnterArgs(),
			DirectionalLightPairs(directionalLights),
			PointLightPairs(pointLights),
			SpotLightPairs(spotLights),
			ActiveCamera(activeCamera),
			ActiveCameraPosition(activeCameraPosition),
			RenderBatches(materialBatches),
			PrimitiveBatches(primitiveBatches),
			PrimCache(primCache)
		{
		}

		const std::vector<std::pair<const DirectionalLight*, glm::mat4x4>>& DirectionalLightPairs;
		const std::vector<std::pair<const PointLight*, glm::mat4x4>>& PointLightPairs;
		const std::vector<std::pair<const SpotLight*, glm::mat4x4>>& SpotLightPairs;
		const Camera& ActiveCamera;
		const glm::mat4x4& ActiveCameraPosition;
		const std::forward_list<MaterialBatch>& RenderBatches;
		const std::forward_list<PrimitiveBatch>& PrimitiveBatches;
		const PrimitiveCache& PrimCache;
	};

	struct OnStateExitArgsPBRPass : OnStateExitArgs {
	};

	struct OnConstructionArgsPBRPass : OnPassConstructionArgs {
	};

	struct OnDestructionArgsPBRPass : OnPassDestructionArgs {
	};

	class PBRRenderPass : public IRenderPass
	{
	public:

		PBRRenderPass(const std::string& name, TOR::ERenderPass pass);
		~PBRRenderPass();

		void OnStateEnter(OnStateEnterArgs* args) override;

		void OnStateExit(OnStateExitArgs* args) override;

		void Initialize(OnPassConstructionArgs* args) override;

		void Destroy(OnPassDestructionArgs* args) override;

	private:

		LowLevelAPI* m_lowLevelRenderingCore;
	};
}