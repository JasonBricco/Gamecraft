@echo off

mkdir W:\Builds\Debug\ > nul 2> nul
pushd W:\Builds\Debug\

set flags=-MTd -nologo -fp:fast -Gm- -GR- -EHa- -Oi -W4 -wd4100 -wd4201 -wd4505 -FC -Od -Zi -std:c++17 -arch:AVX -FeVoxel.exe
set defines=-D_DEBUG=1 -D_CRT_SECURE_NO_WARNINGS=1 -D_HAS_EXCEPTIONS=0
set libs=-incremental:no -opt:ref winmm.lib gdi32.lib user32.lib Shell32.lib opengl32.lib glew32s.lib glfw3.lib noise-d.lib
set link=/LIBPATH:W:\Common\Lib /LTCG /SUBSYSTEM:WINDOWS /ENTRY:"mainCRTStartup"
set inc=-I W:\Common\Include

cl %inc% %flags% %defines% W:\Code\engine.cpp /link %libs% %link%

robocopy W:\Shaders\ W:\Builds\Debug\Shaders\ /NJS /NJH /np /ndl /nfl
robocopy W:\Assets\ W:\Builds\Debug\Assets\ /XC /XN /XO /NJS /NJH /np /ndl /nfl

popd
