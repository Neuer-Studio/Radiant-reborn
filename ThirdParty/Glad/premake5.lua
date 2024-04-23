project "Glad"
    kind "StaticLib"
    language "C"
    staticruntime "off"

	files
	{
        "include/glad/glad.h",
        "src/glad.c"
    }

    includedirs
	{
        "include"
    }
    
	filter "system:windows"
        systemversion "latest"
        
    filter "configurations:Debug"
		runtime "Debug"