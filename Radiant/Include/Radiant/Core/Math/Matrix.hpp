#pragma once

#include <glm/glm.hpp>
#include <assimp/matrix4x4.h>
#include <glm/gtx/matrix_decompose.hpp>

namespace Radiant::Math::Matrix
{
    struct TransformComponents
    {
        glm::vec3 Translation;
        glm::quat Rotation;
        glm::vec3 Scale;
    };

    glm::mat4           AssimpAIMat4toGLMMat4( const aiMatrix4x4& matrix );
    TransformComponents DecomposeTransform( const glm::mat4& transform );
} // namespace Radiant::Math::Matrix
