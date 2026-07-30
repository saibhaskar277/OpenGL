#pragma once

#include "Engine/Scene/Scene.h"

namespace Editor {

class PerformancePanel final {
public:
    void beginFrame();
    void onImGuiRender(const Engine::Scene& scene, float deltaTime);

private:
    float m_smoothedFps = 0.0f;
    float m_smoothedFrameMs = 0.0f;
    float m_minFrameMs = 1000.0f;
    float m_maxFrameMs = 0.0f;
    float m_resetTimer = 0.0f;
};

} // namespace Editor
