#pragma once

#include <Radiant/Rendering/Image.hpp>

namespace Radiant
{
	struct Environment
	{
		std::string FilePath;
		Memory::Shared<Image2D> Radiance;
		Memory::Shared<Image2D> Irradiance;

		static Environment Create(const std::string& filepath);
	};

	struct EnvironmentAttributes
	{
		float Intensity;
		float EnvironmentMapLod;
	};
}