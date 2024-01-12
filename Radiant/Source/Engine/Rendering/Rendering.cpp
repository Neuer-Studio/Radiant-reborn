#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLRenderingAPI.hpp>

namespace Radiant
{
	// =================================================================== //
	// ========================= Rendering API =========================== //

	static RenderingAPIType s_RenderingAPI = RenderingAPIType::None;

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
	static Memory::Shared<RenderingAPI> s_RenderingAPIPlatform = nullptr;
	static Memory::CommandBuffer s_CommandBuffer;

	void Rendering::Clear(float rgba[4])
	{
		Rendering::Submit([rgba]()
			{
				s_RenderingAPIPlatform->Clear(rgba);
			});
	}

	Memory::Shared<RenderingContext> Rendering::Initialize(GLFWwindow* window)
	{
		switch (RenderingAPI::GetAPI())
		{
			case RenderingAPIType::OpenGL:
			{
				s_RenderingAPIPlatform = Memory::Shared<OpenGLRenderingAPI>::Create();
			}
		}
		RADIANT_VERIFY(s_RenderingAPIPlatform);
		s_RenderingContext = RenderingContext::Create(window);
		return s_RenderingContext;
	}

	Radiant::Memory::Shared<Radiant::RenderingContext> Rendering::GetRenderingContext()
	{
		return s_RenderingContext;
	}

	Memory::CommandBuffer& Rendering::GetRenderingCommandBuffer()
	{
		return s_CommandBuffer;
	}
}
