#include <Radiant/Rendering/Rendering.hpp>

namespace Radiant
{
	// =================================================================== //
	// ========================= Rendering API =========================== //

	static RenderingAPIType s_RenderingAPI = RenderingAPIType::Vulkan;

	const RenderingAPIType RenderingAPI::GetAPI()
	{
		return s_RenderingAPI;
	}

	void RenderingAPI::SetAPI(RenderingAPIType api)
	{
		s_RenderingAPI = api;
	}

	// ========================= Rendering API =========================== //
	// =================================================================== //

	static Memory::Shared<RenderingContext> s_RenderingContext = nullptr;

	void Rendering::Create(GLFWwindow* window)
	{
		switch (RenderingAPI::GetAPI())
		{
			case RenderingAPIType::Vulkan:
			{
				s_RenderingContext = RenderingContext::Create(window);
				return;
			}
		}
	}

}
