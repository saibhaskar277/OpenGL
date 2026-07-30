#include "Engine/Physics/PhysicsMath.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Engine {
namespace PhysicsMath {

glm::quat rotationDegreesToQuat(const glm::vec3& rotationDegrees) {
    return glm::quat(glm::radians(rotationDegrees));
}

glm::vec3 quatToRotationDegrees(const glm::quat& rotation) {
    return glm::degrees(glm::eulerAngles(rotation));
}

} // namespace PhysicsMath
} // namespace Engine
