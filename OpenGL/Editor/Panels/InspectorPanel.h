#pragma once

#include "Engine/Assets/AssetManager.h"
#include "Engine/Scene/Scene.h"

#include <string>

namespace Editor {

class InspectorPanel final {
public:
    void onImGuiRender(Engine::Scene& scene, Engine::AssetManager& assets);

private:
    void drawTextureSlot(
        const char* label,
        std::string& path,
        Engine::AssetManager& assets);
};

} // namespace Editor
