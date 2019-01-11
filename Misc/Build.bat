@echo off

pushd ..

IF "%~1" == "-ar" GOTO asset_builder_release
IF "%~1" == "-ad" GOTO asset_builder_debug

set cf=-nologo -fp:fast -Gm- -GR- -EHa- -W4 -wd4201 -wd4505 -wd4390 -FC -std:c++17 -arch:AVX2 -FeGamecraft.exe
set clb=-incremental:no -opt:ref gdi32.lib user32.lib shell32.lib shlwapi.lib opengl32.lib xaudio2.lib

IF "%~1" == "" GOTO end
IF "%~1" == "-r" GOTO build_release
IF "%~1" == "-d" GOTO build_debug

REM Release mode build.
:build_release

set f=-MD -Oi -O2 -Zi
set def=-D_CRT_SECURE_NO_WARNINGS=1 -DNDEBUG -D_HAS_EXCEPTIONS=0
set lb=glew.lib glfw.lib noise.lib stb_vorbis.lib imgui.lib
set link=/LIBPATH:W:\Common\Lib /SUBSYSTEM:WINDOWS

GOTO compile

REM Debug mode build.
:build_debug

set f=-MDd -Oi- -Od -Zi
set def=-D_DEBUG=1 -D_CRT_SECURE_NO_WARNINGS=1 -D_HAS_EXCEPTIONS=0
set lb=glew-d.lib glfw-d.lib noise-d.lib stb_vorbis-d.lib imgui-d.lib
set link=/LIBPATH:W:\Common\Lib /ignore:4099 /SUBSYSTEM:WINDOWS

REM Compile the engine.
:compile

cl -I Common\Include %cf% %f% %def% Code\main.cpp /link %clb% %lb% %link%

GOTO end

REM Asset builder debug build.
:asset_builder_debug

set f=-nologo -Gm- -GR- -W4 -wd4201 -wd4505 -wd4390 -FC -std:c++17 -FeBuildAssets.exe -MDd -Od -Zi
set lb=-incremental:no -opt:ref /LIBPATH:W:\Common\Lib stb_vorbis-d.lib

cl -I Common\Include %f% -D_CRT_SECURE_NO_WARNINGS=1 -D_HAS_EXCEPTIONS=0 Code\assetbuilder.cpp /link %lb%

GOTO end

REM Asset builder release build.
:asset_builder_release

set f=-nologo -Gm- -GR- -W4 -wd4201 -wd4505 -wd4390 -FC -std:c++17 -FeBuildAssets.exe -MD -O2
set lb=-incremental:no -opt:ref /LIBPATH:W:\Common\Lib stb_vorbis.lib

cl -I Common\Include %f% -D_CRT_SECURE_NO_WARNINGS=1 -D_HAS_EXCEPTIONS=0 Code\assetbuilder.cpp /link %lb%

del /Q BuildAssets.pdb > nul 2> nul

GOTO end

REM Clean up.
:end

del /Q *.obj > nul 2> nul
rmdir Debug > nul 2> nul

popd
