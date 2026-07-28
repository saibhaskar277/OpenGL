#pragma once

#include <cstdint>

namespace Engine {

using Entity = std::uint32_t;
constexpr Entity NullEntity = static_cast<Entity>(-1);

} // namespace Engine
