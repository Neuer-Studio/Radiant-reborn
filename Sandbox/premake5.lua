project "Sandbox"
    kind "ConsoleApp"

    files { 
        -- Engine 
        "Source/**.cpp", 
        "Source/**.hpp",
    }

    includedirs {
        "../Radiant/Include/",

        "../ThirdParty/spdlog/include/",
        "../ThirdParty/GLFW/include/",
        "../ThirdParty/Glad/include/",
        "../ThirdParty/glm/",
        "../ThirdParty/",
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
