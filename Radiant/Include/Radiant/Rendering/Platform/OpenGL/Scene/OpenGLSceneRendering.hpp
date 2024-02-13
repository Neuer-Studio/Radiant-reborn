#pragma once

#include <Radiant/Rendering/Scene/SceneRendering.hpp>
#include <Radiant/Rendering/Shader.hpp>

namespace Radiant
{
	class OpenGLSceneRendering : public SceneRendering
	{
	public:
		OpenGLSceneRendering(const Memory::Shared<Scene>& scene);
		~OpenGLSceneRendering() override;
		virtual void Init() override;

		virtual Memory::Shared<Image2D> GetFinalPassImage() const override;

		virtual void SetSceneVeiwPortSize(const glm::vec2& size) override;
		virtual void SubmitScene(Camera* cam) override; 

		virtual void SetEnvironment(const Environment& env) override;

		virtual Environment CreateEnvironmentScene(const std::filesystem::path& filepath) const override;

	private:
		void GeometryPass();
		void Flush();
	private:
		Environment m_Environment;
		Memory::Shared<Scene> m_Scene;

		uint32_t m_ViewportWidth, m_ViewportHeight;
	};
}