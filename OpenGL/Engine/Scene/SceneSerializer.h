#pragma once

#include "Engine/Scene/Scene.h"

#include <filesystem>

namespace Engine {

class SceneSerializer final {
public:
    explicit SceneSerializer(Scene& scene);

    void save(const std::filesystem::path& path) const;
    void load(const std::filesystem::path& path);

private:
    Scene& m_scene;
};

} // namespace Engine
