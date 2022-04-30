#include "SpacialSubdivision/AABB.h"

namespace CRT
{
	AABB::AABB(const glm::vec3& min /*= glm::vec3(-1.0f)*/, const glm::vec3& max /*= glm::vec3(1.0f)*/)
	{
		m_Bounds[0] = min;
		m_Bounds[1] = max;
	}

	bool AABB::Intersect(const Ray& ray, const glm::vec3& inverseDirection, const int directionIsNegative[3], const float maxRayLength) const
	{
		float tMin = (m_Bounds[directionIsNegative[0]].x - ray.Origin.x) * inverseDirection.x;
		float tMax = (m_Bounds[1 - directionIsNegative[0]].x - ray.Origin.x) * inverseDirection.x;
		float tyMin = (m_Bounds[directionIsNegative[1]].y - ray.Origin.y) * inverseDirection.y;
		float tyMax = (m_Bounds[1 - directionIsNegative[1]].y - ray.Origin.y) * inverseDirection.y;

		// Update _tMax_ and _tyMax_ to ensure robust bounds intersection
		if (tMin > tyMax || tyMin > tMax)
		{
			return false;
		}

		if (tyMin > tMin)
		{
			tMin = tyMin;
		}

		if (tyMax < tMax)
		{
			tMax = tyMax;
		}

		// Check for ray intersection against $z$ slab
		float tzMin = (m_Bounds[directionIsNegative[2]].z - ray.Origin.z) * inverseDirection.z;
		float tzMax = (m_Bounds[1 - directionIsNegative[2]].z - ray.Origin.z) * inverseDirection.z;

		// Update _tzMax_ to ensure robust bounds intersection
		if (tMin > tzMax || tzMin > tMax)
		{
			return false;
		}

		if (tzMin > tMin)
		{
			tMin = tzMin;
		}

		if (tzMax < tMax)
		{
			tMax = tzMax;
		}

		return (tMin < maxRayLength) && (tMax > 0);
	}

#if _DEBUG
	bool AABB::Intersect(const Ray& ray, float& debugTmax, float& debugTmin, glm::vec3& bvhColor, const glm::vec3& inverseDirection, const int directionIsNegative[3], const float maxRayLength) const
	{
		float tMin =  (m_Bounds[directionIsNegative[0]].x - ray.Origin.x) * inverseDirection.x;
		float tMax =  (m_Bounds[1 - directionIsNegative[0]].x - ray.Origin.x) * inverseDirection.x;
		float tyMin = (m_Bounds[directionIsNegative[1]].y - ray.Origin.y) * inverseDirection.y;
		float tyMax = (m_Bounds[1 - directionIsNegative[1]].y - ray.Origin.y) * inverseDirection.y;

		// Update _tMax_ and _tyMax_ to ensure robust bounds intersection
		if (tMin > tyMax || tyMin > tMax)
		{
			return false;
		}
		
		if (tyMin > tMin)
		{
			tMin = tyMin;
		}
		
		if (tyMax < tMax)
		{
			tMax = tyMax;
		}

		// Check for ray intersection against $z$ slab
		float tzMin = (m_Bounds[directionIsNegative[2]].z - ray.Origin.z) * inverseDirection.z;
		float tzMax = (m_Bounds[1 - directionIsNegative[2]].z - ray.Origin.z) * inverseDirection.z;

		// Update _tzMax_ to ensure robust bounds intersection
		if (tMin > tzMax || tzMin > tMax)
		{
			return false;
		}

		if (tzMin > tMin)
		{
			tMin = tzMin;
		}
		
		if (tzMax < tMax)
		{
			tMax = tzMax;
		}
		
		if ((tMin > maxRayLength) || (tMax <= 0))
		{
			return false;
		}

		//Checks for tmin.
		const float lineThickness = 0.075f;
		glm::vec3 hitPoints[2] = 
		{
			ray.Origin + ray.Direction * tMin,
			ray.Origin + ray.Direction * tMax
		};

		for(int pointIndex = 0; pointIndex < 2; pointIndex++)
		{
			for (int i = 0; i < 2; i++)
			{
				if (abs(hitPoints[pointIndex].x - m_Bounds[i].x) < lineThickness)
				{
					if (abs(hitPoints[pointIndex].y - m_Bounds[i].y) < lineThickness)
					{
						bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
					}

					if (abs(hitPoints[pointIndex].z - m_Bounds[i].z) < lineThickness)
					{
						bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
					}
				}

				if (abs(hitPoints[pointIndex].y - m_Bounds[i].y) < lineThickness)
				{
					if (abs(hitPoints[pointIndex].x - m_Bounds[i].x) < lineThickness)
					{
						bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
					}

					if (abs(hitPoints[pointIndex].z - m_Bounds[i].z) < lineThickness)
					{
						bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
					}
				}

				if (abs(hitPoints[pointIndex].z - m_Bounds[i].z) < lineThickness)
				{
					if (abs(hitPoints[pointIndex].x - m_Bounds[i].x) < lineThickness)
					{
						bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
					}

					if (abs(hitPoints[pointIndex].y - m_Bounds[i].y) < lineThickness)
					{
						bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
					}
				}
			}

			if (abs(hitPoints[pointIndex].y - m_Bounds[1].y) < lineThickness)
			{
				if (abs(hitPoints[pointIndex].z - m_Bounds[0].z) < lineThickness)
				{
					bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
				}

				if (abs(hitPoints[pointIndex].x - m_Bounds[0].x) < lineThickness)
				{
					bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
				}
			}

			if (abs(hitPoints[pointIndex].y - m_Bounds[0].y) < lineThickness)
			{
				if (abs(hitPoints[pointIndex].z - m_Bounds[1].z) < lineThickness)
				{
					bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
				}

				if (abs(hitPoints[pointIndex].x - m_Bounds[1].x) < lineThickness)
				{
					bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
				}
			}

			if (abs(hitPoints[pointIndex].z - m_Bounds[1].z) < lineThickness)
			{
				if (abs(hitPoints[pointIndex].y - m_Bounds[0].y) < lineThickness)
				{
					bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
				}

				if (abs(hitPoints[pointIndex].x - m_Bounds[0].x) < lineThickness)
				{
					bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
				}
			}

			if (abs(hitPoints[pointIndex].z - m_Bounds[0].z) < lineThickness)
			{
				if (abs(hitPoints[pointIndex].y - m_Bounds[1].y) < lineThickness)
				{
					bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
				}

				if (abs(hitPoints[pointIndex].x - m_Bounds[1].x) < lineThickness)
				{
					bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
				}
			}
		}

		debugTmin = tMin;
		debugTmax = tMax;
		return true;
	}
#endif

	const glm::vec3& AABB::GetMin() const
	{
		return m_Bounds[0];
	}

	const glm::vec3& AABB::GetMax() const
	{
		return m_Bounds[1];
	}

	glm::vec3 AABB::GetCenter() const
	{
		return (m_Bounds[1] + m_Bounds[0]) * 0.5f;
	}

	glm::vec3 AABB::GetOffset(const glm::vec3& point) const
	{
		glm::vec3 o = point - m_Bounds[0];
		
		if(m_Bounds[1].x > m_Bounds[0].x)
		{
			o.x /= m_Bounds[1].x - m_Bounds[0].x;
		}

		if (m_Bounds[1].y > m_Bounds[0].y)
		{
			o.y /= m_Bounds[1].y - m_Bounds[0].y;
		}

		if (m_Bounds[1].z > m_Bounds[0].z)
		{
			o.z /= m_Bounds[1].z - m_Bounds[0].z;
		}

		return o;
	}

	float AABB::GetSurfaceArea() const
	{
		glm::vec3 diagonal = m_Bounds[1] - m_Bounds[0];
		return 2.0f * (diagonal.x * diagonal.y + diagonal.x * diagonal.z + diagonal.y * diagonal.z); //All faces's surfaces summed up together.
	}

	CRT::AABB AABB::Combine(const AABB& a, const AABB& b)
	{
		const glm::vec3 min = glm::vec3(
			glm::min<float>(a.GetMin().x, b.GetMin().x),
			glm::min<float>(a.GetMin().y, b.GetMin().y),
			glm::min<float>(a.GetMin().z, b.GetMin().z)
		);
		const glm::vec3 max = glm::vec3(
			glm::max<float>(a.GetMax().x, b.GetMax().x),
			glm::max<float>(a.GetMax().y, b.GetMax().y),
			glm::max<float>(a.GetMax().z, b.GetMax().z)
		);

		return AABB(min, max);
	}

	CRT::AABB AABB::Combine(const AABB& a, const glm::vec3& b)
	{
		const glm::vec3 min = glm::vec3(
			glm::min<float>(a.GetMin().x, b.x),
			glm::min<float>(a.GetMin().y, b.y),
			glm::min<float>(a.GetMin().z, b.z)
		);
		const glm::vec3 max = glm::vec3(
			glm::max<float>(a.GetMax().x, b.x),
			glm::max<float>(a.GetMax().y, b.y),
			glm::max<float>(a.GetMax().z, b.z)
		);

		return AABB(min, max);
	}
}