#include <Radiant/Core/Math/Ray.hpp>

namespace Radiant::Math
{

    std::pair<glm::vec3, glm::vec3> Ray::CastRay(const Camera& camera, float mouseX, float mouseY)
    {
        glm::vec4 ray_clip = glm::vec4(mouseX, mouseY, -1.0, 1.0);
        glm::vec4 ray_eye = glm::inverse(camera.GetProjectionMatrix()) * ray_clip;
        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

        glm::vec3 ray_wor = (glm::inverse( camera.GetViewMatrix() ) * ray_eye );
        // don't forget to normalise the vector at some point
        ray_wor = glm::normalize( ray_wor );

        return {camera.GetPosition(), ray_wor};
    }

} // namespace Radiant::Math