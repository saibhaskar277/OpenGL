#pragma once

#include "Engine/Renderer/Mesh/Mesh.h"

namespace Engine {

class PrimitiveFactory final {
public:
    PrimitiveFactory() = delete;

    [[nodiscard]] static Mesh createCube();
};

} // namespace Engine
