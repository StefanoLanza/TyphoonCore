project("Core")
	local path = "."
	kind "StaticLib"
	files { path .. "/**.cpp" }
	files { path .. "/**.h" }
	includedirs { path .. "/include", path .. "/include/core", }
	exceptionhandling "Off"
	rtti "Off"
	usage "INTERFACE"
		includedirs { path .. "/include", }
		links { "Core" }
