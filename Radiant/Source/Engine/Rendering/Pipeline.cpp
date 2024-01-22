#include <Radiant/Rendering/Pipeline.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLPipeline.hpp>

namespace Radiant
{
	Memory::Shared<Pipeline> Pipeline::Create(const PipelineSpecification& spec)
	{
		switch (RenderingAPI::GetAPI())
		{
			case RenderingAPIType::None:    return nullptr;
			case RenderingAPIType::OpenGL:  return Memory::Shared<OpenGLPipeline>::Create(spec);
		}
		RADIANT_VERIFY(false, "Unknown Rendering API");
		return nullptr;
	}
}