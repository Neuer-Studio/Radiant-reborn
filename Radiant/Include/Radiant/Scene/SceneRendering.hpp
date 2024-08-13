#pragma once

#include <Radiant/Rendering/Texture.hpp>
#include <Radiant/Scene/Scene.hpp>

namespace Radiant
{
    class Scene;

    class SceneRendering
    {
    public:
        static SceneRendering& Get();

        ~SceneRendering();

    private:
        void BeginScene( Memory::Shared<Scene> m_Scene, const Camera& camera );
        void Init();
        void EndScene();
        void OnUpdate( Timestep ts );

        void SetSceneVeiwPortSize( const glm::vec2& size );
        void SetEnvironment( const Environment& env );

        void SetEnvironmentAttributes( const EnvironmentAttributes& attributes );

        void SetEnvMapRotation( float rotation );
        void SetIBLContribution( float value );
        void OnImGuiRender();

        void SubmitAnimatedMesh( const Memory::Shared<AnimatedMesh>& mesh, std::vector<glm::mat4>& boneTransforms,
                         const glm::mat4& transform );
        void SubmitStaticMesh( const Memory::Shared<StaticMesh>& mesh, const glm::mat4& transform );

        [[nodiscard]] static Memory::Shared<Image2D> GetFinalPassImage();
        static Memory::Shared<Image2D>               GetShadowMapPassImage();

        [[nodiscard]] static Environment CreateEnvironmentMap( const std::filesystem::path& filepath );
    private:
        void UpdateEnvTextures(const Memory::Shared<Material>& material);
    private:
        void FlushDrawList();
        void ShadowMapPass();
        void GeometryPass();
        void CompositePass();
    private:

    private:
        friend class Scene;
        friend class Rendering;
        friend class SceneHierarchyPanel;
        friend class Environment;
    };
} // namespace Radiant