#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Radiant/Rendering/Mesh.hpp>
#include <Radiant/Core/Camera.hpp>

#include <Radiant/Rendering/Environment.hpp>

namespace Radiant
{
	struct IDComponent 
	{
		Math::UUID ID;
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent& other) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}

		operator std::string& () { return Tag; }
		operator const std::string& () const { return Tag; }
	};

	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent& other) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation) {}

		glm::mat4 GetTransform() const
		{
			return glm::translate(glm::mat4(1.0f), Translation)
				* glm::toMat4(glm::quat(Rotation))
				* glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct MeshComponent 
	{
		Memory::Shared<class Mesh> Mesh;

		MeshComponent() = default;
		MeshComponent(const Memory::Shared<class Mesh>& mesh)
			: Mesh(mesh) {}

		operator Memory::Shared<Radiant::Mesh>() { return Mesh; }
	};

	struct EnvironmentMap
	{
		Environment SceneEnvironment;
		float Intensity = 1.0f;
		float EnvironmentMapLod = 0.0f;
	};

	struct CameraComponent
	{
		class Camera Camera;
		bool Primary = true;

		CameraComponent() = default;
		CameraComponent(const CameraComponent& other) = default;

		operator class Camera& () { return Camera; }
		operator const class Camera& () const { return Camera; }
	};

	struct DirectionalLightComponent
	{
		glm::vec3 Radiance = { 1.0f, 1.0f, 1.0f };
		float Multiplier = 1.0f;
	};

}