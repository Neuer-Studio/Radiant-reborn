local currentDir = _MAIN_SCRIPT_DIR
local dependenciesPath = currentDir .. "/BuildSystemDependencies/UNIX/MacOS/Dependencies.lua"
include(dependenciesPath)

project "Sandbox"
    kind "ConsoleApp"

    files { 
        -- Engine 
        "%{wks.location}/%{prj.name}/Source/**.cpp", 
        "%{wks.location}/%{prj.name}/Source/**.hpp",
    }

    externalincludedirs {
        "%{wks.location}/Radiant/Include/",

        "%{wks.location}/ThirdParty/spdlog/include/",
        "%{wks.location}/ThirdParty/GLFW/include/",
        "%{wks.location}/ThirdParty/Glad/include/",
        "%{wks.location}/ThirdParty/glm/",
        "%{wks.location}/ThirdParty/",
        "%{IncludeDir.entt}",
    }

    defines { "INCLUDE_HEADERS=#include <Radiant/Radiant.hpp>" }

    links{
        "Radiant",
        "GLFW",
        "Glad",
    }
    filter "configurations:Debug"
        defines { "RADIANT_CONFIG_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "RADIANT_CONFIG_RELEASE" }

    filter { "system:windows" }
        defines { "RADIANT_PLATFORM_WINDOWS" }

    filter { "system:macosx" }
        defines { "RADIANT_PLATFORM_MACOS" }

