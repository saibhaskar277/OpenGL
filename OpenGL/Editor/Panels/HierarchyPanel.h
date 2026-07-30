#pragma once

#include "Engine/Scene/Scene.h"

namespace Editor {

class HierarchyPanel final {
public:
    // Returns true if the scene was mutated (add/delete/drop).
    bool onImGuiRender(Engine::Scene& scene);
};

} // namespace Editor
