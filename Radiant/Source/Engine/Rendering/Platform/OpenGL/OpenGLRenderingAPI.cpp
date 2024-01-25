#include <glad/glad.h>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLRenderingAPI.hpp>
#include <Radiant/Rendering/Rendering.hpp>

namespace Radiant
{
	void OpenGLRenderingAPI::Clear(float rgba[4]) const
	{
			float r = (*rgba);
			rgba++;
			float g = (*rgba);
			rgba++;
			float b = (*rgba);
			rgba++;
			float a = (*rgba);
			glClearColor(r, g, b, a);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRenderingAPI::DrawPrimitive(Primitives primitive, uint32_t count) const
	{
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

}