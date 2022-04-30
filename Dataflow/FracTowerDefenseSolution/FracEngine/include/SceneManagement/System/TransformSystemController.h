#pragma once
#include <Core/ISystem.h>

namespace TOR {
	struct Transform;
}

namespace Frac
{
	class TransformSystemController : public ISystem
	{
	public:
		TransformSystemController();
		~TransformSystemController() = default;
		void Update(float DeltaTime) override;
		private:
		void UpdateTransform(Frac::Entity& entity);
		void SetDirtyFlag(entt::registry& registry, entt::entity entity);
	};

}//namespace Frac
