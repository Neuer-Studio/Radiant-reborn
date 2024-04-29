#pragma once

#include <Radiant/Rendering/RendererAPI.hpp>
#include <Radiant/Rendering/Pipeline.hpp>
#include <Radiant/Rendering/Material.hpp>
#include <Radiant/Rendering/Mesh.hpp>
#include <Radiant/Rendering/RenderingContext.hpp>
#include <Radiant/Core/Memory/CommandBuffer.hpp>
#include <Radiant/Core/Math/AABB.hpp>

namespace Radiant
{
	struct DrawDeclarationCommand
	{
		glm::mat4 Transform;
		Memory::Shared<Mesh> Mesh;
	};

	struct DrawSpecificationCommandWithMaterial
	{
		explicit DrawSpecificationCommandWithMaterial() = default;

		DrawDeclarationCommand Declration;
		Memory::Shared<Material> Material;
	};

	class Rendering : public Memory::RefCounted
	{
	public:
		virtual ~Rendering();

		static void Clear(float rgba[4]);
		static void SubmitMesh(const DrawDeclarationCommand& specification, const Memory::Shared<Pipeline>& pipeline, const Memory::Shared<Material>& Material);
		static void SubmitMeshWithMaterial(const DrawSpecificationCommandWithMaterial& specification, const Memory::Shared<Pipeline>& pipeline);
		static void DrawPrimitive(Primitives primitive = Primitives::Triangle, uint32_t count = 3, bool depthTest = true);
		static void SetLineWidth(float width = 1.0f);
		static void DrawLine(const glm::vec3& p1, const glm::vec3& p2, float lineWidth = 1.0f);
		static void DrawAABB(const Math::AABB& aabb, const glm::mat4& transform);
		static void DrawAABB(const Memory::Shared<Mesh>& mesh, const glm::mat4& transform);

		static void BeginRenderPass(Memory::Shared <RenderPass>& renderPass, bool clear = true);
		static void EndRenderPass();
		
		[[nodiscard]] static Environment CreateEnvironmentMap(const std::filesystem::path& filepath);

		[[nodiscard]] static const Memory::Shared<Texture2D>& GetWhiteTexure();
	public:
		[[nodiscard]] static Memory::Shared<RenderingContext> Initialize(GLFWwindow * window);
		[[nodiscard]] static Memory::Shared<RenderingContext> GetRenderingContext();

		[[nodiscard]] static const ShaderLibrary* GetShaderLibrary();

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

		[[nodiscard]]  static Memory::CommandBuffer& GetRenderingCommandBuffer();
	private:
	};
}