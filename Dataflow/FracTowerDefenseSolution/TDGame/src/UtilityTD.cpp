#include "TDpch.h"
#include "UtilityTD.h"

bool IsInRange(const glm::vec3& curPos, const glm::vec3& targetPos, const float& margin)
{
	glm::vec3 target = glm::abs(targetPos);
	glm::vec3 current = glm::abs(curPos);

	float resultX = glm::distance(current.x, target.x);
	float resultY = glm::distance(current.z, target.z);

	if ((resultX <= margin) && (resultY <= margin))
	{
		return true;
	}

	return false;
}

bool IsInRadius(const glm::vec3& curPos, const glm::vec3& targetPos, const float& radius)
{
	float curDistance = glm::length(curPos - targetPos);

	if (curDistance <= radius)
		return true;

	return false;
}
