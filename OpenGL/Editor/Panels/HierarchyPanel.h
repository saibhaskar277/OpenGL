#pragma once

#include "Engine/Scene/Scene.h"

namespace Editor {

class HierarchyPanel final {
public:
    void onImGuiRender(Engine::Scene& scene);
};

} // namespace Editor
