#pragma once

#include <Radiant/Core/Camera.hpp>
#include <Common/Core/Math/AABB.hpp>

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

        bool IntersectsAABB( const Common::Math::AABB& aabb ) const
        {
            glm::vec3 dirfrac;
            float     t;
            // r.dir is unit direction vector of ray
            dirfrac.x = 1.0f / m_Direction.x;
            dirfrac.y = 1.0f / m_Direction.y;
            dirfrac.z = 1.0f / m_Direction.z;
            // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
            // r.org is origin of ray
            const glm::vec3& lb = aabb.Min;
            const glm::vec3& rt = aabb.Max;

            float t1 = ( lb.x - m_Origin.x ) * dirfrac.x;
            float t2 = ( rt.x - m_Origin.x ) * dirfrac.x;
            float t3 = ( lb.y - m_Origin.y ) * dirfrac.y;
            float t4 = ( rt.y - m_Origin.y ) * dirfrac.y;
            float t5 = ( lb.z - m_Origin.z ) * dirfrac.z;
            float t6 = ( rt.z - m_Origin.z ) * dirfrac.z;

            float tmin = glm::max( glm::max( glm::min( t1, t2 ), glm::min( t3, t4 ) ), glm::min( t5, t6 ) );
            float tmax = glm::min( glm::min( glm::max( t1, t2 ), glm::max( t3, t4 ) ), glm::max( t5, t6 ) );

            // if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
            if ( tmax < 0 )
            {
                t = tmax;
                return false;
            }

            // if tmin > tmax, ray doesn't intersect AABB
            if ( tmin > tmax )
            {
                t = tmax;
                return false;
            }

            t = tmin;
            return true;
        }

    public:
        static std::pair<glm::vec3, glm::vec3> CastRay( const Camera& camera, float mouseX, float mouseY );

    private:
        glm::vec3 m_Origin, m_Direction;
    };
} // namespace Radiant::Math