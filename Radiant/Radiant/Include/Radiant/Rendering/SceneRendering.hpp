#pragma once

#include <Radiant/Rendering/Texture.hpp>
#include <Radiant/Scene/Scene.hpp>

#include <Radiant/Rendering/UniformBufferInfo.hpp>

namespace Radiant
{
    class Scene;

    class SceneRendering
    {
    public:
        static SceneRendering& Get();

        ~SceneRendering();

    private:
        void BeginScene( Common::Memory::Shared<Scene> m_Scene, const Camera& camera );
        void Init();
        void EndScene();
        void OnUpdate( Common::Timestep ts );

        void SetSceneVeiwPortSize( const glm::vec2& size );
        void SetEnvironment( const Environment& env );

        void SetEnvironmentAttributes( const struct EnvironmentAttributes& attributes );

        void SetEnvMapRotation( float rotation );
        void SetIBLContribution( float value );
        void OnImGuiRender();

        void SubmitAnimatedMesh( const Common::Memory::Shared<AnimatedMesh>& mesh,
                                 const std::vector<glm::mat4>& boneTransforms, const glm::mat4& transform );
        void SubmitStaticMesh( const Common::Memory::Shared<StaticMesh>& mesh, const glm::mat4& transform );

        [[nodiscard]] static Common::Memory::Shared<Image2D> GetFinalPassImage();
        static Common::Memory::Shared<Image2D>               GetShadowMapPassImage();

        [[nodiscard]] static Environment CreateEnvironmentMap( const std::filesystem::path& filepath );

    private:
        void UpdateEnvTextures( const Common::Memory::Shared<Material>& material );
        void UploadMeshMaterials( const Common::Memory::Shared<Material>& material );

    private:
        void FlushDrawList();
        void ShadowMapPass();
        void GeometryPass();
        void CompositePass();

    private:
        Common::Memory::Shared<UniformBufferInfo> m_UniformBufferInfo;

    private:
        friend class Scene;
        friend class Rendering;
        friend class SceneHierarchyPanel;
        friend class Environment;
    };
} // namespace Radiant