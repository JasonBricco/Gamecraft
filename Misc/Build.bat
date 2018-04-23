@echo off

pushd ..

set cf=-nologo -fp:fast -Gm- -GR- -EHa- -W4 -wd4100 -wd4201 -wd4505 -FC -std:c++17 -arch:AVX -FeVoxel.exe
set clb=-incremental:no -opt:ref winmm.lib gdi32.lib user32.lib Shell32.lib opengl32.lib

IF "%~1" == "" GOTO end
IF "%~1" == "-r" GOTO build_release
IF "%~1" == "-d" GOTO build_debug

:build_release

set f=-MD -Oi -O2
set def=-D_CRT_SECURE_NO_WARNINGS=1 -D_HAS_EXCEPTIONS=0
set lb=glew32s.lib glfw3.lib noise.lib
set link=/LIBPATH:W:\Common\Lib /LTCG /SUBSYSTEM:WINDOWS

GOTO compile_release

:build_debug

set f=-MDd -Oi- -Od -Zi
set def=-D_DEBUG=1 -D_CRT_SECURE_NO_WARNINGS=1 -D_HAS_EXCEPTIONS=0
set lb=glew32sd.lib glfw3-d.lib noise-d.lib
set link=/LIBPATH:W:\Common\Lib /SUBSYSTEM:WINDOWS

GOTO compile_debug

:compile_release

cl -I Common\Include %cf% %f% %def% Code\engine.cpp /link %clb% %lb% %link%
del /Q *.obj
del /Q *.pdb > nul 2> nul
GOTO end

:compile_debug

cl -I Common\Include %cf% %f% %def% Code\engine.cpp /link %clb% %lb% %link%
del /Q *.obj
GOTO end

:end

popd