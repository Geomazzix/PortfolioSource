#include "Primitives/Triangle.h"
#include "Utility/MathUtility.h"

namespace CRT
{
	Triangle::Triangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& normal) :
		Primitive()
	{
		m_TrianglePoints[0] = v0;
		m_TrianglePoints[1] = v1;
		m_TrianglePoints[2] = v2;
		m_Normal = glm::normalize(normal);

		glm::vec3 minBounds = glm::vec3(INFINITY);
		glm::vec3 maxBounds = glm::vec3(-INFINITY);
		minBounds = CRT::min(minBounds, v0);
		minBounds = CRT::min(minBounds, v1);
		minBounds = CRT::min(minBounds, v2);

		maxBounds = CRT::max(maxBounds, v0);
		maxBounds = CRT::max(maxBounds, v1);
		maxBounds = CRT::max(maxBounds, v2);

		m_AABB = AABB(minBounds, maxBounds);
	}

	bool Triangle::Intersect(const Ray& ray, HitInfo& hitInfo, const float maxRayLength)
	{
		glm::vec3 vertex0 = m_TrianglePoints[0];
		glm::vec3 vertex1 = m_TrianglePoints[1];
		glm::vec3 vertex2 = m_TrianglePoints[2];

		glm::vec3 edge1 = vertex1 - vertex0;
		glm::vec3 edge2 = vertex2 - vertex0;
		
		glm::vec3 h = glm::cross(ray.Direction, edge2);
		float a = glm::dot(edge1, h);
		if (a > -0.000001 && a < 0.000001)
		{
			// This ray is parallel to this triangle.
			return false;   
		}

		float f = 1.0 / a;
		glm::vec3 s = ray.Origin - vertex0;
		float u = f * glm::dot(s, h);
		if (u < 0.0 || u > 1.0)
		{
			return false;
		}
		
		glm::vec3 q = glm::cross(s, edge1);
		float v = f * glm::dot(ray.Direction, q);
		if (v < 0.0 || (u + v) > 1.0)
		{
			return false;
		}

		// At this stage we can compute t to find out where the intersection point is on the line.
		float t = f * glm::dot(edge2, q);
		if (t > 0.000001 && t < maxRayLength)
		{
			hitInfo.Distance = t;
			hitInfo.HitPrimitive = this;
			hitInfo.Normal = m_Normal;
			return true;
		}
		else
		{
			// This means that there is a line intersection but not a ray intersection.
			return false;
		}
	}
}