#include <Radiant/Rendering/Platform/OpenGL/OpenGLRenderPass.hpp>

namespace Radiant
{
	OpenGLRenderPass::OpenGLRenderPass(const RenderPassSpecification& spec)
		: m_Specification(spec)
	{
	}

	OpenGLRenderPass::~OpenGLRenderPass()
	{
	}
}