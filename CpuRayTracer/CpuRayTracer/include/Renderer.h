#pragma once
#include <vec3.hpp>
#include "Camera.h"
#include "Ray.h"
#include "Light.h"
#include "Scene.h"
#include "Image.h"
#include "ResourceManager.h"
#include "JobSystem.h"
#include "SpacialSubdivision/BVH.h"

namespace CRT
{
	class Scene;

	/// <summary>
	/// Used while rendering DoF and AA.
	/// </summary>
	enum class ESampleRate
	{
		X1 = 1,
		X2 = 2,
		X4 = 4,
		X8 = 8,
		X16 = 16
	};

	/// <summary>
	/// The configuration at which the renderer will create the image.
	/// </summary>
	struct RendererConfig
	{
		RendererConfig() = default;
		~RendererConfig() = default;

		void operator=(const RendererConfig& config)
		{
			BackGroundColor = config.BackGroundColor;
			AASampleRate = config.AASampleRate;
			DoFSampleRate = config.DoFSampleRate;
			MaxRenderDepth = config.MaxRenderDepth;
			EnableShadows = config.EnableShadows;
		}

		glm::vec3 BackGroundColor = glm::vec3(0.2f, 1.0f, 1.0f);
		ESampleRate	AASampleRate = ESampleRate::X4;
		ESampleRate DoFSampleRate = ESampleRate::X4;
		int MaxRenderDepth = 4;
		bool EnableShadows = true;
	};

	/// <summary>
	/// The Renderer renders an image of the scene onto a PNG.
	/// </summary>
	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		void Initialize(std::shared_ptr<JobSystem> jobSystem, int numPixels, const RendererConfig& renderConfig);

		glm::vec3* Render(Scene& scene, glm::vec3 obliqueDirection = glm::vec3(0, 0, -1));

		void SetActiveCamera(int cameraNodeIndex);
		int GetActiveCameraNodeIndex() const;

		void SetSkySphere(std::weak_ptr<Image> skysphere);

	private:
		int m_ActiveCameraIndex;
		float m_OuterRefractiveIndex;
		glm::vec3* m_ColorData;
		std::weak_ptr<Image> m_Skysphere;
		std::shared_ptr<JobSystem> m_Jobsystem;
		RendererConfig m_Config;

		glm::vec3 ComputeRayColor(const Ray& ray, Scene& scene, float maxRayLength, float& absorbDistance, int depth = 0);

		glm::vec3 ApplyRefraction(const Ray& ray, Scene& scene, const HitInfo& hitInfo, float maxRayLength, float& absorbDistance, int depth = 0);
		glm::vec3 ApplyLighting(const Ray& ray, Scene& scene, std::shared_ptr<Material> material, const glm::vec3& hitPoint, const glm::vec3& normal, float maxRayLength);

		void RenderPerspective(Scene& scene);
		void RenderOblique(Scene& scene, const glm::vec3& rayDirection);

		glm::vec3 SampleSky(const Ray& ray);

		float SchlickApproximation(float n1, float n2, const glm::vec3& normal, const glm::vec3& incident, float reflectivity);
	};
}