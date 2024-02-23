#pragma once

#include <Radiant/Rendering/RenderingAPI.hpp>
#include <Radiant/Rendering/Pipeline.hpp>
#include <Radiant/Rendering/Material.hpp>
#include <Radiant/Rendering/Mesh.hpp>
#include <Radiant/Rendering/RenderingContext.hpp>
#include <Radiant/Core/Memory/CommandBuffer.hpp>

namespace Radiant
{
	class Rendering : public Memory::RefCounted
	{
	public:
		virtual ~Rendering();

		static void Clear(float rgba[4]);
		static void SubmitMesh(const Memory::Shared<Mesh>& mesh, const Memory::Shared<Pipeline>& pipeline);
		static void DrawPrimitive(Primitives primitive = Primitives::Triangle, uint32_t count = 3, bool depthTest = true);

		static void BeginRenderPass(Memory::Shared <RenderPass>& renderPass, bool clear = true);
		static void EndRenderPass();

	public:
		static Memory::Shared<RenderingContext> Initialize(GLFWwindow * window);
		static Memory::Shared<RenderingContext> GetRenderingContext();

		static const ShaderLibrary* GetShaderLibrary();

	public:
		static void SubmitFullscreenQuad(const Memory::Shared<Pipeline>& pipeline, const Memory::Shared<Material>& material);
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