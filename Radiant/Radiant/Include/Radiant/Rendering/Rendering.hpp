#pragma once

#include <Radiant/Rendering/RendererAPI.hpp>
#include <Radiant/Rendering/Pipeline.hpp>
#include <Radiant/Rendering/Material.hpp>
#include <Radiant/Rendering/Mesh.hpp>
#include <Radiant/Rendering/RenderingContext.hpp>
#include <Common/Core/Memory/CommandBuffer.hpp>
#include <Common/Core/Math/AABB.hpp>

namespace Radiant
{
    class Rendering : public Common::Memory::RefCounted
    {
    public:
        virtual ~Rendering();

        static void Clear( const std::array<float, 4>& rgba );
        static void SubmitMesh( const DrawDeclarationCommand&           specification,
                                const Common::Memory::Shared<Pipeline>& pipeline,
                                const Common::Memory::Shared<Material>& Material );
        static void SubmitMeshWithMaterial( const DrawSpecificationCommandWithMaterial& specification );
        static void DrawPrimitive( Primitives primitive = Primitives::Triangle, uint32_t count = 3,
                                   bool depthTest = true );
        static void SetLineWidth( float width = 1.0f );
        static void DrawLine( const glm::vec3& p1, const glm::vec3& p2, float lineWidth = 1.0f );
        static void DrawAABB( const Common::Math::AABB& aabb, const glm::mat4& transform );
        static void DrawAABB( const Common::Memory::Shared<Mesh>& mesh, const glm::mat4& transform );

        static void BeginRenderPass( Common::Memory::Shared<RenderPass>& renderPass, bool clear = true );
        static void EndRenderPass();

        [[nodiscard]] static Environment CreateEnvironmentMap( const std::filesystem::path& filepath );

        [[nodiscard]] static const Common::Memory::Shared<Texture2D>& GetWhiteTexure();

    public:
        [[nodiscard]] static Common::Memory::Shared<RenderingContext> Initialize( GLFWwindow* window );
        [[nodiscard]] static Common::Memory::Shared<RenderingContext> GetRenderingContext();

        [[nodiscard]] static const ShaderLibrary* GetShaderLibrary();

    public:
        static void SubmitFullscreenQuad( const Common::Memory::Shared<Pipeline>&                pipeline,
                                          const std::optional<Common::Memory::Shared<Material>>& material );

    public:
        template <typename FuncT>
        static void SubmitCommand( FuncT&& func )
        {
            auto renderCMD = []( void* ptr )
            {
                auto pFunc = (FuncT*)ptr;

                ( *pFunc )();

                pFunc->~FuncT();
            };

            auto storageBuffer = GetRenderingCommandBuffer().AddCommand( renderCMD, sizeof( func ) );
            new ( storageBuffer ) FuncT( std::forward<FuncT>( func ) );
        }

        [[nodiscard]] static Common::Memory::CommandBuffer& GetRenderingCommandBuffer();

    private:
    };
} // namespace Radiant