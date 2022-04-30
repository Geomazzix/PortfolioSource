#include "Scene.h"

namespace CRT
{
	Scene::Scene()
	{
		m_AmbientLight = std::make_shared<AmbientLight>();
	}

	int Scene::AddLight(std::shared_ptr<Light> light)
	{
		m_DirectionalLights.push_back(light);
		return static_cast<int>(m_DirectionalLights.size() - 1);
	}

	int Scene::AddCamera(std::shared_ptr<Camera> camera)
	{
		m_Cameras.push_back(camera);
		return static_cast<int>(m_Cameras.size() - 1);
	}

	int Scene::AddPrimitive(std::shared_ptr<Primitive> primitive)
	{
		m_Primitives.push_back(primitive);
		return static_cast<int>(m_Primitives.size() - 1);
	}

	Light& Scene::GetLight(int dataIndex)
	{
		return *m_DirectionalLights[dataIndex].get();
	}

	size_t Scene::GetLightCount()
	{
		return m_DirectionalLights.size();
	}

	Camera& Scene::GetCameraData(int dataIndex)
	{
		return *m_Cameras[dataIndex].get();
	}

	size_t Scene::GetCameraDataSize()
	{
		return m_Cameras.size();
	}

	Primitive& Scene::GetPrimitive(int dataIndex)
	{
		return *m_Primitives[dataIndex].get();
	}

	size_t Scene::GetPrimitiveCount()
	{
		return m_Primitives.size();
	}

	AmbientLight& Scene::GetAmbientLight()
	{
		return *m_AmbientLight;
	}

	void Scene::GenerateBVH()
	{
		m_SceneBVH = std::make_unique<BVH>();
		BVHConfig bvhConfig;
		bvhConfig.PartitionType = EPartitionType::SAH;
		m_SceneBVH->Initialize(bvhConfig, m_Primitives);
	}

	bool Scene::Intersect(const Ray& ray, HitInfo& lastHitInfo, float maxRayLength)
	{
		//Check if the BVH is available, if not, use brute force linear search.
		if(m_SceneBVH != nullptr)
		{
			return m_SceneBVH->Intersect(ray, lastHitInfo, maxRayLength);
		}
		else
		{
			lastHitInfo.Distance = maxRayLength;
			HitInfo hitInfo;

			for (int i = 0; i < m_Primitives.size(); i++)
			{
				Primitive& primitive = *m_Primitives[i];
				if (primitive.Intersect(ray, hitInfo, maxRayLength) && hitInfo.Distance < lastHitInfo.Distance)
				{
					lastHitInfo = hitInfo;
				}
			}

			return lastHitInfo.Distance < maxRayLength;
		}
	}

	void Scene::ShutDown()
	{
		m_SceneBVH->Shutdown();
	}

#if defined(_DEBUG)
	void Scene::SetBVHDebugLayer(int index)
	{
		m_SceneBVH->SetDebugLayer(index);
	}
#endif
}