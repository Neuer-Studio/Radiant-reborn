#pragma once

#include <Radiant/Core/Camera.hpp>
#include <Radiant/Core/Math/AABB.hpp>

namespace Radiant::Math
{
    class Ray
    {
    public:
        Ray( const glm::vec3& origin, const glm::vec3& direction )
        {
            m_Origin    = origin;
            m_Direction = direction;
        }

        bool IntersectsAABB( const AABB& box ) const
        {
            glm::vec3 invDir = 1.0f / m_Direction;

            glm::vec3 t0 = ( box.Min - m_Origin ) * invDir;
            glm::vec3 t1 = ( box.Max - m_Origin ) * invDir;

            glm::vec3 tmin = glm::min( t0, t1 );
            glm::vec3 tmax = glm::max( t0, t1 );

            float tminFinal = std::max( std::max( tmin.x, tmin.y ), tmin.z );
            float tmaxFinal = std::min( std::min( tmax.x, tmax.y ), tmax.z );

            return tminFinal <= tmaxFinal && tmaxFinal > 0;
        }
    public:
        static std::pair<glm::vec3, glm::vec3> CastRay( const Camera& camera, float mouseX, float mouseY );

    private:
        glm::vec3 m_Origin, m_Direction;
    };
} // namespace Radiant::Math