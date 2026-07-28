#pragma once

#include "Engine/Assets/AssetManager.h"

#include <string>

namespace Editor {

class ContentBrowserPanel final {
public:
    void onImGuiRender(Engine::AssetManager& assets);

    [[nodiscard]] const std::string& selectedTextureId() const;

private:
    std::string m_selectedTextureId;
};

} // namespace Editor
