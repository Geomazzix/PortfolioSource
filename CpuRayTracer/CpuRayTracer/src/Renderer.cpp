#include "Renderer.h"
#include "Primitives/Primitive.h"
#include "Transform.h"
#include "Camera.h"
#include "Scene.h"
#include "DialetricTable.h"
#include <ctime>
#include "Utility/ProgressReporter.h"

//#define NORMAL_COLORING


namespace CRT
{
	Renderer::Renderer() : 
		m_ColorData(nullptr),
		m_Jobsystem(),
		m_Skysphere()
	{}

	Renderer::~Renderer()
	{
		delete[] m_ColorData;
	}

	void Renderer::Initialize(std::shared_ptr<JobSystem> jobSystem, int numPixels, const RendererConfig& renderConfig)
	{
		m_Config = renderConfig;
		m_Jobsystem = jobSystem;
		m_ColorData = new glm::vec3[numPixels];
		m_OuterRefractiveIndex = DialetricIndexTable::GetDialetricIndex(EDialetricType::AIR); //Starting dialetric index for the camera.
#pragma warning (push)
#pragma warning (disable : 4244)
		srand(time(nullptr));
#pragma warning (pop)
	}

	glm::vec3 Renderer::ComputeRayColor(const Ray& ray, Scene& scene, float maxRayLength, float& absorbDistance, int depth)
	{
		//Whitted raytracing algorithm.
		//1. Check intersection.
			//Return (applied emission).
		//2. Compute surface scattering functions. If these dont exist, call recursive self.
		//3. if the hit primitive is an area light, add the emitted light of the area light.
		//4. Add light source contribution of other light sources in the scene.
			//Sample incoming light.
			//Check if the light has any power contribution.
			//Calculate the BSDF of the hit primitive.
			//Check if the resulting color of the BSDF isn't black and if the light isn't occluded.
				//If so continue to the next light.
			//Add the BSDG color * Sampled light * absDot(incoming light dir, normal).
		//5. Check if the depth + 1 < maxdepth
			// Add specularReflection.
			// Add specularTransmit (refraction).
		//6. Return the calculated color.

		HitInfo hitInfo;
		bool intersectResult = scene.Intersect(ray, hitInfo, maxRayLength);

#if _DEBUG
		if (hitInfo.BVHColor != glm::vec3(0.0f))
		{
			return hitInfo.BVHColor;
		}
#endif

		if (!intersectResult)
		{
			return SampleSky(ray);
		}

		glm::vec3 color = glm::vec3(0, 0, 0);
#if defined (NORMAL_COLORING)
		color = glm::normalize(hitInfo.Normal);
#else
		auto material = hitInfo.HitPrimitive->GetMaterial().lock();
		color += ApplyLighting(ray, scene, material, hitInfo.Point, hitInfo.Normal, maxRayLength);
		//Depth check for reflection and refraction.
		if (depth >= m_Config.MaxRenderDepth)
			return color;

		//Dialectics. (dialetrics are refective and refractive by nature, therefor there is a seperate check for only reflective materials after).
		if(material->IsDialetic)
		{
			color += ApplyRefraction(ray, scene, hitInfo, maxRayLength, absorbDistance, depth);
		}
		else if (material->IsReflective)
		{
			Ray reflectionRay;
			reflectionRay.Origin = hitInfo.Point + hitInfo.Normal * 0.1f;
			reflectionRay.Direction = glm::reflect(ray.Direction, hitInfo.Normal);
			color += material->Reflectivity * ComputeRayColor(reflectionRay, scene, maxRayLength, absorbDistance, depth + 1);
		}

		//Ambient lighting.
		color += material->AlbedoCoefficient * scene.GetAmbientLight().Intensity;
#endif
		return color;
	}

	glm::vec3 Renderer::ApplyRefraction(const Ray& ray, Scene& scene, const HitInfo& hitInfo, float maxRayLength, float& absorbDistance, int depth)
	{
		auto material = hitInfo.HitPrimitive->GetMaterial().lock();
		
		//#TODO: Move these into the main method and select them based on the material the raytracer is entering.
		float n1 = m_OuterRefractiveIndex;
		float n2 = material->RefractiveIndex;

		glm::vec3 normal = hitInfo.Normal;
		float cosTheta = glm::dot(ray.Direction, normal);
		glm::vec3 attenuation = glm::vec3(1.0f);

		if(depth > 0)
			absorbDistance += hitInfo.Distance;

		//Check if the ray is leaving or entering the material.
		if (cosTheta < 0.0f)
		{
			cosTheta = -glm::dot(ray.Direction, normal);
		}
		else
		{
			glm::vec3 absorbance = material->AbsorbanceCoefficient * absorbDistance;
			attenuation.x = expf(-absorbance.x);
			attenuation.y = expf(-absorbance.y);
			attenuation.z = expf(-absorbance.z);

			normal = -hitInfo.Normal;
			std::swap(n1, n2);
		}

		//Reflection.
		Ray reflectionRay;
		glm::vec3 reflectionDirection = glm::reflect(ray.Direction, normal);
		reflectionRay.Origin = hitInfo.Point + reflectionDirection * 0.01f;
		reflectionRay.Direction = reflectionDirection;
		glm::vec3 reflectionColor = ComputeRayColor(reflectionRay, scene, maxRayLength, absorbDistance, depth + 1);

		//Refraction.
		glm::vec3 refractionDir = glm::refract(ray.Direction, normal, n1 / n2);
		Ray refractionRay;
		refractionRay.Origin = hitInfo.Point + refractionDir * 0.01f;
		refractionRay.Direction = refractionDir;
		glm::vec3 refractionColor = ComputeRayColor(refractionRay, scene, maxRayLength, absorbDistance, depth + 1);

		//Reflectiveness + attenuation applied.
		float reflectionMultiplier = SchlickApproximation(n1, n2, hitInfo.Normal, ray.Direction, material->Reflectivity);
		return attenuation * (reflectionMultiplier * reflectionColor + (1.0f - reflectionMultiplier) * refractionColor);
	}

	glm::vec3 Renderer::ApplyLighting(const Ray& ray, Scene& scene, std::shared_ptr<Material> material, const glm::vec3& hitPoint, const glm::vec3& normal, float maxRayLength)
	{
		glm::vec3 pixelColor = glm::vec3(0.0f);
		for (int i = 0; i < scene.GetLightCount(); i++)
		{
			Light& pointLight = scene.GetLight(i);
			glm::vec3 lightPosition = pointLight.GetTransform().GetTranslation();

			HitInfo shadowHitInfo;
			Ray shadowRay;
			glm::vec3 shadowRayDirection = glm::normalize(lightPosition - hitPoint);
			shadowRay.Origin = hitPoint + shadowRayDirection * 0.0001f;
			shadowRay.Direction = shadowRayDirection;

			if (m_Config.EnableShadows && scene.Intersect(shadowRay, shadowHitInfo, maxRayLength))
			{
				continue;
			}

			glm::vec3 lightDir = glm::vec3(0.0f);
			float lightingAttentuation = 0.0f;

			switch(scene.GetLight(i).GetType())
			{
			case ELightType::DIRECTIONAL:
			{
				lightDir = scene.GetLight(i).GetTransform().GetMatrix()[3];
				lightingAttentuation = scene.GetLight(i).GetIntensity();
				break;
			}
			case ELightType::POINT:
			{
				lightDir = glm::normalize(lightPosition - hitPoint);
				lightingAttentuation = pointLight.CalculateLightingAttentuation(hitPoint);
				break;
			}
			default:
				printf("ERROR: Unsupported light type is being used in the renderer, this could lead to incorrect render results!\n");
				break;
			}

			glm::vec3 halfwayVector = glm::normalize(-ray.Direction + lightDir);
			glm::vec3 diffuse = material->AlbedoCoefficient * lightingAttentuation * std::max(glm::dot(normal, lightDir), 0.0f);
			glm::vec3 specular = material->SpecularCoefficient * lightingAttentuation * std::powf(std::max(glm::dot(normal, halfwayVector), 0.0f), static_cast<float>(material->PhongExponent));

			pixelColor += diffuse + specular;
		}

		return pixelColor;
	}

	void Renderer::RenderPerspective(Scene& scene)
	{
		Camera camera = scene.GetCameraData(m_ActiveCameraIndex);

		glm::mat4 cameraWorldMatrix = camera.GetTransform().GetMatrix();
		glm::vec3 cameraPosition = cameraWorldMatrix[3];

		const float maxRayLength = camera.GetZFar();
		float aspectRatio = camera.GetAspectRatio();

		int viewPortPixelWidth = camera.GetViewPortPixelWidth();
		int viewPortPixelHeight = camera.GetViewPortPixelHeight();

		const float fieldOfViewInRads = camera.GetFovInRads();
		const float imagePlaneScale = std::tanf(fieldOfViewInRads / 2);
		
		const float lensRadius = camera.GetAperture() / 2;

		int aaSampleRate = static_cast<int>(m_Config.AASampleRate);
		int dofSampleRate = static_cast<int>(m_Config.DoFSampleRate);

		glm::ivec2 rayCluserSize = glm::ivec2(4, 4);

		printf("\nRender starting...\n");
		ProgressReporter reporter(m_Jobsystem, static_cast<uint64_t>(viewPortPixelWidth * viewPortPixelHeight), "RenderReporter");

		for (int y = 0; y < viewPortPixelHeight; y += rayCluserSize.y)
		{
			for (int x = 0; x < viewPortPixelWidth; x += rayCluserSize.x)
			{
				ThreadJob job = [&, rayCluserSize, x, y]()
				{
					//Cluster loops
					for (int i = 0; i < rayCluserSize.x; i++)
					{
						for (int j = 0; j < rayCluserSize.y; j++)
						{
							int elementIndex = x + i + (y + j) * viewPortPixelWidth;
							glm::vec3 pixelColor = glm::vec3(0.0f);

							//Apply MSAA.
							for (int s = 0; s < aaSampleRate; s++)
							{
								for (int t = 0; t < aaSampleRate; t++)
								{
									float r = (static_cast<float>(rand()) / (RAND_MAX));
									const glm::vec2 samplePosition = {
										static_cast<float>(s + r) / static_cast<float>(m_Config.AASampleRate),
										static_cast<float>(t + r) / static_cast<float>(m_Config.AASampleRate)
									};

									const glm::vec2 imagePlanePixelPos = {
										(2 * (static_cast<float>(x + i) + samplePosition.x) / static_cast<float>(viewPortPixelWidth) - 1) * aspectRatio,
										1 - 2 * (static_cast<float>(y + j) + samplePosition.y) / static_cast<float>(viewPortPixelHeight)
									};

									glm::vec3 rayDirection = glm::normalize(cameraWorldMatrix * glm::vec4(
										imagePlanePixelPos.x * imagePlaneScale,
										imagePlanePixelPos.y * imagePlaneScale,
										1.0f,
										0.0f
									));

									//Depth of Field.
									glm::vec3 focalPoint = cameraPosition + rayDirection * camera.GetFocalLength();
									for (int k = 0; k < dofSampleRate * dofSampleRate; k++)
									{
										glm::vec3 apertureShift = glm::vec3(
											(static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 2.0f) - 1.0f) * lensRadius,
											(static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 2.0f) - 1.0f) * lensRadius,
											0.0f
										);

										Ray ray;
										ray.Origin = cameraPosition + apertureShift;
										ray.Direction = glm::normalize(focalPoint - ray.Origin);

										float absorbDistance = 0.0f;
										pixelColor += ComputeRayColor(ray, scene, maxRayLength, absorbDistance);
									}
								}
							}

							//Take the average of the sampled ray and apply gamma correction.
							glm::vec3 srgb = pixelColor / static_cast<float>(aaSampleRate * aaSampleRate * dofSampleRate * dofSampleRate);
							m_ColorData[elementIndex] = glm::vec3(
								std::powf(srgb.r, 1.0f / 2.2f),
								std::powf(srgb.g, 1.0f / 2.2f),
								std::powf(srgb.b, 1.0f / 2.2f)
							);
						}
					}
				};

				m_Jobsystem->Execute(job);
				reporter.Update(rayCluserSize.x * rayCluserSize.y); //Updated when a new thread for a new cluster is launched, since this can't be tracked.
			}
		}

		reporter.Done();
		m_Jobsystem->Wait();
	}

	void Renderer::RenderOblique(Scene& scene, const glm::vec3& rayDirection)
	{
		Camera camera = scene.GetCameraData(m_ActiveCameraIndex);
		glm::mat4 cameraWorldMatrix = camera.GetTransform().GetMatrix();
		
		const float maxRayLength = camera.GetZFar();
		float aspectRatio = camera.GetAspectRatio();

		float cameraSize = camera.GetCameraSize();

		int viewPortPixelWidth = camera.GetViewPortPixelWidth();
		int viewPortPixelHeight = camera.GetViewPortPixelHeight();

		int aaSampleRate = static_cast<int>(m_Config.AASampleRate);
		int dofSampleRate = static_cast<int>(m_Config.DoFSampleRate);

		printf("\nRender starting...\n");
		ProgressReporter reporter(m_Jobsystem, static_cast<uint64_t>(viewPortPixelWidth * viewPortPixelHeight), "RenderReporter");

		glm::ivec2 rayCluserSize = glm::ivec2(4, 4);
		for (int y = 0; y < viewPortPixelHeight; y += rayCluserSize.y)
		{
			for (int x = 0; x < viewPortPixelWidth; x += rayCluserSize.x)
			{
				ThreadJob job = [&, rayCluserSize, x, y]()
				{
					for (int i = 0; i < rayCluserSize.x; i++)
					{
						for (int j = 0; j < rayCluserSize.y; j++)
						{
							//Apply MSAA.
							int elementIndex = x + i + (y + j) * viewPortPixelWidth;
							glm::vec3 pixelColor = glm::vec3(0.0f);
							for (int s = 0; s < aaSampleRate; s++)
							{
								for (int t = 0; t < aaSampleRate; t++)
								{
									float r = (static_cast<float>(rand()) / (RAND_MAX));
									const glm::vec2 samplePosition = {
										static_cast<float>(s + r) / static_cast<float>(m_Config.AASampleRate),
										static_cast<float>(t + r) / static_cast<float>(m_Config.AASampleRate)
									};

									const glm::vec2 imagePlanePixelPos = {
										(2 * (static_cast<float>(x + i) + samplePosition.x) / static_cast<float>(viewPortPixelWidth) - 1) * aspectRatio,
										1 - 2 * (static_cast<float>(y + j) + samplePosition.y) / static_cast<float>(viewPortPixelHeight)
									};

									Ray ray;
									ray.Origin = cameraWorldMatrix * glm::vec4(imagePlanePixelPos.x * cameraSize, imagePlanePixelPos.y * cameraSize, 0.0f, 1.0f);
									ray.Direction = glm::normalize(cameraWorldMatrix * glm::vec4(rayDirection, 0.0f));

									float absorbDistance = 0.0f;
									pixelColor += ComputeRayColor(ray, scene, maxRayLength, absorbDistance);
								}
							}

							//Apply gamma correction.
							glm::vec3 srgb = pixelColor / static_cast<float>(aaSampleRate * aaSampleRate * dofSampleRate * dofSampleRate);
							glm::vec3 rgb = glm::vec3(
								std::powf(srgb.r, 1.0f / 2.2f),
								std::powf(srgb.g, 1.0f / 2.2f),
								std::powf(srgb.b, 1.0f / 2.2f)
							);

							m_ColorData[elementIndex] = rgb;
						}
					}
				};

				m_Jobsystem->Execute(job);
				reporter.Update(rayCluserSize.x * rayCluserSize.y); //Updated when a new thread for a new cluster is launched, since this can't be tracked.
			}
		}

		reporter.Done();
		m_Jobsystem->Wait();
	}

	glm::vec3 Renderer::SampleSky(const Ray& ray)
	{
		if (m_Skysphere.expired())
			return m_Config.BackGroundColor;

		constexpr float pi = glm::pi<float>();
		float u = 1 - (atan2(ray.Direction.z, ray.Direction.x) + pi) / (glm::two_pi<float>());
		float v = (asin(ray.Direction.y) + glm::half_pi<float>()) / pi;

		return m_Skysphere.lock()->LookUp(u, v);
	}

	glm::vec3* Renderer::Render(Scene& scene, glm::vec3 obliqueDirection)
	{
		if(m_ColorData == nullptr)
		{
			printf("ERROR: The renderer has not been initialized yet!");
			return nullptr;
		}

		switch (scene.GetCameraData(m_ActiveCameraIndex).GetCameraType())
		{
		case ECameraType::PERSPECTIVE:
			RenderPerspective(scene);
			break;
		case ECameraType::ORTHOGRAPHIC:
			RenderOblique(scene, glm::vec3(0.0f, 0.0f, 1.0f));
			break;
		case ECameraType::OBLIQUE:
			RenderOblique(scene, obliqueDirection);
			break;
		default:
			printf("ERROR: Camera type is not supported!");
			__debugbreak();
			break;
		}

		return m_ColorData;
	}

	void Renderer::SetActiveCamera(int cameraIndex)
	{
		m_ActiveCameraIndex = cameraIndex;
	}

	int Renderer::GetActiveCameraNodeIndex() const
	{
		return m_ActiveCameraIndex;
	}

	void Renderer::SetSkySphere(std::weak_ptr<Image> skysphere)
	{
		m_Skysphere = skysphere;
	}

	float Renderer::SchlickApproximation(float n1, float n2, const glm::vec3& normal, const glm::vec3& incident, float reflectivity)
	{
		float cosTheta = -glm::dot(normal, incident);
		float r0 = (n1 - n2) / (n1 + n2);
		r0 *= r0;

		if (n1 > n2)
		{
			float n = n1 / n2;
			float sinT2 = n * n * (1.0f - cosTheta * cosTheta);

			//Total Internal Reflection.
			if (sinT2 > 1.0f)
			{
				return 1.0f;
			}
			cosTheta = sqrt(1.0f - sinT2);
		}

		float x = 1.0f - cosTheta;
		float ret = r0 + (1.0f - r0) * (x * x * x * x * x);
		return (reflectivity + (1.0f - reflectivity) * ret);
	}
}