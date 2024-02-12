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

		virtual void SubmitScene(Camera* cam) const override; 

		virtual void SetEnvironment(const Environment& env) override { m_Environment = env; }

		virtual Environment CreateEnvironmentScene(const std::filesystem::path& filepath) const override;

	private:

	private:
		Environment m_Environment;
		Memory::Shared<Scene> m_Scene;
	};
}