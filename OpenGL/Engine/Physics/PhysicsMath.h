#pragma once

#include "Engine/Physics/PhysicsTypes.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Engine {
namespace PhysicsMath {

[[nodiscard]] glm::quat rotationDegreesToQuat(const glm::vec3& rotationDegrees);
[[nodiscard]] glm::vec3 quatToRotationDegrees(const glm::quat& rotation);

} // namespace PhysicsMath
} // namespace Engine
