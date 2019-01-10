@echo off

set flags_d=-c -I W:\Common\Include -MDd -Oi- -Od -nologo -fp:fast -Gm- -GR- -EHa- -W0 -wd4201 -wd4505 -wd4390 -FC -std:c++17
set def_d=-D_DEBUG=1 -D_CRT_SECURE_NO_WARNINGS=1 -D_HAS_EXCEPTIONS=0

set flags=-c -I W:\Common\Include -MD -Oi -O2 -nologo -fp:fast -Gm- -GR- -EHa- -W0 -wd4201 -wd4505 -wd4390 -FC -std:c++17
set def=-D_CRT_SECURE_NO_WARNINGS=1 -DNDEBUG -D_HAS_EXCEPTIONS=0
set dir=W:\Common\Lib

set glfw_files=context.c egl_context.c init.c input.c monitor.c vulkan.c wgl_context.c win32_init.c win32_joystick.c win32_monitor.c win32_time.c win32_tls.c win32_window.c window.c xkb_unicode.c
set glfw_obj=context.obj egl_context.obj init.obj input.obj monitor.obj vulkan.obj wgl_context.obj win32_init.obj win32_joystick.obj win32_monitor.obj win32_time.obj win32_tls.obj win32_window.obj window.obj xkb_unicode.obj

IF "%~1" == "fastnoisesimd" (
	pushd ..\Common\Source\FastNoiseSIMD
	GOTO fastnoisesimd
)

IF "%~1" == "glew" (
	pushd ..\Common\Source\GLEW
	GOTO glew
)

IF "%~1" == "glfw" (
	pushd ..\Common\Source\GLFW
	GOTO glfw
)

IF "%~1" == "stb_vorbis" (
	pushd ..\Common\Source\STB_Vorbis
	GOTO stb_vorbis
)

IF "%~1" == "imgui" (
	pushd ..\Common\Source\IMGUI
	GOTO imgui
)

echo The specific library does not exist.
GOTO quit

REM ------------------------------------- FAST NOISE SIMD -------------------------------------

:fastnoisesimd

cl %flags% %def% FastNoiseSIMD.cpp FastNoiseSIMD_internal.cpp FastNoiseSIMD_sse2.cpp FastNoiseSIMD_sse41.cpp
cl %flags% %def% -arch:AVX FastNoiseSIMD_avx2.cpp FastNoiseSIMD_avx512.cpp
lib /NOLOGO /OUT:%dir%\noise.lib FastNoiseSIMD.obj FastNoiseSIMD_internal.obj FastNoiseSIMD_sse2.obj FastNoiseSIMD_sse41.obj FastNoiseSIMD_avx2.obj FastNoiseSIMD_avx512.obj

cl %flags_d% %def_d% FastNoiseSIMD.cpp FastNoiseSIMD_internal.cpp FastNoiseSIMD_sse2.cpp FastNoiseSIMD_sse41.cpp
cl %flags_d% %def_d% -arch:AVX FastNoiseSIMD_avx2.cpp FastNoiseSIMD_avx512.cpp
lib /NOLOGO /OUT:%dir%\noise-d.lib FastNoiseSIMD.obj FastNoiseSIMD_internal.obj FastNoiseSIMD_sse2.obj FastNoiseSIMD_sse41.obj FastNoiseSIMD_avx2.obj FastNoiseSIMD_avx512.obj

GOTO end

REM ------------------------------------- GLEW -------------------------------------

:glew

cl %flags% -DGLEW_STATIC %def% glew.c glewinfo.c
lib /NOLOGO /LTCG /OUT:%dir%\glew.lib glew.obj glewinfo.obj

cl %flags_d% -DGLEW_STATIC %def_d% glew.c glewinfo.c
lib /NOLOGO /OUT:%dir%\glew-d.lib glew.obj glewinfo.obj

GOTO end

REM ------------------------------------- GLFW -------------------------------------

:glfw

cl %flags% -D_GLFW_WIN32 %def% %glfw_files%
lib /NOLOGO /LTCG /OUT:%dir%\glfw.lib %glfw_obj%

cl %flags_d% -D_GLFW_WIN32 %def_d% %glfw_files%
lib /NOLOGO /OUT:%dir%\glfw-d.lib %glfw_obj%

GOTO end

REM ------------------------------------- STB_VORBIS -------------------------------------

:stb_vorbis

cl %flags% %def% stb_vorbis.c
lib /NOLOGO /OUT:%dir%\stb_vorbis.lib stb_vorbis.obj

cl %flags_d% %def_d% stb_vorbis.c
lib /NOLOGO /OUT:%dir%\stb_vorbis-d.lib stb_vorbis.obj

GOTO end;

REM ------------------------------------- IMGUI -------------------------------------

:imgui

cl %flags% %def% imgui.cpp imgui_draw.cpp imgui_widgets.cpp
lib /NOLOGO /LTCG /OUT:%dir%\imgui.lib imgui.obj imgui_draw.obj imgui_widgets.obj

cl %flags_d% %def_d% imgui.cpp imgui_draw.cpp imgui_widgets.cpp
lib /NOLOGO /OUT:%dir%\imgui-d.lib imgui.obj imgui_draw.obj imgui_widgets.obj

GOTO end;

:end

del /Q *.obj > nul 2> nul
del /Q pt > nul 2> nul
popd

:quit
