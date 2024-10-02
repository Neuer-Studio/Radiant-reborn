
project "Common"
    kind "StaticLib"

    files { 
        -- COMMON 
        "Source/Common/**.cpp", 
        "Source/Common/**.hpp",

        "Include/**.hpp",
    }

    includedirs {
        "Source/",
        "Include/",
        "Include/Common/",
    }

    includedirs {
        "%{wks.location}/ThirdParty/spdlog/include/",
        "%{wks.location}/ThirdParty/yaml-cpp/include",
        "%{wks.location}/ThirdParty/GLFW/include/",
        "%{wks.location}/ThirdParty/glm/",
    }
    
    links{
        "yaml-cpp",
    }

    defines { "YAML_CPP_STATIC_DEFINE" }
    filter "configurations:Debug"
        defines { "RADIANT_CONFIG_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "RADIANT_CONFIG_RELEASE" }

    filter { "system:windows" }
        defines { "RADIANT_PLATFORM_WINDOWS" }
        files {
            "Source/Common/Platform/Windows/**.cpp",
            "Source/Common/Platform/Windows/**.hpp",
        }
