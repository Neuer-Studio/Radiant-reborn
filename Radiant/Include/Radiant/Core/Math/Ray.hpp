#pragma once

#include <Radiant/Core/Camera.hpp>

namespace Radiant::Math 
{
	std::pair<glm::vec3, glm::vec3> CastRay(const Camera& camera, float mouseX, float mouseY);
}