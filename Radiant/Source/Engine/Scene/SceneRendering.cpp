#include <Radiant/Scene/Scene.hpp>
#include <Radiant/Rendering/Rendering.hpp>

#include <Radiant/Rendering/Platform/OpenGL/Scene/OpenGLSceneRendering.hpp>

namespace Radiant
{

	SceneRendering* SceneRendering::Create(const Memory::Shared<Scene>& scene)
	{
		switch (RenderingAPI::GetAPI())
		{
			case RenderingAPIType::OpenGL:
			{
				return new OpenGLSceneRendering(scene);
			}
		}
	}

}