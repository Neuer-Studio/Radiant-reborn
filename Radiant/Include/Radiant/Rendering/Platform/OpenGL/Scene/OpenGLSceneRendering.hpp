#pragma once

#include <Radiant/Rendering/Scene/SceneRendering.hpp>
#include <Radiant/Rendering/Shader.hpp>

namespace Radiant
{
	class OpenGLSceneRendering : public SceneRendering
	{
	public:
		OpenGLSceneRendering(const std::string& sceneName);
		~OpenGLSceneRendering() override;
		virtual void Init() override;

		virtual void SubmitScene() const override;

		virtual Environment CreateEnvironmentScene(const std::filesystem::path& filepath) const override;

	private:

	private:
		std::string m_SceneName;
	};
}