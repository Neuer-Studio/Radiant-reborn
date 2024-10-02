#pragma once

#include <Radiant/Rendering/Pipeline.hpp>
#include <Radiant/Rendering/VertexBuffer.hpp>
#include <Radiant/Rendering/IndexBuffer.hpp>
#include <Radiant/Rendering/Material.hpp>
#include <Radiant/Rendering/Mesh.hpp>
#include <Common/Core/Memory/Shared.hpp>
#include <Radiant/Rendering/Environment.hpp>
#include <GLFW/glfw3.h>

namespace Radiant
{
    enum class RenderingAPIType : uint8_t
    {
        None   = 0,
        Vulkan = 1,
        OpenGL = 2
    };

    enum class Primitives
    {
        Triangle = 0,
        Line
    };

    struct GraphicsInfo
    {
        std::string Vendor;
        std::string Renderer;
        std::string Version;

        int   MaxSamples      = 0;
        float MaxAnisotropy   = 0.0f; // Texture filtering
        int   MaxTextureUnits = 0;
    };

    struct RendererResources
    {
        Common::Memory::Shared<Pipeline>     Pipeline;
        Common::Memory::Shared<VertexBuffer> VertexBuffer;
        Common::Memory::Shared<IndexBuffer>  IndexBuffer;
    };

    struct DrawDeclarationCommand
    {
        glm::mat4                             Transform;
        std::optional<std::vector<glm::mat4>> BoneTransforms;
        Common::Memory::Shared<Mesh>                  Mesh;

        bool IsRigged() const
        {
            return BoneTransforms && !BoneTransforms->empty();
        }
    };

    struct DrawSpecificationCommandWithMaterial
    {
        explicit DrawSpecificationCommandWithMaterial() = default;

        DrawDeclarationCommand   Declration;
        Common::Memory::Shared<Material> Material;
        Common::Memory::Shared<Pipeline> Pipeline;
    };

    class RendererAPI : public Common::Memory::RefCounted
    {
    public:
        static GraphicsInfo& GetGraphicsInfo()
        {
            static GraphicsInfo info;
            return info;
        }

        virtual void RT_SubmitFullscreenQuad( const RendererResources&                       resources,
                                              const std::optional<Common::Memory::Shared<Material>>& material ) = 0;

        virtual void SubmitMeshWithMaterial( const DrawSpecificationCommandWithMaterial& specification ) = 0;
        virtual void Clear( const std::array<float, 4>& rgba ) const                                     = 0;
        virtual void DrawPrimitive( Primitives primitive = Primitives::Triangle, uint32_t count = 3,
                                    bool depthTest = true ) const                                        = 0;
        virtual void SetLineWidth( float width ) const                                                   = 0;

    public:
        virtual const Environment CreateEnvironmentMap( const std::filesystem::path& filepath ) const = 0;

    public:
        static const RenderingAPIType GetAPI();
        static void                   SetAPI( RenderingAPIType api );
    };
} // namespace Radiant