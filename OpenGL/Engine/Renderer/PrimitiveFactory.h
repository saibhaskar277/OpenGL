#pragma once

#include "Engine/Renderer/Mesh.h"

namespace Engine {

class PrimitiveFactory final {
public:
    PrimitiveFactory() = delete;

    [[nodiscard]] static Mesh createCube();
};

} // namespace Engine
