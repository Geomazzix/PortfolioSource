#include "pch.h"
#include "CoreRenderAPI/Components/Transform.h"
#include "CoreRenderAPI/Components/Camera.h"
#include "CoreRenderAPI/Components/Light.h"
#include "CoreRenderAPI/Components/Material.h"
#include "CoreRenderAPI/Components/Particle.h"
#include "Graphics/RenderSystem.h"
#include "CoreRenderAPI/RenderAPI.h"
#include "SceneManagement/SceneManager.h"
#include "CoreRenderAPI/Instancing/MaterialBatch.h"
#include "CoreRenderAPI/Instancing/PrimitiveBatch.h"

namespace Frac
{
	//------------------------------------------------------------------------------------------------------------------
	//RENDERER
	//------------------------------------------------------------------------------------------------------------------
	Renderer::Renderer(EntityRegistry& entityRegistry) : ISystem(eStage::OnStore),
		m_entityRegistry(entityRegistry), m_renderAPI(nullptr)
	{
		entityRegistry.GetEnTTRegistry().on_update<TOR::Camera>().connect<&Renderer::SetNewActiveCamera>(this);
	}

	void Renderer::Update(float dt)
	{
		// Sort
		SortRenderables();

		// Set data
		SetRenderBatches();

		// Set Particles
		SetPrimitiveBatches();

		// Update Camera Position
		m_renderAPI->SetActiveCameraPosition(m_entityRegistry.GetComponent<TOR::Transform>(*m_activeCamera).ModelTransformData);

		// Render
		m_renderAPI->Render();
		m_renderAPI->ClearLights();
	}

	void Renderer::SortRenderables()
	{
		// #TODO: Do this in 1 sort call. -> Done, check validity
		// First sort by Meshes as they have a lower priority in the renderable batches.
		m_entityRegistry.GetEnTTRegistry().sort<Renderable>([this](const entt::entity lhs, const entt::entity rhs)
			{
				entt::registry& entityRegistry = m_entityRegistry.GetEnTTRegistry();
				Renderable& renderableLhs = entityRegistry.get<Renderable>(lhs);
				Renderable& renderableRhs = entityRegistry.get<Renderable>(rhs);
				return renderableLhs.Mesh < renderableRhs.Mesh;
			});

		// Then sort by Materials so that materials are dominant in the batches rendered. Why? glUseProgram minimalisation.
		m_entityRegistry.GetEnTTRegistry().sort<Renderable>([this](const entt::entity lhs, const entt::entity rhs)
			{
				entt::registry& entityRegistry = m_entityRegistry.GetEnTTRegistry();
				Renderable& renderableLhs = entityRegistry.get<Renderable>(lhs);
				Renderable& renderableRhs = entityRegistry.get<Renderable>(rhs);
				return renderableLhs.Material < renderableRhs.Material&& renderableLhs.Mesh < renderableRhs.Mesh;
			});
	}

	void Renderer::CreateRenderBatch(TOR::MaterialBatch& newBatch, TOR::Material* material)
	{
		newBatch = TOR::MaterialBatch(material);
	}

	void Renderer::PushMaterialBatch(const TOR::MaterialBatch& matBatch)
	{
		m_renderAPI->InsertMaterialBatch(matBatch);
	}

	void Renderer::SetRenderBatches()
	{
		entt::registry& entityRegister = m_entityRegistry.GetEnTTRegistry();
		auto renderableView = entityRegister.view<Renderable>();

		int meshCount = 0;
		TOR::MaterialBatch batchPlaceholder(nullptr);
		const TOR::Mesh* meshPlaceHolder = nullptr;

		auto spotLightview = entityRegister.view<TOR::SpotLight>();
		auto directionLightview = entityRegister.view<TOR::DirectionalLight>();
		directionLightview.each([this, &entityRegister](entt::entity handle, TOR::DirectionalLight& light)
			{
				m_renderAPI->AddNewDirectionalLight(&light, m_entityRegistry.GetComponent<TOR::Transform>(m_entityRegistry.TranslateENTTEntity(handle)).ModelTransformData);
			});
	
#if not defined(PLATFORM_SWITCH)
		auto pointLightview = entityRegister.view<TOR::PointLight>();
		pointLightview.each([this, &entityRegister](entt::entity handle, TOR::PointLight& light)
			{
				m_renderAPI->AddNewPointLight(&light, m_entityRegistry.GetComponent<TOR::Transform>(m_entityRegistry.TranslateENTTEntity(handle)).ModelTransformData);
			});
#endif

		renderableView.each([this, &entityRegister, &meshCount, &batchPlaceholder, &meshPlaceHolder](entt::entity handle, Renderable& renderable)
			{
				if (batchPlaceholder.SurfaceMaterial != renderable.Material)
				{
					if (batchPlaceholder.SurfaceMaterial != nullptr)
					{
						PushMaterialBatch(batchPlaceholder);
					}

					// Create new batch when new material is being looped over in entity view
					CreateRenderBatch(batchPlaceholder, renderable.Material);
					meshCount = 0;
				}

				bool skipDuplicateMesh = false;
				if (meshPlaceHolder != renderable.Mesh) //works because the renderables are sorted? - double check this.
				{
					if (std::find(batchPlaceholder.Meshes.begin(), batchPlaceholder.Meshes.end(), renderable.Mesh) == batchPlaceholder.Meshes.end())
					{
						batchPlaceholder.Meshes.push_back(renderable.Mesh);
						meshCount++;
						batchPlaceholder.MeshTransforms.push_back(std::vector<glm::mat4x4>());
						batchPlaceholder.MeshNormalTransforms.push_back(std::vector<glm::mat3x3>());
						meshPlaceHolder = renderable.Mesh;
					}
					else {
						int meshFindIndex = -1;
						for (int k = 0; k < batchPlaceholder.Meshes.size(); ++k)
						{
							if (batchPlaceholder.Meshes[k] == renderable.Mesh)
							{
								meshFindIndex = k;
							}
						}
						batchPlaceholder.MeshTransforms[meshFindIndex].push_back(entityRegister.get<TOR::Transform>(handle).ModelTransformData);
						batchPlaceholder.MeshNormalTransforms[meshFindIndex].push_back(glm::transpose(glm::inverse(glm::mat3(entityRegister.get<TOR::Transform>(handle).ModelTransformData))));
						skipDuplicateMesh = true;
					}
				}
				if (!skipDuplicateMesh) {
					batchPlaceholder.MeshTransforms[meshCount - 1].push_back(entityRegister.get<TOR::Transform>(handle).ModelTransformData);
					batchPlaceholder.MeshNormalTransforms[meshCount - 1].push_back(glm::transpose(glm::inverse(glm::mat3(entityRegister.get<TOR::Transform>(handle).ModelTransformData))));
				}
			});
		if (!batchPlaceholder.Meshes.empty())
		{
			// push last mat batch (batches are normally pushed when a new batch is created in the entity loop,
			// since the last batch doesn't come there we need to call this one solely)
			PushMaterialBatch(batchPlaceholder);
		}
	}

	void Renderer::SetNewActiveCamera(entt::registry& ref, entt::entity ent)
	{
		m_renderAPI->SetActiveCamera(&ref.get<TOR::Camera>(ent));
		m_activeCamera = &m_entityRegistry.TranslateENTTEntity(ent);
	}

	void Renderer::SetPrimitiveBatches()
	{
		entt::registry& entityRegister = m_entityRegistry.GetEnTTRegistry();
		// Sort particles on primitive shape.
		/*entityRegister.sort<TOR::Particle>([this, &entityRegister](const entt::entity lhs, const entt::entity rhs)
			{
				TOR::Particle& particleLhs = entityRegister.get<TOR::Particle>(lhs);
				TOR::Particle& particleRhs = entityRegister.get<TOR::Particle>(rhs);
				return particleLhs.ParticleShape < particleRhs.ParticleShape;
			});*/

			// Set particles in render API

			// PrimitiveBatch for the cube shaped particles
		TOR::PrimitiveBatch primitiveBatchCubeholder(TOR::PrimitiveShape::CUBE);

		auto particleView = entityRegister.view<TOR::Particle>();
		particleView.each([this, &entityRegister, &primitiveBatchCubeholder](entt::entity handle, TOR::Particle& particle) {
			float ageRatio = particle.Age / particle.TotalLifetime;
			glm::mat4x4 transformOfParticle = glm::identity<glm::mat4x4>();
			const glm::mat4 t = glm::translate(glm::mat4{ 1.f }, particle.Position);
			const glm::mat4 r = glm::toMat4(particle.Rotation);
			glm::vec3 scaleOfParticle = glm::mix(particle.StartSize, particle.EndSize, ageRatio);
			const glm::mat4 s = glm::scale(glm::mat4{ 1.f }, scaleOfParticle);
			transformOfParticle = (t * r * s);

			// IF EMISSIVE (see PrimitiveBatch.h)
			transformOfParticle[0][3] = particle.IsEmissive ? 1 : 0;

			glm::vec4 particleColor = glm::mix(particle.StartColor, particle.EndColor, ageRatio);

			switch (particle.ParticleShape)
			{
			case TOR::PrimitiveShape::CUBE:
			{
				primitiveBatchCubeholder.PrimitiveTransforms.push_back(transformOfParticle);
				//primitiveBatchCubeholder.PrimitiveNormalTransforms.push_back(glm::transpose(glm::inverse(glm::mat3(transformOfParticle))));
				primitiveBatchCubeholder.PrimitiveColors.push_back(particleColor);
				break;
			}
			}
			});
		if (primitiveBatchCubeholder.PrimitiveColors.size() > 0)
			m_renderAPI->InsertPrimitiveBatch(primitiveBatchCubeholder);
	}

	void Renderer::Initialize(const std::string& dllName, unsigned viewportWidth, unsigned viewportHeight)
	{
		m_renderAPI = TOR::RenderAPI::CreateRenderAPI(dllName.c_str());
		m_renderAPI->Initialize(FileIO::GetPathFromWildcard("[Assets]"), viewportWidth, viewportHeight);
		m_renderAPI->CreatePipelineStage("TDRenderPipeline", true);
	}

	void Renderer::Shutdown()
	{
		m_renderAPI->Shutdown();
	}

	void Renderer::ResizeViewport(unsigned viewportWidth, unsigned viewportHeight)
	{
		m_renderAPI->ResizeViewport(viewportWidth, viewportHeight);
	}

	TOR::RenderAPI& Renderer::GetRenderAPI() const
	{
		return *m_renderAPI;
	}
}