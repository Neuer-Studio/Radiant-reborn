#pragma once

#include <Radiant/Rendering/RenderingAPI.hpp>
#include <Radiant/Rendering/RenderingContext.hpp>
#include <Radiant/Core/Memory/CommandBuffer.hpp>

namespace Radiant
{
	class Rendering : public Memory::RefCounted
	{
	public:
		virtual ~Rendering() = default;

		static void Clear(float rgba[4]);
		static void DrawPrimitive(Primitives primitive = Primitives::Triangle, uint32_t count = 3);
	public:
		static Memory::Shared<RenderingContext> Initialize(GLFWwindow * window);
		static Memory::Shared<RenderingContext> GetRenderingContext();
	public:
		template <typename FuncT>
		static void SubmitCommand(FuncT&& func)
		{
			auto renderCMD = [](void* ptr)
			{
				auto pFunc = (FuncT*)ptr;

				(*pFunc)();

				pFunc->~FuncT();
			};

			auto storageBuffer = GetRenderingCommandBuffer().AddCommand(renderCMD, sizeof(func));
			new (storageBuffer) FuncT(std::forward<FuncT>(func));
		}

		static Memory::CommandBuffer& GetRenderingCommandBuffer();
	private:
	};
}