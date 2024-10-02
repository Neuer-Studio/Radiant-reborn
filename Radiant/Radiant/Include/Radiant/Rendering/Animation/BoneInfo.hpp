#pragma once

#include <glm/glm.hpp>

namespace Radiant::Animation
{
    struct BoneInfo
    {
        int       ID;
        glm::mat4 BoneOffset;
    };
} // namespace Radiant::Animation