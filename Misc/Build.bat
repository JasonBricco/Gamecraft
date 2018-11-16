@echo off

pushd ..

set cf=-nologo -fp:fast -Gm- -GR- -EHa- -W4 -wd4201 -wd4505 -wd4390 -FC -std:c++17 -arch:AVX2 -FeGamecraft.exe
set clb=-incremental:no -opt:ref gdi32.lib user32.lib shell32.lib shlwapi.lib opengl32.lib xaudio2.lib

IF "%~1" == "" GOTO end
IF "%~1" == "-r" GOTO build_release
IF "%~1" == "-d" GOTO build_debug

:build_release

set f=-MD -Oi -O2 -Zi
set def=-D_CRT_SECURE_NO_WARNINGS=1 -DNDEBUG -D_HAS_EXCEPTIONS=0 -DSFML_STATIC=1
set lb=glew32s.lib glfw3.lib noise.lib
set link=/LIBPATH:W:\Common\Lib /LTCG /SUBSYSTEM:WINDOWS

GOTO compile

:build_debug

set f=-MDd -Oi- -Od -Zi
set def=-D_DEBUG=1 -D_CRT_SECURE_NO_WARNINGS=1 -D_HAS_EXCEPTIONS=0 -DSFML_STATIC=1
set lb=glew32sd.lib glfw3-d.lib noise-d.lib
set link=/LIBPATH:W:\Common\Lib /ignore:4099 /SUBSYSTEM:WINDOWS

:compile

cl -I Common\Include %cf% %f% %def% Code\main.cpp /link %clb% %lb% %link%

:end

del /Q *.obj > nul 2> nul
rmdir Debug > nul 2> nul

popd
