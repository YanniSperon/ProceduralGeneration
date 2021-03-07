workspace "ProceduralGeneration"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "ProceduralGeneration"
	location "ProceduralGeneration"
	kind "ConsoleApp"
	language "C++"
	
	targetdir ("Bin/" .. outputdir .. "/%{prj.name}")
	objdir ("Bin-Intermediates/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.hpp"
	}

	includedirs
	{
		"Dependencies/GLEW",
		"Dependencies/GLFW",
		"Dependencies/ASSIMP/include",
		"%{prj.name}/src"
	}
	
	libdirs
	{
		"Dependencies/GLEW",
		"Dependencies/GLFW",
		"Dependencies/ASSIMP/lib"
	}
	
	links {
		"glew32s.lib",
		"glfw3.lib",
		"opengl32.lib",
		"zlibstaticd.lib",
		"IrrXMLd.lib",
		"assimpd.lib"
	}
	
	cppdialect "C++17"
	staticruntime "off"
	systemversion "latest"
	
	defines
	{
		"GLEW_BUILD",
		"GLFW_INCLUDE_NONE"
	}
	
	filter "system:not windows"
		
		defines
		{
			"PG_PLATFORM_OTHER"
		}

	filter "system:windows"

		defines
		{
			"PG_PLATFORM_WINDOWS"
		}
		
	filter {}

	filter "configurations:Debug"
		defines "PG_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "PG_RELEASE"
		optimize "On"

	filter {}