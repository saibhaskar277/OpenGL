#pragma once

#include "Engine/Assets/AssetManager.h"
#include "Engine/Scene/Scene.h"

#include <string>

namespace Editor {

class InspectorPanel final {
public:
    // Returns true if any inspected property changed.
    bool onImGuiRender(Engine::Scene& scene, Engine::AssetManager& assets);

private:
    bool drawTextureSlot(
        const char* label,
        std::string& path,
        Engine::AssetManager& assets);
};

} // namespace Editor
