#pragma once

#include "Engine/Camera/FlyCameraController.h"
#include "Engine/Camera/PerspectiveCamera.h"

namespace Editor {

class CameraPanel final {
public:
    void onImGuiRender(
        Engine::PerspectiveCamera& camera,
        Engine::FlyCameraController& controller);
};

} // namespace Editor
