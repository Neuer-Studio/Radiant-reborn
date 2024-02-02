#include <Radiant/Rendering/Scene/Scene.hpp>
#include <Radiant/Rendering/Rendering.hpp>

#include <Radiant/Rendering/Platform/OpenGL/Scene/OpenGLSceneRendering.hpp>

namespace Radiant
{

	Memory::Shared<SceneRendering> SceneRendering::Create(const std::string& sceneName)
	{
		switch (RenderingAPI::GetAPI())
		{
			case RenderingAPIType::OpenGL:
			{
				return Memory::Shared<OpenGLSceneRendering>::Create(sceneName);
			}
		}
	}

}