include "../Dependencies.lua"

project "Sandbox"
    kind "ConsoleApp"

    files { 
        -- Engine 
        "Source/**.cpp", 
        "Source/**.hpp",
    }

    includedirs {
        "%{wks.location}/Radiant/Radiant/Include/",
        "%{wks.location}/Radiant/Common/Include/",

        "%{wks.location}/ThirdParty/spdlog/include/",
        "%{wks.location}/ThirdParty/GLFW/include/",
        "%{wks.location}/ThirdParty/Glad/include/",
        "%{wks.location}/ThirdParty/glm/",
        "%{wks.location}/ThirdParty/",
        "%{IncludeDir.entt}",
    }

    defines { "INCLUDE_HEADERS=#include <Radiant/Radiant.hpp>","YAML_CPP_STATIC_DEFINE" }

    links{
        "Radiant",
        "yaml-cpp",
        "GLFW",
        "Glad",
    }

    filter "configurations:Debug"
        defines { "RADIANT_CONFIG_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "RADIANT_CONFIG_RELEASE" }
