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
		static float _rgba[4] = { rgba[0], rgba[1], rgba[2], rgba[3] };
		Rendering::Submit([]()
			{
				s_RenderingAPIPlatform->Clear((float*)_rgba);
			});
	}

	void Rendering::DrawPrimitive(Primitives primitive)
	{
		Rendering::Submit([primitive]()
			{
				s_RenderingAPIPlatform->DrawPrimitive(primitive);
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
