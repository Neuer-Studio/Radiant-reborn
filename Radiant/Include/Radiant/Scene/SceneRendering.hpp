#pragma once

#include <Radiant/Rendering/Texture.hpp>
#include <Radiant/Scene/Scene.hpp>

namespace Radiant
{
	struct Environment
	{
		Memory::Shared<Image2D> Radiance;
		Memory::Shared<Image2D> Irradiance;
	};

	class Scene;

	class SceneRendering
	{
	public:
		virtual ~SceneRendering() = default;

		virtual void BeginScene(const Camera& camera) = 0;
		virtual void OnUpdate(Timestep ts) = 0;
		virtual void Init() = 0;

		virtual void SetSceneVeiwPortSize(const glm::vec2& size) = 0;
		virtual void SetEnvironment(const Environment& env) = 0;

		virtual void SetEnvMapRotation(float rotation) = 0;
		virtual void SetIBLContribution(float value) = 0;
		virtual void OnImGuiRender() = 0;
		virtual const float GetTexureLod() const { return m_TextureLod; }
		virtual void SetTexureLod(float lod) 
		{ 
			m_TextureLod = lod; 
			Material::SetUBO(10, "u_TextureLod", m_TextureLod);
		}

		virtual void SubmitMesh(const Memory::Shared<Mesh>& mesh, const glm::mat4& transform) const = 0;

		virtual Memory::Shared<Image2D> GetFinalPassImage() const = 0;
		virtual Memory::Shared<Image2D> GetShadowMapPassImage() const = 0;

		virtual Environment CreateEnvironmentScene(const std::filesystem::path& filepath) const = 0;

		static SceneRendering* Create(const Memory::Shared<Scene>& scene);
	private:
		float m_TextureLod = 1.0f;
		float m_SkyIntensity = 1.0f;
				
	};
}