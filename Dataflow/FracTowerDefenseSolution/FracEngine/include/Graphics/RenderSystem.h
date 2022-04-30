#pragma once

#include "CoreRenderAPI/RenderAPI.h"
#include "Core/FileIO.h"
#include "Core/EntityRegistry.h"
#include "Core/ISystem.h"

namespace TOR
{
	struct Material;
	struct MaterialBatch;
}

namespace Frac
{
	/// <summary>
	/// The rendersystem renders all the objects using a forward based loop.
	/// </summary>
	class Renderer : public ISystem
	{
	public:
		Renderer(EntityRegistry& entityRegistry);
		~Renderer() = default;

		void Update(float dt) override;

		virtual void Initialize(const std::string& dllName, unsigned viewportWidth, unsigned viewportHeight);
		virtual void Shutdown();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		virtual TOR::RenderAPI& GetRenderAPI() const;

	private:
		void ResizeViewport(unsigned viewportWidth, unsigned viewportHeight);

		void SortRenderables();
		void CreateRenderBatch(TOR::MaterialBatch& newBatch, TOR::Material* material);
		void PushMaterialBatch(const TOR::MaterialBatch& matBatch);
		void SetRenderBatches();
		void SetPrimitiveBatches();

		void SetNewActiveCamera(entt::registry& ref, entt::entity ent);

		EntityRegistry& m_entityRegistry;
		TOR::RenderAPI* m_renderAPI;
		Frac::Entity* m_activeCamera;
	};
}