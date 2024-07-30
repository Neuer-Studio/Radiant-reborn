#pragma once
 
#include <Radiant/Rendering/Animation/Joint.hpp>
#include <Radiant/Rendering/Animation/Animation.hpp>

struct aiNode;
struct aiAnimation;
struct aiNodeAnim;
struct aiScene;

namespace Assimp
{
	class Importer;
}

namespace Radiant::Animation
{
	class Exporter
	{
	public:
		[[nodiscard]] std::vector<std::string> GetAnimationNames(const aiScene* scene) const;
		[[nodiscard]] std::optional<Radiant::Animation::Joints> ImportJoints(const aiScene* scene) const;
		[[nodiscard]] std::optional<Animation> ImportAnimation(const aiScene* scene, const std::string_view animationName, const Joints& joint);
	};
}