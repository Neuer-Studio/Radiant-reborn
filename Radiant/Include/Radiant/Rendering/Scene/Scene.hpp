#pragma once

#include <Radiant/Rendering/Scene/SceneRendering.hpp>

namespace Radiant
{
	class Scene : public Memory::RefCounted
	{
	public:

		static Environment CreateScene(const Memory::Shared<SceneRendering> sr);
	};
}