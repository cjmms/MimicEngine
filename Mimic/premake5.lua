-- Project Solution name: Mimic
workspace "Mimic"
    architecture "x64"

    -- "Dist" is ship version
    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

-- build type, system, architecture
outputdir = "{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"



project "Mimic"         -- name of Mimic project
    location "Mimic"    -- location of Mimic.vcxproj
    kind "SharedLib"    -- generate DLL
    language "C++"

    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",      -- include all the header file
        "%{prj.name}/src/**.cpp"    -- include all cpp file
    }

    includedirs
    {

    }

    -- ifdef system: windows
    filter "system:windows"
    cppdialect "C++17"
    staticruntime "On"      -- dll
    systemversion "latest"  -- system version


    defines
    {
        "MC_PLATFORM_WINDOWS",
        "MC_BUILD_DLL"
    }

    -- copy the dll file from build dir to output dir 
    postbuildcommands
    {
        {"{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/SandBox"}
    }


    filter "configurations:Debug"
        defines "MC_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "MC_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "MC_DIST"
        optimize "On"



project "SandBox"
    location "SandBox"
    kind "ConsoleApp"   -- .exe file

    language "C++"

    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "Mimic/src"     -- include Mimic Engine
    }

    links
    {
        "Mimic"
    }

    -- ifdef system: windows
    filter "system:windows"
    cppdialect "C++17"
    staticruntime "On"      -- dll
    systemversion "latest"  -- system version


    defines
    {
        "MC_PLATFORM_WINDOWS",
    }

    filter "configurations:Debug"
        defines "MC_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "MC_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "MC_DIST"
        optimize "On"