@echo off

IF "%~1" == "" GOTO end
IF "%~1" == "-r" GOTO build_release
IF "%~1" == "-d" GOTO build_debug

:build_release

mkdir W:\Builds\Release\ > nul 2> nul
pushd W:\Builds\Release\

set f=-MD -Oi -O2
set def=-D_CRT_SECURE_NO_WARNINGS=1 -D_HAS_EXCEPTIONS=0
set lb=glew32s.lib glfw3.lib noise.lib
set link=/LIBPATH:W:\Common\Lib /LTCG /SUBSYSTEM:WINDOWS /OUT:Voxel.exe

robocopy W:\Shaders\ W:\Builds\Release\Shaders\ /NJS /NJH /np /ndl /nfl /XO
robocopy W:\Assets\ W:\Builds\Release\Assets\ /XC /XN /XO /NJS /NJH /np /ndl /nfl /XO
GOTO compile

:build_debug

mkdir W:\Builds\Debug\ > nul 2> nul
pushd W:\Builds\Debug\

set f=-MDd -Oi- -Od -Zi
set def=-D_DEBUG=1 -D_CRT_SECURE_NO_WARNINGS=1 -D_HAS_EXCEPTIONS=0
set lb=glew32sd.lib glfw3-d.lib noise-d.lib
set link=/LIBPATH:W:\Common\Lib /SUBSYSTEM:WINDOWS /OUT:Voxel.exe

robocopy W:\Shaders\ W:\Builds\Debug\Shaders\ /NJS /NJH /np /ndl /nfl /XO
robocopy W:\Assets\ W:\Builds\Debug\Assets\ /XC /XN /XO /NJS /NJH /np /ndl /nfl /XO
GOTO compile

:compile

set cf=-nologo -fp:fast -Gm- -GR- -EHa- -W4 -wd4100 -wd4201 -wd4505 -FC -std:c++17 -arch:AVX
set clb=-incremental:no -opt:ref winmm.lib gdi32.lib user32.lib Shell32.lib opengl32.lib

IF NOT EXIST stdafx.pch GOTO compile_pch
GOTO compile_norm

:compile_pch

cl -I W:\Common\Include %cf% %f% %def% -Ycstdafx.h W:\Code\stdafx.cpp -c
cl -I W:\Common\Include %cf% %f% %def% -Yustdafx.h W:\Code\engine.cpp -c
link -nologo stdafx.obj engine.obj %clb% %lb% %link%
GOTO finish

:compile_norm

cl -I W:\Common\Include %cf% %f% %def% -Yustdafx.h W:\Code\engine.cpp -c
link -nologo stdafx.obj engine.obj %clb% %lb% %link%
GOTO finish

:finish

popd

:end
