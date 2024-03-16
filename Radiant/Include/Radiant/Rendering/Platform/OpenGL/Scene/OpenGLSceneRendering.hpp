#pragma once

#include <Radiant/Scene/SceneRendering.hpp>
#include <Radiant/Rendering/Shader.hpp>

namespace Radiant
{
	struct DirectionalLight;
	class OpenGLSceneRendering : public SceneRendering
	{
	public:
		OpenGLSceneRendering(const Memory::Shared<Scene>& scene);
		~OpenGLSceneRendering() override;
		virtual void Init() override;

		virtual Memory::Shared<Image2D> GetFinalPassImage() const override;
		virtual Memory::Shared<Image2D> GetShadowMapPassImage() const override;

		virtual void SetEnvMapRotation(float rotation) override { m_EnvMapRotation = rotation; }
		virtual void SetRadiancePrefilter(bool enable) override { m_RadiancePrefilter = enable; }

		virtual void SetSceneVeiwPortSize(const glm::vec2& size) override;
		virtual void BeginScene(const Camera& camera) override;

		virtual void OnUpdate(Timestep ts) override;

		virtual void SubmitMesh(const Memory::Shared<Mesh>& mesh, const glm::mat4& transform) const override;

		virtual void SetEnvironment(const Environment& env) override;

		virtual Environment CreateEnvironmentScene(const std::filesystem::path& filepath) const override;

	private:
		void CompositePass();
		void GeometryPass();
		void ShadowMapPass();
		void FlushDrawList();
	private:
		Environment m_Environment;
		Memory::Shared<Scene> m_Scene;

		float m_EnvMapRotation = 0.0f;
		bool m_RadiancePrefilter = false;

		uint32_t m_ViewportWidth, m_ViewportHeight;
	};
}