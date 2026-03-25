-- Options
newoption {
	trigger     = "with-tests",
	description = "Build the unit test application",
}

-- Global settings
local workspacePath = path.join("build", _ACTION)  -- e.g. build/vs2022

-- Filters
local filter_msvc = "toolset:msc*"
local filter_gmake = "action:gmake*"
local filter_gcc = "toolset:gcc"
local filter_clang = "toolset:clang"
local filter_x86 = "platforms:x86"
local filter_x64 = "platforms:x64"
local filter_debug =  "configurations:Debug*"
local filter_release =  "configurations:Release*"
local filter_windows = "system:windows"

workspace ("Core")
configurations { "Debug", "Release" }
platforms { "x86", "x64" }
language "C++"
location (workspacePath)
characterset "MBCS"
flags   { "MultiProcessorCompile", "NoPCH", }
startproject "UnitTest"
exceptionhandling "Off"
cppdialect "c++20"
rtti "Off"

filter { filter_msvc }
	buildoptions {
		"/permissive-",
		"/Zc:__cplusplus",    -- __cplusplus will now report 202002L (for C++20)
	}
	defines { "_ENABLE_EXTENDED_ALIGNED_STORAGE", "_HAS_EXCEPTIONS=0", }
	system "Windows"

filter { filter_gcc }
    -- https://stackoverflow.com/questions/39236917/using-gccs-link-time-optimization-with-static-linked-libraries
    buildoptions { "-ffat-lto-objects" }

filter { filter_clang }
	buildoptions { "-fuse-ld=lld" }

filter { filter_x86 }
	architecture "x86"
	  
filter { filter_x64 }
	architecture "x86_64"

filter { filter_windows, filter_x86, }
	defines { "WIN32", "_WIN32", }

filter { filter_windows, filter_x64, }
	defines { "WIN64", "_WIN64", }

filter { filter_msvc, filter_debug, }
	defines { "_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1", "_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1",  }

filter { filter_msvc, filter_release, }
	defines { "_ITERATOR_DEBUG_LEVEL=0", "_SECURE_SCL=0", "_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1", "_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1",  }

filter { filter_debug }
	defines { "_DEBUG", "DEBUG", }
	flags   { "NoManifest", }
	optimize("Off")
	inlining "Default"
	warnings "Extra"
	symbols "Full"
	runtime "Debug"

filter { filter_release }
	defines { "NDEBUG", }
	flags   { "NoManifest", "NoBufferSecurityCheck", "NoRuntimeChecks", }
	optimize("Full")
	inlining "Auto"
	warnings "Extra"
	symbols "Off"
	runtime "Release"
	linktimeoptimization "On"

filter { filter_clang, filter_debug, }
	-- Address sanitizer for clang
	buildoptions
	{
		"/fsanitize=address",
	}
	-- Turn off incompatible options
	flags { "NoIncrementalLink", "NoRuntimeChecks", }
	editAndContinue "Off"

filter {}

require "core"

if _OPTIONS["with-tests"] then
	project("Catch")
		kind "StaticLib"
		files { "external/Catch/*.cpp", "external/Catch/*.hpp", } 
		includedirs { "external/Catch", }

	project("UnitTest")
		kind "ConsoleApp"
		files "test/**.*"
		includedirs { "external", } --Catch
		uses { "Core", }
		links({ "Catch", })
end
