#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <Radiant/Core/Application.hpp>

#include <Radiant/Rendering/Image.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/2D/Rendering2D.hpp>
#include <Radiant/Rendering/Framebuffer.hpp>
#include <Radiant/Rendering/Pipeline.hpp>
#include <Radiant/Rendering/Material.hpp>
#include <Radiant/Rendering/Mesh.hpp>
#include <Radiant/Scene/Components.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLImage.hpp>

#include <Radiant/Scene/Scene.hpp>

#include <Radiant/Rendering/SceneRendering.hpp>

namespace Radiant
{
    enum Bindings : uint32_t
    {
        Transformations          = 0,
        Lights                   = 2,
        EnvironmentMapAttributes = 10,
    };

    struct PointLightsDeclaration
    {
        uint32_t   Count{ 0 };
        PointLight PointLights[1024]{};
    };

    struct UBTransformations
    {
        glm::mat4 ViewProjectionMatrix;
        glm::mat4 InversedViewProjectionMatrix;
        glm::mat4 ViewMatrix;
        glm::mat4 ProjectionMatrix;
        glm::vec3 CameraPosition;
    };
    static constexpr uint32_t kUBTransformationsSize = sizeof( UBTransformations );

    struct UBLights
    {
        DirectionalLight       directionalLight;
        PointLightsDeclaration pointLights;
    };

    static constexpr int kShadowMapSize           = 4096;
    static constexpr int kBRDF_LUT_Size           = 256;
    static constexpr int kLightEnvironmentSize    = sizeof( UBLights );
    static constexpr int kUBEnvironmentAttributes = sizeof( EnvironmentAttributes );

    struct GeometryData
    {
        Memory::Shared<Pipeline> pipeline;
        Memory::Shared<Material> material;
    };

    struct Shadow
    {
        Memory::Shared<Pipeline> ShadowPassPipeline[4];
        Memory::Shared<Material> ShadowMapMaterial;
        float                    ShadowMapSize = 20.0f;
        float                    LightDistance = 0.1f;
        glm::mat4                LightMatrices[4];
        glm::mat4                LightViewMatrix;
        float                    CascadeSplitLambda = 0.91f;
        glm::vec4                CascadeSplits;
        float                    CascadeFarPlaneOffset = 15.0f, CascadeNearPlaneOffset = -15.0f;
        bool                     ShowCascades          = false;
        bool                     SoftShadows           = true;
        float                    LightSize             = 0.5f;
        float                    MaxShadowDistance     = 200.0f;
        float                    ShadowFade            = 25.0f;
        float                    CascadeTransitionFade = 1.0f;
        bool                     CascadeFading         = true;

        RenderingID ShadowMapSampler;
    };

    struct CompositeData
    {
        Memory::Shared<Pipeline> pipeline;
        Memory::Shared<Material> material;
    };

    struct RenderPassList
    {
        GeometryData  Geometry;
        GeometryData  GeometryAnimated;
        GeometryData  SelectedGeomerty;
        CompositeData Composite;
        Shadow        Shadow;
    };

    struct DrawCommandRigged
    {
        glm::mat4                             Transform;
        std::optional<std::vector<glm::mat4>> BoneTransforms;
        Memory::Shared<AnimatedMesh>          Mesh;
    };

    struct DrawCommand
    {
        glm::mat4                  Transform;
        Memory::Shared<StaticMesh> Mesh;
    };

    struct SceneInfo
    {

        struct RenderPassList          RenderPassList;
        Memory::Shared<Shader>         DefaultShader;
        std::vector<DrawCommand>       StaticMeshDrawList;
        std::vector<DrawCommand>       StaticSelectedMeshDrawList;
        std::vector<DrawCommandRigged> RiggedMeshDrawList;
        Memory::Shared<Pipeline>       GridPipeline;
        Memory::Shared<Material>       GridMaterial;
        Memory::Shared<Pipeline>       SkyboxPipeline;
        Memory::Shared<Material>       SkyboxMaterial;
        struct LightEnvironment        LightEnvironment;
        Memory::Shared<Texture2D>      BRDF_LUT;
        UBLights                       LightUB;

        struct
        {
            glm::mat4 ViewProjection;
            glm::mat4 View;
            glm::mat4 Projection;
            glm::mat4 InversedViewProjection;
            glm::vec3 CameraPos;
            float     Exposure;
        } SceneCamera;

        uint32_t                     ViewportWidth;
        uint32_t                     ViewportHeight;
        Memory::Shared<Scene>        ActiveScene;
        Environment                  EnvironmentMap;
        struct EnvironmentAttributes Attributes;
    };

    static SceneInfo* s_SceneInfo = nullptr;

    SceneRendering& SceneRendering::Get()
    {
        static SceneRendering instance;
        return instance;
    }

    SceneRendering::~SceneRendering()
    {
        delete s_SceneInfo;
    }

    void SceneRendering::Init()
    {
        s_SceneInfo = new SceneInfo();

        s_SceneInfo->ViewportWidth  = 1280;
        s_SceneInfo->ViewportHeight = 720;

        // Geometry pass

        {
            RenderPassSpecification renderPassSpec;
            renderPassSpec.TargetFramebuffer =
                 Framebuffer::Create( { s_SceneInfo->ViewportWidth,
                                        s_SceneInfo->ViewportHeight,
                                        8,
                                        { ImageFormat::RGBA16F, ImageFormat::DEPTH32F } } );
            renderPassSpec.DebugName = "Geometry Render Pass";

            {
                // Geometry static pass

                PipelineSpecification pipelineSpecification;
                pipelineSpecification.Layout = { { ShaderDataType::Float3, "a_Position" },
                                                 { ShaderDataType::Float3, "a_Normals" },
                                                 { ShaderDataType::Float2, "a_TexCoord" },
                                                 { ShaderDataType::Float3, "a_Tangent" },
                                                 { ShaderDataType::Float3, "a_Bitangent" } };

                pipelineSpecification.DebugName  = "PBR-Static";
                pipelineSpecification.RenderPass = RenderPass::Create( renderPassSpec );
                pipelineSpecification.Shader     = Rendering::GetShaderLibrary()->Get( "StaticPBR_Radiant.glsl" );

                s_SceneInfo->RenderPassList.Geometry.pipeline = Pipeline::Create( pipelineSpecification );
                s_SceneInfo->RenderPassList.Geometry.material = Material::Create( pipelineSpecification.Shader );
            }

            {
                // Geometry animated pass

                PipelineSpecification pipelineSpecification;
                pipelineSpecification.Layout = {
                     { ShaderDataType::Float3, "a_Position" },   { ShaderDataType::Float3, "a_Normals" },
                     { ShaderDataType::Float2, "a_TexCoord" },   { ShaderDataType::Float3, "a_Tangent" },
                     { ShaderDataType::Float3, "a_Bitangent" },  { ShaderDataType::Int4, "a_BoneIndices" },
                     { ShaderDataType::Float4, "a_BoneWeights" } };

                pipelineSpecification.DebugName  = "PBR-Animated";
                pipelineSpecification.RenderPass = RenderPass::Create( renderPassSpec );
                pipelineSpecification.Shader     = Rendering::GetShaderLibrary()->Get( "AnimPBR_Radiant.glsl" );

                s_SceneInfo->RenderPassList.GeometryAnimated.pipeline = Pipeline::Create( pipelineSpecification );
                s_SceneInfo->RenderPassList.GeometryAnimated.material =
                     Material::Create( pipelineSpecification.Shader );
            }
        }

        // Composite pass

        {
            RenderPassSpecification renderPassSpec;
            renderPassSpec.TargetFramebuffer = Framebuffer::Create(
                 { s_SceneInfo->ViewportWidth, s_SceneInfo->ViewportHeight, 1, { ImageFormat::RGBA16F } } );
            renderPassSpec.DebugName = "Composite Render Pass";

            PipelineSpecification pipelineSpecification;
            pipelineSpecification.Layout = { { ShaderDataType::Float3, "a_Position" },
                                             { ShaderDataType::Float2, "a_TexCoord" } };

            pipelineSpecification.DebugName  = "Scene Composite";
            pipelineSpecification.RenderPass = RenderPass::Create( renderPassSpec );
            pipelineSpecification.Shader = Rendering::GetShaderLibrary()->Get( "SceneCompositeMSAA.glsl" ); // TODO
            s_SceneInfo->RenderPassList.Composite.pipeline = Pipeline::Create( pipelineSpecification );

            s_SceneInfo->RenderPassList.Composite.material = Material::Create( pipelineSpecification.Shader );
        }

        // Selected geometry 

        {

            RenderPassSpecification renderPassSpec;
            renderPassSpec.TargetFramebuffer =
                 Framebuffer::Create( { s_SceneInfo->ViewportWidth,
                                        s_SceneInfo->ViewportHeight,
                                        8,
                                        { ImageFormat::RGBA16F, ImageFormat::DEPTH32F } } );
            renderPassSpec.DebugName = "Selected Geometry Render Pass";

            PipelineSpecification pipelineSpecification;
            pipelineSpecification.Layout = { { ShaderDataType::Float3, "a_Position" },
                                             { ShaderDataType::Float3, "a_Normals" },
                                             { ShaderDataType::Float2, "a_TexCoord" },
                                             { ShaderDataType::Float3, "a_Tangent" },
                                             { ShaderDataType::Float3, "a_Bitangent" } };

            pipelineSpecification.DebugName  = "Selected-Static";
            pipelineSpecification.RenderPass = RenderPass::Create( renderPassSpec );
            pipelineSpecification.Shader     = Rendering::GetShaderLibrary()->Get( "SelectedGeometry.glsl" );

            s_SceneInfo->RenderPassList.SelectedGeomerty.pipeline = Pipeline::Create( pipelineSpecification );
            s_SceneInfo->RenderPassList.SelectedGeomerty.material = Material::Create( pipelineSpecification.Shader );
        }

        // Grid

        {
            const auto& gridShader    = Rendering::GetShaderLibrary()->Get( "Grid.glsl" );
            s_SceneInfo->GridMaterial = Material::Create( gridShader );
            s_SceneInfo->GridMaterial->SetFlag( MaterialFlag::TwoSided, true );
            s_SceneInfo->GridMaterial->SetFlag( MaterialFlag::DepthTest, true );

            PipelineSpecification pipelineSpec;
            pipelineSpec.DebugName  = "Grid";
            pipelineSpec.Shader     = gridShader;
            pipelineSpec.Layout     = { { ShaderDataType::Float3, "a_Position" },
                                        { ShaderDataType::Float2, "a_TexCoord" } };
            pipelineSpec.RenderPass = s_SceneInfo->RenderPassList.Geometry.pipeline->GetSpecification().RenderPass;
            s_SceneInfo->GridPipeline = Pipeline::Create( pipelineSpec );
        }

        // Skybox

        {
            const auto& skyboxShader = Rendering::GetShaderLibrary()->Get( "Skybox.glsl" );

            PipelineSpecification pipelineSpec;
            pipelineSpec.DebugName      = "Skybox";
            pipelineSpec.Shader         = skyboxShader;
            pipelineSpec.Layout         = { { ShaderDataType::Float3, "a_Position" },
                                            { ShaderDataType::Float2, "a_TexCoord" } };
            s_SceneInfo->SkyboxPipeline = Pipeline::Create( pipelineSpec );
            s_SceneInfo->SkyboxMaterial = Material::Create( skyboxShader );
        }

        // Shadow map

        {
            PipelineSpecification ps;
            ps.DebugName = "Shadow map";
            ps.Layout    = { { ShaderDataType::Float3, "a_Position" },   { ShaderDataType::Float3, "a_Normals" },
                             { ShaderDataType::Float2, "a_TexCoord" },   { ShaderDataType::Float3, "a_Tangent" },
                             { ShaderDataType::Float3, "a_Bitangent" },  { ShaderDataType::Int4, "a_BoneIndices" },
                             { ShaderDataType::Float4, "a_BoneWeights" } };
            ps.Shader    = Rendering::GetShaderLibrary()->Get( "ShadowMap.glsl" );
            s_SceneInfo->RenderPassList.Shadow.ShadowMapMaterial = Material::Create( ps.Shader );

            // 4 cascades
            for ( int i = 0; i < 4; i++ )
            {
                RenderPassSpecification shadowMapRenderPassSpec;
                shadowMapRenderPassSpec.TargetFramebuffer =
                     Framebuffer::Create( { kShadowMapSize, kShadowMapSize, 1, { ImageFormat::DEPTH32F }, true } );
                shadowMapRenderPassSpec.DebugName = "Geometry Render Pass";

                ps.RenderPass = RenderPass::Create( shadowMapRenderPassSpec );

                s_SceneInfo->RenderPassList.Shadow.ShadowPassPipeline[i] = Pipeline::Create( ps );
            }

            Rendering::SubmitCommand(
                 []()
                 {
                     glGenSamplers( 1, &s_SceneInfo->RenderPassList.Shadow.ShadowMapSampler );

                     // Setup the shadowmap depth sampler
                     glSamplerParameteri( s_SceneInfo->RenderPassList.Shadow.ShadowMapSampler,
                                          GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                     glSamplerParameteri( s_SceneInfo->RenderPassList.Shadow.ShadowMapSampler,
                                          GL_TEXTURE_MIN_FILTER, GL_LINEAR );
                     glSamplerParameteri( s_SceneInfo->RenderPassList.Shadow.ShadowMapSampler, GL_TEXTURE_WRAP_S,
                                          GL_CLAMP_TO_EDGE );
                     glSamplerParameteri( s_SceneInfo->RenderPassList.Shadow.ShadowMapSampler, GL_TEXTURE_WRAP_T,
                                          GL_CLAMP_TO_EDGE );
                 } );
        }

        // Uniform buffers

        {
            m_UniformBufferInfo = Memory::Shared<UniformBufferInfo>::Create();

            m_UniformBufferInfo->Create( kUBTransformationsSize, Bindings::Transformations );
            m_UniformBufferInfo->Create( kLightEnvironmentSize, Bindings::Lights );
            m_UniformBufferInfo->Create( kUBEnvironmentAttributes, Bindings::EnvironmentMapAttributes );
        }

        s_SceneInfo->BRDF_LUT = Texture2D::Create( "Resources/Textures/BRDF_LUT.tga" );
        s_SceneInfo->StaticMeshDrawList.reserve( 100 ); // TODO: Add a capcaity from YAML(scene)
        s_SceneInfo->RiggedMeshDrawList.reserve( 100 ); // TODO: Add a capcaity from YAML(scene)
    }

    void SceneRendering::BeginScene( Memory::Shared<Scene> scene, const Camera& camera )
    {
        RADIANT_VERIFY( s_SceneInfo, "Did you call Init() ?" );
        RADIANT_VERIFY( !s_SceneInfo->ActiveScene, "There active scene! Can you call EndScene()?" );
        RADIANT_VERIFY( scene );
        s_SceneInfo->ActiveScene = scene;

        s_SceneInfo->SceneCamera.ViewProjection         = camera.GetViewProjection();
        s_SceneInfo->SceneCamera.View                   = camera.GetViewMatrix();
        s_SceneInfo->SceneCamera.Projection             = camera.GetProjectionMatrix();
        s_SceneInfo->SceneCamera.CameraPos              = camera.GetPosition();
        s_SceneInfo->SceneCamera.Exposure               = 0.8f; // camera.GetExposure();
        s_SceneInfo->SceneCamera.InversedViewProjection = glm::inverse( camera.GetViewProjection() );

        s_SceneInfo->LightEnvironment = s_SceneInfo->ActiveScene->GetLightEnvironment();

        UBLights& lightsUB         = s_SceneInfo->LightUB;
        lightsUB.directionalLight  = s_SceneInfo->LightEnvironment.DirectionalLights;
        lightsUB.pointLights.Count = s_SceneInfo->LightEnvironment.PointLights.size();
        std::memcpy( lightsUB.pointLights.PointLights, s_SceneInfo->LightEnvironment.PointLights.data(),
                     s_SceneInfo->LightEnvironment.GetPointLightsSize() );

        m_UniformBufferInfo->Get( Bindings::EnvironmentMapAttributes )
             ->SetData( &s_SceneInfo->Attributes.EnvironmentMapLod, kUBEnvironmentAttributes );
        m_UniformBufferInfo->Get( Bindings::Lights )->SetData( &lightsUB, kLightEnvironmentSize );
    }

    void SceneRendering::EndScene()
    {
        RADIANT_VERIFY( s_SceneInfo->ActiveScene, "No active scene! Can you call BeginScene()?" );
        s_SceneInfo->ActiveScene = nullptr;
    }

    void SceneRendering::SetSceneVeiwPortSize( const glm::vec2& size )
    {
        RADIANT_VERIFY( s_SceneInfo, "Did you call Init() ?" );
        RADIANT_VERIFY( s_SceneInfo->ActiveScene, "Did you call BeginScene() ?" );

        if ( s_SceneInfo->ViewportWidth != size.x || s_SceneInfo->ViewportHeight != size.y )
        {
            s_SceneInfo->ViewportWidth  = size.x;
            s_SceneInfo->ViewportHeight = size.y;

            s_SceneInfo->RenderPassList.Geometry.pipeline->GetSpecification()
                 .RenderPass->GetSpecification()
                 .TargetFramebuffer->Resize( size.x, size.y );

            s_SceneInfo->RenderPassList.GeometryAnimated.pipeline->GetSpecification()
                 .RenderPass->GetSpecification()
                 .TargetFramebuffer->Resize( size.x, size.y );

            s_SceneInfo->RenderPassList.Composite.pipeline->GetSpecification()
                 .RenderPass->GetSpecification()
                 .TargetFramebuffer->Resize( size.x, size.y );
        }
    }

    void SceneRendering::UpdateEnvTextures( const Memory::Shared<Material>& material )
    {
        ImageDescriptor descriptor;

        descriptor.Name = "u_EnvRadianceTex";
        material->SetImage2D( descriptor, s_SceneInfo->EnvironmentMap.Radiance );

        descriptor.Name = "u_EnvIrradianceTex";
        material->SetImage2D( descriptor, s_SceneInfo->EnvironmentMap.Irradiance );

        descriptor.Name = "u_BRDFLUTTexture";
        material->SetImage2D( descriptor, s_SceneInfo->BRDF_LUT->GetImage2D() );
    }

    void SceneRendering::UploadMeshMaterials( const Memory::Shared<Material>& material )
    {
        material->SetImage2D( "u_EnvRadianceTex", s_SceneInfo->EnvironmentMap.Radiance );
        material->SetImage2D( "u_EnvIrradianceTex ", s_SceneInfo->EnvironmentMap.Irradiance );
        material->SetImage2D( "u_BRDFLUTTexture ", s_SceneInfo->BRDF_LUT->GetImage2D() );
    }

    void SceneRendering::SetEnvironment( const Environment& env )
    {
        s_SceneInfo->EnvironmentMap = env;
        ImageDescriptor descriptor;

        descriptor.Name = "u_EnvTexture";
        s_SceneInfo->SkyboxMaterial->SetImage2D( descriptor, env.Radiance );

        UpdateEnvTextures( s_SceneInfo->RenderPassList.Geometry.material );
        UpdateEnvTextures( s_SceneInfo->RenderPassList.GeometryAnimated.material );
    }

    void SceneRendering::SetEnvironmentAttributes( const struct EnvironmentAttributes& attributes )
    {
        RADIANT_VERIFY( s_SceneInfo, "Did you call Init() ?" );
        s_SceneInfo->Attributes = attributes;
    }

    void SceneRendering::SetEnvMapRotation( float rotation )
    {
        RADIANT_VERIFY( s_SceneInfo, "Did you call Init() ?" );
        s_SceneInfo->Attributes.Rotation = rotation;
    }

    void SceneRendering::SetIBLContribution( float value )
    {
        RADIANT_VERIFY( s_SceneInfo, "Did you call Init() ?" );
        s_SceneInfo->Attributes.Rotation = value;
    }

    void SceneRendering::OnImGuiRender()
    {
        RADIANT_VERIFY( s_SceneInfo, "Did you call Init() ?" );
    }

    void SceneRendering::SubmitAnimatedMesh( const Memory::Shared<AnimatedMesh>& mesh,
                                             const std::vector<glm::mat4>&       boneTransforms,
                                             const glm::mat4&                    transform )
    {
        RADIANT_VERIFY( s_SceneInfo, "Did you call Init() ?" );

        auto&                  boneInfo = mesh.As<AnimatedMesh>()->GetBoneInfo();
        std::vector<glm::mat4> updatedBoneTransforms( boneTransforms.size() );
        for ( const auto& bone : boneInfo )
        {
            updatedBoneTransforms[bone.second.ID] =
                 mesh->GetGlobalInverseTransform() * boneTransforms[bone.second.ID] * bone.second.BoneOffset;

            //    // TODO:
            //    // the basic logic is that our const auto transform is stored in TransformComponent we get it,
            //    // then we count all transformations on the level above, and here we already equate inversed
            //    matrix
        }

        s_SceneInfo->RiggedMeshDrawList.push_back( { transform, updatedBoneTransforms, mesh } );
    }

    void SceneRendering::SubmitStaticMesh( const Memory::Shared<StaticMesh>& mesh, const glm::mat4& transform )
    {
        RADIANT_VERIFY( s_SceneInfo, "Did you call Init() ?" );
        s_SceneInfo->StaticMeshDrawList.push_back( { transform, mesh } );
        s_SceneInfo->StaticSelectedMeshDrawList.push_back( { transform, mesh } );
    }

    Radiant::Memory::Shared<Radiant::Image2D> SceneRendering::GetFinalPassImage()
    {
        RADIANT_VERIFY( s_SceneInfo, "Did you call Init() ?" );
        return s_SceneInfo->RenderPassList.Composite.pipeline->GetSpecification()
             .RenderPass->GetSpecification()
             .TargetFramebuffer->GetColorAttachmentImage( 0 );
    }

    Radiant::Memory::Shared<Radiant::Image2D> SceneRendering::GetShadowMapPassImage()
    {
        RADIANT_VERIFY( s_SceneInfo, "Did you call Init() ?" );
        return nullptr;
    }

    Radiant::Environment SceneRendering::CreateEnvironmentMap( const std::filesystem::path& filepath )
    {
        RADIANT_VERIFY( s_SceneInfo, "Did you call Init() ?" );
        return Rendering::CreateEnvironmentMap( filepath );
    }

    struct FrustumBounds
    {
        float r, l, b, t, f, n;
    };

    struct CascadeData
    {
        glm::mat4 ViewProj;
        glm::mat4 View;
        float     SplitDepth;
    };

    static void CalculateCascades( CascadeData* cascades, const glm::vec3& lightDirection )
    {
        FrustumBounds frustumBounds[3] = {};

        auto viewProjection = s_SceneInfo->SceneCamera.ViewProjection;

        const int                                   SHADOW_MAP_CASCADE_COUNT = 4;
        std::array<float, SHADOW_MAP_CASCADE_COUNT> cascadeSplits            = { 0.0 };
        cascadeSplits.fill( 0.0 );

        // TODO: less hard-coding!
        float nearClip  = 0.1f;
        float farClip   = 1000.0f;
        float clipRange = farClip - nearClip;

        float minZ = nearClip;
        float maxZ = nearClip + clipRange;

        float range = maxZ - minZ;
        float ratio = maxZ / minZ;

        // Calculate split depths based on view camera frustum
        // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
        for ( uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++ )
        {
            float p          = ( i + 1 ) / static_cast<float>( SHADOW_MAP_CASCADE_COUNT );
            float log        = minZ * std::pow( ratio, p );
            float uniform    = minZ + range * p;
            float d          = s_SceneInfo->RenderPassList.Shadow.CascadeSplitLambda * ( log - uniform ) + uniform;
            cascadeSplits[i] = ( d - nearClip ) / clipRange;
        }

        cascadeSplits[3] = 0.3f;

        // Manually set cascades here
        // cascadeSplits[0] = 0.05f;
        // cascadeSplits[1] = 0.15f;
        // cascadeSplits[2] = 0.3f;
        // cascadeSplits[3] = 1.0f;

        // Calculate orthographic projection matrix for each cascade
        float lastSplitDist = 0.0;
        for ( uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++ )
        {
            float splitDist = cascadeSplits[i];

            glm::vec3 frustumCorners[8] = {
                 glm::vec3( -1.0f, 1.0f, -1.0f ), glm::vec3( 1.0f, 1.0f, -1.0f ),
                 glm::vec3( 1.0f, -1.0f, -1.0f ), glm::vec3( -1.0f, -1.0f, -1.0f ),
                 glm::vec3( -1.0f, 1.0f, 1.0f ),  glm::vec3( 1.0f, 1.0f, 1.0f ),
                 glm::vec3( 1.0f, -1.0f, 1.0f ),  glm::vec3( -1.0f, -1.0f, 1.0f ),
            };

            // Project frustum corners into world space
            glm::mat4 invCam = glm::inverse( viewProjection );
            for ( uint32_t i = 0; i < 8; i++ )
            {
                glm::vec4 invCorner = invCam * glm::vec4( frustumCorners[i], 1.0f );
                frustumCorners[i]   = invCorner / invCorner.w;
            }

            for ( uint32_t i = 0; i < 4; i++ )
            {
                glm::vec3 dist        = frustumCorners[i + 4] - frustumCorners[i];
                frustumCorners[i + 4] = frustumCorners[i] + ( dist * splitDist );
                frustumCorners[i]     = frustumCorners[i] + ( dist * lastSplitDist );
            }

            // Get frustum center
            glm::vec3 frustumCenter = glm::vec3( 0.0f );
            for ( uint32_t i = 0; i < 8; i++ )
                frustumCenter += frustumCorners[i];

            frustumCenter /= 8.0f;

            // frustumCenter *= 0.01f;

            float radius = 0.0f;
            for ( uint32_t i = 0; i < 8; i++ )
            {
                float distance = glm::length( frustumCorners[i] - frustumCenter );
                radius         = glm::max( radius, distance );
            }
            radius = std::ceil( radius * 16.0f ) / 16.0f;

            glm::vec3 maxExtents = glm::vec3( radius );
            glm::vec3 minExtents = -maxExtents;

            glm::vec3 lightDir         = -lightDirection;
            glm::mat4 lightViewMatrix  = glm::lookAt( frustumCenter - lightDir * -minExtents.z, frustumCenter,
                                                      glm::vec3( 0.0f, 0.0f, 1.0f ) );
            glm::mat4 lightOrthoMatrix = glm::ortho(
                 minExtents.x, maxExtents.x, minExtents.y, maxExtents.y,
                 0.0f + s_SceneInfo->RenderPassList.Shadow.CascadeNearPlaneOffset,
                 maxExtents.z - minExtents.z + s_SceneInfo->RenderPassList.Shadow.CascadeFarPlaneOffset );

            // Store split distance and matrix in cascade
            cascades[i].SplitDepth = ( nearClip + splitDist * clipRange ) * -1.0f;
            cascades[i].ViewProj   = lightOrthoMatrix * lightViewMatrix;
            cascades[i].View       = lightViewMatrix;

            lastSplitDist = cascadeSplits[i];
        }
    }

    void SceneRendering::GeometryPass()
    {
        Rendering::BeginRenderPass( s_SceneInfo->RenderPassList.Geometry.pipeline->GetSpecification().RenderPass );

        s_SceneInfo->SkyboxPipeline->GetSpecification().Shader->Use();
        Rendering::SubmitFullscreenQuad( s_SceneInfo->SkyboxPipeline, s_SceneInfo->SkyboxMaterial );

        const auto& options = s_SceneInfo->ActiveScene->GetSceneOptions();

        for ( const auto& dc : s_SceneInfo->RiggedMeshDrawList )
        {
            DrawSpecificationCommandWithMaterial command;
            command.Material = dc.Mesh->GetMaterial();
            UploadMeshMaterials( command.Material );
            command.Pipeline   = s_SceneInfo->RenderPassList.GeometryAnimated.pipeline;
            command.Declration = { dc.Transform, dc.BoneTransforms, dc.Mesh };

            Rendering::SubmitMeshWithMaterial( command );
        }

        for ( const auto& dc : s_SceneInfo->StaticSelectedMeshDrawList )
        {
        }

        for ( const auto& dc : s_SceneInfo->StaticMeshDrawList )
        {
            DrawSpecificationCommandWithMaterial command;
            command.Material = dc.Mesh->GetMaterial();
            UploadMeshMaterials( command.Material );
            command.Pipeline   = s_SceneInfo->RenderPassList.Geometry.pipeline;
            command.Declration = { dc.Transform, std::nullopt, dc.Mesh };

            if ( options.ShowAABB )
            {
                Rendering2D::Get().BeginScene( {} ); // TODO: move to Rendering class
                Rendering::DrawAABB( dc.Mesh, dc.Transform );
                Rendering2D::Get().EndScene();
            }

            Rendering::SubmitMeshWithMaterial( command );
        }
        // for ( const auto& mesh : s_SceneInfo->MeshDrawList )
        //{
        //     // Env. map
        //     TextureDescriptor descriptor;

        //    descriptor.Name = "u_EnvRadianceTex";
        //    s_SceneInfo->RenderPassList.Geometry.material->SetImage2D(
        //         descriptor, s_SceneInfo->EnvironmentMap.Radiance ); // TODO: create ubo, contatins the textures

        //    descriptor.Name = "u_EnvIrradianceTex";
        //    s_SceneInfo->RenderPassList.Geometry.material->SetImage2D( descriptor,
        //                                                               s_SceneInfo->EnvironmentMap.Irradiance );

        //    descriptor.Name = "u_BRDFLUTTexture";
        //    s_SceneInfo->RenderPassList.Geometry.material->SetImage2D( descriptor,
        //                                                               s_SceneInfo->BRDF_LUT->GetImage2D() );

        //    // Shadow

        //    TextureDescriptor shadowDescriptor;
        //    shadowDescriptor.Name    = "u_ShadowMapTexture";
        //    shadowDescriptor.Sampler = s_SceneInfo->RenderPassList.Shadow.ShadowMapSampler;

        //    for ( int i = 0; i < 4; i++ )
        //    {
        //        shadowDescriptor.ArrayIndex = i;

        //        s_SceneInfo->RenderPassList.Geometry.material->SetMat4(
        //             "u_LightMatrixCascade", s_SceneInfo->RenderPassList.Shadow.LightMatrices[i], i );
        //        s_SceneInfo->RenderPassList.Geometry.material->SetImage2D(
        //             shadowDescriptor, s_SceneInfo->RenderPassList.Shadow.ShadowPassPipeline[i]
        //                                    ->GetSpecification()
        //                                    .RenderPass->GetSpecification()
        //                                    .TargetFramebuffer->GetDepthAttachmentImage() );
        //    }

        //    s_SceneInfo->RenderPassList.Geometry.material->SetVec4(
        //         "u_CascadeSplits", s_SceneInfo->RenderPassList.Shadow.CascadeSplits );
        //    s_SceneInfo->RenderPassList.Geometry.material->SetMat4(
        //         "u_LightView", s_SceneInfo->RenderPassList.Shadow.LightViewMatrix );

        //    DrawSpecificationCommandWithMaterial command;
        //    command.Material   = s_SceneInfo->RenderPassList.Geometry.material;
        //    command.Declration = { mesh.Transform, mesh.BoneTransforms, mesh.Mesh };

        //    Rendering::SubmitMeshWithMaterial( command, s_SceneInfo->RenderPassList.Geometry.pipeline );

        //    if ( options.ShowAABB )
        //    {
        //        Rendering2D::Get().BeginScene( {} ); // TODO: move to Rendering class
        //        Rendering::DrawAABB( mesh.Mesh, mesh.Transform );
        //        Rendering2D::Get().EndScene();
        //    }
        //}

        if ( options.ShowGrid )
        {
            Rendering::SubmitFullscreenQuad( s_SceneInfo->GridPipeline, s_SceneInfo->GridMaterial );
        }

        Rendering::EndRenderPass();
    }

    void SceneRendering::ShadowMapPass()
    {
        auto& directionalLights = s_SceneInfo->LightEnvironment.DirectionalLights;
        if ( directionalLights.Intensity == 0.0f || !directionalLights.CastShadows )
        {
            for ( int i = 0; i < 4; i++ )
            {
                // Clear shadow maps
                Rendering::BeginRenderPass(
                     s_SceneInfo->RenderPassList.Shadow.ShadowPassPipeline[i]->GetSpecification().RenderPass );
                Rendering::EndRenderPass();
            }
            return;
        }
        Rendering::SubmitCommand(
             []()
             {
                 glEnable( GL_CULL_FACE );
                 glCullFace( GL_BACK );
             } );

        CascadeData cascades[4];
        CalculateCascades( cascades, directionalLights.Direction );
        s_SceneInfo->RenderPassList.Shadow.LightViewMatrix = cascades[0].View;

        /*for ( int i = 0; i < 4; i++ )
        {
            Rendering::BeginRenderPass(
                 s_SceneInfo->RenderPassList.Shadow.ShadowPassPipeline[i]->GetSpecification().RenderPass );

            s_SceneInfo->RenderPassList.Shadow.CascadeSplits[i] = cascades[i].SplitDepth;
            glm::mat4 shadowMapVP                               = cascades[i].ViewProj;
            s_SceneInfo->RenderPassList.Shadow.ShadowMapMaterial->SetMat4( "u_ViewProjection", shadowMapVP );

            static glm::mat4 scaleBiasMatrix = glm::scale( glm::mat4( 1.0f ), { 0.5f, 0.5f, 0.5f } ) *
                                               glm::translate( glm::mat4( 1.0f ), { 1, 1, 1 } );
            s_SceneInfo->RenderPassList.Shadow.LightMatrices[i] = scaleBiasMatrix * cascades[i].ViewProj;

            for ( const auto& mesh : s_SceneInfo->MeshDrawList )
            {
                Rendering::SubmitMesh( { mesh.Transform, std::nullopt, mesh.Mesh },
                                       s_SceneInfo->RenderPassList.Shadow.ShadowPassPipeline[i],
                                       s_SceneInfo->RenderPassList.Shadow.ShadowMapMaterial );
            }

            Rendering::EndRenderPass();
        }*/

        Rendering::SubmitCommand( []() { glDisable( GL_CULL_FACE ); } );
    }

    void SceneRendering::CompositePass()
    {
        Rendering::BeginRenderPass(
             s_SceneInfo->RenderPassList.Composite.pipeline->GetSpecification().RenderPass );
        s_SceneInfo->RenderPassList.Composite.material->SetFloat(
             "u_Exposure", s_SceneInfo->SceneCamera.Exposure ); // TODO: move to the UBO
        s_SceneInfo->RenderPassList.Composite.material->SetUint(
             "u_SamplesCount", s_SceneInfo->ActiveScene->GetSceneSamplesCount() );

        ImageDescriptor descriptor;
        descriptor.Name = "u_Texture";
        s_SceneInfo->RenderPassList.Composite.material->SetImage2D(
             descriptor, s_SceneInfo->RenderPassList.Geometry.pipeline->GetSpecification()
                              .RenderPass->GetSpecification()
                              .TargetFramebuffer->GetColorAttachmentImage() );

        s_SceneInfo->RenderPassList.Composite.pipeline->GetSpecification().Shader->Use();
        Rendering::SubmitFullscreenQuad( s_SceneInfo->RenderPassList.Composite.pipeline,
                                         s_SceneInfo->RenderPassList.Composite.material );
        Rendering::EndRenderPass();
    }

    void SceneRendering::FlushDrawList()
    {
        ShadowMapPass();
        GeometryPass();
        CompositePass();

        s_SceneInfo->StaticMeshDrawList.clear(); // TODO: Optimize
        s_SceneInfo->RiggedMeshDrawList.clear(); // TODO: Optimize
    }

    void SceneRendering::OnUpdate( Timestep ts )
    {
        RADIANT_VERIFY( s_SceneInfo, "Did you call Init() ?" );

        static const glm::mat4 transform =
             glm::rotate( glm::mat4( 1.0f ), glm::radians( 90.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) ) *
             glm::scale( glm::mat4( 1.0f ), glm::vec3( 16.0f ) );

        FlushDrawList();

        UBTransformations transformations;
        transformations.CameraPosition               = s_SceneInfo->SceneCamera.CameraPos;
        transformations.InversedViewProjectionMatrix = s_SceneInfo->SceneCamera.InversedViewProjection;
        transformations.ProjectionMatrix             = s_SceneInfo->SceneCamera.Projection;
        transformations.ViewMatrix                   = s_SceneInfo->SceneCamera.View;
        transformations.ViewProjectionMatrix         = s_SceneInfo->SceneCamera.ViewProjection;

        m_UniformBufferInfo->Get( Bindings::Transformations )->SetData( &transformations, kUBTransformationsSize );

        s_SceneInfo->GridMaterial->SetMat4( "u_Transform", transform ); // TODO: UBO
    }
} // namespace Radiant