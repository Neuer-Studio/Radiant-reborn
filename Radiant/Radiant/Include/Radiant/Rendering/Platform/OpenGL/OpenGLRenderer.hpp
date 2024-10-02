#pragma once

#include <Radiant/Rendering/RendererAPI.hpp>

namespace Radiant
{
    class OpenGLRenderer : public RendererAPI
    {
    public:
        OpenGLRenderer() = default;

        virtual void
                     RT_SubmitFullscreenQuad( const RendererResources&                               resources,
                                              const std::optional<Common::Memory::Shared<Material>>& material ) override;
        virtual void SubmitMeshWithMaterial( const DrawSpecificationCommandWithMaterial& specification ) override;

        virtual void Clear( const std::array<float, 4>& rgba ) const override;
        virtual void DrawPrimitive( Primitives primitive = Primitives::Triangle, uint32_t count = 3,
                                    bool depthTest = true ) const override;
        virtual void SetLineWidth( float width ) const override;

        virtual const Environment CreateEnvironmentMap( const std::filesystem::path& filepath ) const override;

    private:
        static void RT_UpdateMaterialForRendering( const Common::Memory::Shared<Material>& material );
        static void UpdateMaterialForRendering( const Common::Memory::Shared<Material>& material );
        static void RT_BindBuffersAndPipeline( const RendererResources& resources );
        static void BindBuffersAndPipeline( const RendererResources& resources );
    };
} // namespace Radiant