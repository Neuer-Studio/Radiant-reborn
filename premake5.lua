-- Config file for Radiant Engine

workspace "Radiant"
    configurations { "Debug", "Release" }
    architecture "x64"
    startproject "Sandbox"

    language "C++"
	cppdialect "C++17"
    targetdir "build/Bin/%{cfg.buildcfg}"
	objdir "build/Intermediates/%{cfg.buildcfg}"

	externalanglebrackets "On"
	externalwarnings "Off"
	warnings "Off"


group "ThirdParty"
include "ThirdParty/"
group ""

-- Platform-specific includes
if os.target() == "windows" then
    include "Radiant/BuildSystem/Windows"
elseif os.target() == "macosx" then
    include "Radiant/BuildSystem/UNIX/MacOS/premake5.lua"
end

include "Sandbox/"
