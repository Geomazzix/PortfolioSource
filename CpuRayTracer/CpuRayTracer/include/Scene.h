#pragma once
#include <vector>
#include <memory>
#include "Transform.h"
#include "Light.h"
#include "Camera.h"
#include "Primitives/Primitive.h"
#include "SpacialSubdivision/BVH.h"

namespace CRT
{
	/// <summary>
	/// The scene holds all the data a renderer needs to take into account when rendering.
	/// </summary>
	class Scene
	{
	public:
		Scene();
		~Scene() = default;

		int AddLight(std::shared_ptr<Light> light);

		int AddCamera(std::shared_ptr<Camera> camera);
		int AddPrimitive(std::shared_ptr<Primitive> primitive);

		Light& GetLight(int dataIndex);
		size_t GetLightCount();

		Camera& GetCameraData(int dataIndex);
		size_t GetCameraDataSize();

		Primitive& GetPrimitive(int dataIndex);
		size_t GetPrimitiveCount();

		AmbientLight& GetAmbientLight();

		void GenerateBVH();
		bool Intersect(const Ray& ray, HitInfo& hitInfo, float maxRayLength);
		void ShutDown();

#if defined(_DEBUG)
		void SetBVHDebugLayer(int index);
#endif

	private:
		std::vector<std::shared_ptr<Primitive>> m_Primitives;
		std::shared_ptr<AmbientLight> m_AmbientLight;
		std::vector<std::shared_ptr<Light>> m_DirectionalLights;
		std::vector<std::shared_ptr<Camera>> m_Cameras;
		std::unique_ptr<BVH> m_SceneBVH;
	};
}