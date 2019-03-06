@echo off

set flags_d=-c -I W:\Common\Include -MDd -Oi- -Od -nologo -fp:fast -Gm- -GR- -EHa- -W0 -wd4201 -wd4505 -wd4390 -FC -std:c++17
set def_d=-D_DEBUG=1 -D_CRT_SECURE_NO_WARNINGS=1 -D_HAS_EXCEPTIONS=0

set flags=-c -I W:\Common\Include -MD -Oi -O2 -nologo -fp:fast -Gm- -GR- -EHa- -W0 -wd4201 -wd4505 -wd4390 -FC -std:c++17
set def=-D_CRT_SECURE_NO_WARNINGS=1 -DNDEBUG -D_HAS_EXCEPTIONS=0
set dir=W:\Common\Lib

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

echo The specific library does not exist. Options: 'glew', 'glfw', 'stb_vorbis', 'imgui', 'fastnoisesimd'.
GOTO quit

REM ------------------------------------- FAST NOISE SIMD -------------------------------------

:fastnoisesimd

cl %flags% %def% FastNoiseSIMD.cpp FastNoiseSIMD_internal.cpp FastNoiseSIMD_sse2.cpp FastNoiseSIMD_sse41.cpp
cl %flags% %def% -arch:AVX FastNoiseSIMD_avx2.cpp FastNoiseSIMD_avx512.cpp
lib /NOLOGO /OUT:%dir%\noise.lib *.obj

cl %flags_d% %def_d% FastNoiseSIMD.cpp FastNoiseSIMD_internal.cpp FastNoiseSIMD_sse2.cpp FastNoiseSIMD_sse41.cpp
cl %flags_d% %def_d% -arch:AVX FastNoiseSIMD_avx2.cpp FastNoiseSIMD_avx512.cpp
lib /NOLOGO /OUT:%dir%\noise-d.lib *.obj

GOTO end

REM ------------------------------------- GLEW -------------------------------------

:glew

cl %flags% -DGLEW_STATIC %def% *.c
lib /NOLOGO /LTCG /OUT:%dir%\glew.lib *.obj

cl %flags_d% -DGLEW_STATIC %def_d% *.c
lib /NOLOGO /OUT:%dir%\glew-d.lib *.obj

GOTO end

REM ------------------------------------- GLFW -------------------------------------

:glfw

cl %flags% -D_GLFW_WIN32 %def% *.c
lib /NOLOGO /LTCG /OUT:%dir%\glfw.lib *.obj

cl %flags_d% -D_GLFW_WIN32 %def_d% *.c
lib /NOLOGO /OUT:%dir%\glfw-d.lib *.obj

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

cl %flags% %def% *.cpp
lib /NOLOGO /LTCG /OUT:%dir%\imgui.lib *.obj

cl %flags_d% %def_d% *.cpp
lib /NOLOGO /OUT:%dir%\imgui-d.lib *.obj

GOTO end;

:end

del /Q *.obj > nul 2> nul
del /Q pt > nul 2> nul
popd

:quit
