#pragma once

#include <Radiant/Rendering/RenderingAPI.hpp>
#include <Radiant/Rendering/RenderingContext.hpp>

namespace Radiant
{
	class Rendering : public Memory::RefCounted
	{
	public:
		~Rendering() = default;

		static void Create(GLFWwindow* window);
	private:
	};
}