#pragma once
#include "Glm/glm.hpp"

/// <summary>
/// Checks if a position is in range of another position.
/// </summary>
/// <param name="curPos"><Our Current Position>
/// <param name="targetPos"><The Target Position>
/// <param name="margin"><The range/margin of how close we can be to the target position >
/// <returns><Returns true if our curPos, is in 'margin/range' of the targetPos>
bool IsInRange(const glm::vec3& curPos, const glm::vec3& targetPos, const float& margin);
bool IsInRadius(const glm::vec3& curPos, const glm::vec3& targetPos, const float& radius);
