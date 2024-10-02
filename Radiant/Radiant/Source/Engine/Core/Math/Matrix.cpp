#include <Radiant/Core/Math/Matrix.hpp>

namespace Radiant::Math::Matrix
{
    glm::mat4 AssimpAIMat4toGLMMat4( const aiMatrix4x4& matrix )
    {
        glm::mat4 result;
        // the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
        result[0][0] = matrix.a1;
        result[1][0] = matrix.a2;
        result[2][0] = matrix.a3;
        result[3][0] = matrix.a4;
        result[0][1] = matrix.b1;
        result[1][1] = matrix.b2;
        result[2][1] = matrix.b3;
        result[3][1] = matrix.b4;
        result[0][2] = matrix.c1;
        result[1][2] = matrix.c2;
        result[2][2] = matrix.c3;
        result[3][2] = matrix.c4;
        result[0][3] = matrix.d1;
        result[1][3] = matrix.d2;
        result[2][3] = matrix.d3;
        result[3][3] = matrix.d4;
        return result;
    }

    TransformComponents DecomposeTransform( const glm::mat4& transform )
    {
        glm::vec3 skew;
        glm::vec3 scale;
        glm::vec3 translation;
        glm::quat rotation;
        glm::vec4 perspective;

        glm::decompose( transform, scale, rotation, translation, skew, perspective );

        return { translation, rotation, scale };
    }
} // namespace Radiant::Math::Matrix