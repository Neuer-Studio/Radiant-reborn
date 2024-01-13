#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLShader.hpp>
#include <Radiant/Rendering/Shader.hpp>

namespace Radiant
{

	Radiant::Memory::Shared<Radiant::Shader> Shader::Create(const std::filesystem::path& path)
	{
		switch (RenderingAPI::GetAPI())
		{
			case RenderingAPIType::OpenGL:
			{
				return Memory::Shared<OpenGLShader>::Create(path);
			}
		}

		RADIANT_VERIFY(false);
		return nullptr;
	}

}